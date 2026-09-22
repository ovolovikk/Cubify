#include "DX12Renderer.hpp"

#include "d3dx12.h"
#include "Graphics/DirectX12Backend/DX12Common.hpp"
#include "Graphics/DirectX12Backend/DX12Pipeline.hpp"
#include "Logging/Log.hpp"
#include "stb_image.h"
#include "stb_image_write.h"

namespace Cubify::DX12
{
    static ID3D12PipelineState* ToPipelineState(const IPipeline* pipeline)
    {
        return pipeline ? static_cast<const DX12Pipeline*>(pipeline)->pipelineState() : nullptr;
    }

    DX12Renderer::DX12Renderer(DX12Device& device, void* windowHandle, int width, int height)
        : m_factory(device.factory())
        , m_device(device.device())
        , m_commandQueue(device.commandQueue())
        , m_rootSignature(device.rootSignature())
        , m_deletionQueue(std::make_shared<DX12DeletionQueue>()), m_width(width), m_height(height)
    {
        CreateSwapChain(windowHandle, width, height);
        CreateRtvHeap();
        CreateRenderTargets();
        CreateDsvHeap();
        CreateDepthStencil();
        CreateCommandObjects();
        CreateFence();
        m_solidPipeline = device.createPipeline({
            .debugName = "Solid",
            .vertexShader = { "vertex_shader", "VSMain" },
            .fragmentShader = { "pixel_shader", "PSMain" },
        });
        m_transparentPipeline = device.createPipeline({
            .debugName = "Transparent",
            .vertexShader = { "vertex_shader", "VSMain" },
            .fragmentShader = { "pixel_shader", "PSMain" },
            .blend = BlendMode::AlphaBlend,
            .cull = CullMode::None,
            .depthWrite = false,
        });
        CreateSrvHeap();
        CreateTextureArray();
    }

    DX12Renderer::~DX12Renderer()
    {
        if (m_device && m_fence && m_fenceEvent)
        {
            WaitForGpu();
        }
        if (m_fenceEvent)
        {
            CloseHandle(m_fenceEvent);
        }
    }

    // IRendererBackend interface implementation

    void DX12Renderer::resize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }
        if (width == m_width && height == m_height)
        {
            return;
        }

        // Careful about back buffers still having work
        WaitForGpu();

        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            m_renderTargets[i].Reset();
            m_fenceValues[i] = m_fenceValues[m_currentFrame];
        }

        m_swapChain->ResizeBuffers(
            FRAME_COUNT,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            BACK_BUFFER_FORMAT,
            m_swapChainFlags);

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();
        m_width = width;
        m_height = height;

        // RTVs need to be recreated
        CreateRenderTargets();

        m_depthStencil.Reset();
        CreateDepthStencil();
    }

    void DX12Renderer::beginFrame()
    {
        if (m_commandListOpen)
        {
            return;
        }

        m_commandAllocators[m_currentFrame].Get()->Reset();
        m_commandList->Reset(m_commandAllocators[m_currentFrame].Get(), ToPipelineState(m_solidPipeline.get()));
        m_commandListOpen = true;

        // MoveToNextFrame already waited on this slot, so whatever the GPU was
        // reading two frames ago is definitely free now
        m_deferredReleases[m_currentFrame].clear();
        ProcessMeshDeletions();

        // Update the back buffer state to be writable before rendering
        CD3DX12_RESOURCE_BARRIER toRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_currentFrame].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        m_commandList->ResourceBarrier(1, &toRenderTarget);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrame, m_rtvDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        const float clearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
        CD3DX12_RECT scissor(0, 0, m_width, m_height);
        m_commandList->RSSetViewports(1, &viewport);
        m_commandList->RSSetScissorRects(1, &scissor);

        if (m_rootSignature)
        {
            m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
            m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            if (m_srvHeap && m_textureArray)
            {
                // The heap must be bound before the table that points into it
                ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
                m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
                m_commandList->SetGraphicsRootDescriptorTable(
                    DX12Pipeline::ROOT_PARAM_TEXTURES, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
            }
        }
    }

    void DX12Renderer::endFrame()
    {
        CD3DX12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_currentFrame].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        );

        m_commandList->ResourceBarrier(1, &toPresent);
        m_commandList->Close();
        m_commandListOpen = false;
        ID3D12CommandList* commandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        UINT presentFlags = (m_swapChainFlags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
            ? DXGI_PRESENT_ALLOW_TEARING
            : 0;

        HRESULT presentResult = m_swapChain->Present(0, presentFlags);
        if (FAILED(presentResult))
        {
            LOGE("[DX12Renderer] Failed to present swap chain");
            LOGE("[DX12Renderer] hr = 0x%08X", presentResult);
        }

        MoveToNextFrame();
    }

    void DX12Renderer::beginTransparentPass()
    {
        if (m_transparentPipeline)
        {
            m_commandList->SetPipelineState(ToPipelineState(m_transparentPipeline.get()));
        }
    }

    void DX12Renderer::endTransparentPass()
    {
        if (m_solidPipeline)
        {
            m_commandList->SetPipelineState(ToPipelineState(m_solidPipeline.get()));
        }
    }

    void DX12Renderer::setViewProjection(const glm::mat4& view, const glm::mat4& projection)
    {
        // This remapping is needed because OpenGL uses [-1, 1] for depth,
        // while DX12 uses [0, 1].
        glm::mat4 depthZeroToOne(1.0f);
        depthZeroToOne[2][2] = 0.5f;
        depthZeroToOne[3][2] = 0.5f;

        m_viewProj = depthZeroToOne * projection * view;

        if (m_commandListOpen && m_rootSignature)
        {
            m_commandList->SetGraphicsRoot32BitConstants(
                DX12Pipeline::ROOT_PARAM_VIEW_PROJ, DX12Pipeline::MATRIX_CONSTANT_COUNT, &m_viewProj, 0);
        }
    }

    // Test mode only
    bool DX12Renderer::captureBackbuffer(const char* filePath)
    {
        if (!m_commandListOpen || !m_device)
        {
            LOGE("[DX12Renderer] captureBackbuffer called outside of a frame");
            return false;
        }

        ID3D12Resource* backBuffer = m_renderTargets[m_currentFrame].Get();
        D3D12_RESOURCE_DESC backBufferDesc = backBuffer->GetDesc();

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        UINT rowCount = 0;
        UINT64 rowSizeInBytes = 0;
        UINT64 totalBytes = 0;
        m_device->GetCopyableFootprints(&backBufferDesc, 0, 1, 0,
            &footprint, &rowCount, &rowSizeInBytes, &totalBytes);

        // READBACK is the mirror image of UPLOAD: GPU writes, CPU reads
        CD3DX12_HEAP_PROPERTIES readbackHeap(D3D12_HEAP_TYPE_READBACK);
        CD3DX12_RESOURCE_DESC readbackDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);

        ComPtr<ID3D12Resource> readback;
        HR_FALLBACK(m_device->CreateCommittedResource(
            &readbackHeap,
            D3D12_HEAP_FLAG_NONE,
            &readbackDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&readback)),
            false, "[DX12Renderer] Failed to create readback buffer");

        CD3DX12_RESOURCE_BARRIER toCopySource = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_commandList->ResourceBarrier(1, &toCopySource);

        CD3DX12_TEXTURE_COPY_LOCATION destination(readback.Get(), footprint);
        CD3DX12_TEXTURE_COPY_LOCATION source(backBuffer, 0);
        m_commandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);

        CD3DX12_RESOURCE_BARRIER backToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &backToRenderTarget);

        m_commandList->Close();
        ID3D12CommandList* commandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
        WaitForGpu();

        m_commandList->Reset(m_commandAllocators[m_currentFrame].Get(), ToPipelineState(m_solidPipeline.get()));

        void* mapped = nullptr;
        CD3DX12_RANGE readRange(0, static_cast<SIZE_T>(totalBytes));
        HR_FALLBACK(readback->Map(0, &readRange, &mapped), false,
            "[DX12Renderer] Failed to map readback buffer");

        const size_t rowBytes = static_cast<size_t>(rowSizeInBytes);
        std::vector<uint8_t> pixels(rowBytes * rowCount);
        for (UINT row = 0; row < rowCount; ++row)
        {
            memcpy(pixels.data() + rowBytes * row,
                static_cast<const uint8_t*>(mapped) + static_cast<size_t>(footprint.Footprint.RowPitch) * row,
                rowBytes);
        }

        CD3DX12_RANGE writtenRange(0, 0);
        readback->Unmap(0, &writtenRange);

        // D3D and PNG both start at the top left, so unlike the OpenGL path
        // there is no flip here.
        stbi_flip_vertically_on_write(0);
        return stbi_write_png(filePath,
            static_cast<int>(backBufferDesc.Width),
            static_cast<int>(backBufferDesc.Height),
            4, pixels.data(), static_cast<int>(rowBytes)) != 0;
    }

    void DX12Renderer::setWorldSettings(const WorldSettings& settings) {}

    void DX12Renderer::uploadMesh(MeshHandle& mesh, const std::vector<Quad>& quads)
    {
        if (quads.empty())
        {
            // Chunk became empty. Keep the id, just stop drawing it.
            if (mesh.isValid())
            {
                auto it = m_meshes.find(mesh.id());
                if (it != m_meshes.end())
                {
                    it->second.quadCount = 0;
                }
            }
            return;
        }

        if (!m_commandListOpen)
        {
            LOGE("[DX12Renderer] uploadMesh called outside of a frame, skipping");
            return;
        }

        if (!mesh.isValid())
        {
            mesh = MeshHandle(MeshId{ m_nextMeshId++ }, m_deletionQueue);
        }

        ComPtr<ID3D12Resource> uploadBuffer;
        ComPtr<ID3D12Resource> buffer = CreateGpuBuffer(
            quads.data(),
            quads.size() * sizeof(Quad),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            uploadBuffer);
        if (!buffer)
        {
            return;
        }

        GpuMesh& gpuMesh = m_meshes[mesh.id()];

        // Remeshing a chunk replaces its buffer, but frames in flight may still
        // be reading the old one
        if (gpuMesh.buffer)
        {
            m_deferredReleases[m_currentFrame].push_back(std::move(gpuMesh.buffer));
        }

        gpuMesh.buffer = std::move(buffer);
        gpuMesh.quadCount = static_cast<UINT>(quads.size());

        // The copy is only recorded, not executed, so staging has to survive
        // until this frame is done
        m_deferredReleases[m_currentFrame].push_back(std::move(uploadBuffer));
    }

    void DX12Renderer::draw(MeshId mesh, const glm::mat4& model)
    {
        auto it = m_meshes.find(mesh);
        if (it == m_meshes.end() || it->second.quadCount == 0 || !it->second.buffer)
        {
            return;
        }

        m_commandList->SetGraphicsRoot32BitConstants(
            DX12Pipeline::ROOT_PARAM_MODEL, DX12Pipeline::MATRIX_CONSTANT_COUNT, &model, 0);
        m_commandList->SetGraphicsRootShaderResourceView(
            DX12Pipeline::ROOT_PARAM_QUADS, it->second.buffer->GetGPUVirtualAddress());

        // One instance per quad, the shader builds its six corners
        m_commandList->DrawInstanced(VERTICES_PER_QUAD, it->second.quadCount, 0, 0);
    }

    // DX12 Initialization starts there

    void DX12Renderer::CreateSwapChain(void* windowHandle, int width, int height)
    {
        HWND hwnd = static_cast<HWND>(windowHandle);

        BOOL allowTearing = FALSE;
        m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        m_swapChainFlags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        DXGI_SWAP_CHAIN_DESC1 desc{
            .Width = static_cast<UINT>(width),
            .Height = static_cast<UINT>(height),
            .Format = BACK_BUFFER_FORMAT,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = FRAME_COUNT,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .Flags = m_swapChainFlags
        };

        ComPtr<IDXGISwapChain1> oldSwapChain;
        HR_CHECK(m_factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(), hwnd, &desc, nullptr, nullptr, &oldSwapChain
        ), "[DX12Renderer] Failed to create swap chain");

        m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        HR_CHECK(oldSwapChain.As(&m_swapChain),
            "[DX12Renderer] Failed to query IDXGISwapChain3");

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();
        LOGI("[DX12Renderer] Swap chain created successfully");
    }

    void DX12Renderer::CreateRtvHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = FRAME_COUNT,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        HR_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap)),
            "[DX12Renderer] Failed to create RTV descriptor heap");
        SetDebugName(m_rtvHeap.Get(), L"RTV Heap");
        LOGI("[DX12Renderer] RTV descriptor heap created successfully");

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    void DX12Renderer::CreateRenderTargets()
    {
        // create view for each buffer in swap chain
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            HR_CHECK(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])),
                "[DX12Renderer] Failed to get swap chain buffer %u", i);

            std::wstring rtName = L"Back Buffer " + std::to_wstring(i);
            SetDebugName(m_renderTargets[i].Get(), rtName.c_str());

            m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
        LOGI("[DX12Renderer] Render targets created successfully");
    }

    void DX12Renderer::CreateDsvHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        HR_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_dsvHeap)),
            "[DX12Renderer] Failed to create DSV descriptor heap");
        SetDebugName(m_dsvHeap.Get(), L"DSV Heap");
        LOGI("[DX12Renderer] DSV descriptor heap created successfully");
    }

    void DX12Renderer::CreateDepthStencil()
    {
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        CD3DX12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DEPTH_FORMAT,
            static_cast<UINT64>(m_width),
            static_cast<UINT>(m_height),
            1, 1);
        depthDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        CD3DX12_CLEAR_VALUE clearValue(DEPTH_FORMAT, 1.0f, 0);

        HR_CHECK(m_device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_depthStencil)),
            "[DX12Renderer] Failed to create depth stencil buffer");

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{
            .Format = DEPTH_FORMAT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Flags = D3D12_DSV_FLAG_NONE
        };

        m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        SetDebugName(m_depthStencil.Get(), L"Depth Buffer");
        LOGI("[DX12Renderer] Depth stencil buffer created successfully");
    }

    void DX12Renderer::CreateCommandObjects()
    {

        // Command Allocator
        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            HR_CHECK(m_device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])),
                "[DX12Renderer] Failed to create command allocator %u", i);

            std::wstring allocName = L"Command Allocator " + std::to_wstring(i);
            SetDebugName(m_commandAllocators[i].Get(), allocName.c_str());
        }

        // Command List
        HR_CHECK(m_device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_commandAllocators[m_currentFrame].Get(),
            nullptr,
            IID_PPV_ARGS(&m_commandList)),
            "[DX12Renderer] Failed to create command list");

        m_commandList->Close();
        SetDebugName(m_commandList.Get(), L"Main Command List");
        LOGI("[DX12Renderer] Command objects created successfully");
    }

    void DX12Renderer::CreateFence()
    {
        HR_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)),
            "[DX12Renderer] Failed to create fence");
        m_fenceValues[m_currentFrame] = 1;
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            LOGE("[DX12Renderer] Failed to create fence event");
        }
        SetDebugName(m_fence.Get(), L"Frame Fence");
        LOGI("[DX12Renderer] Fence created successfully");
    }

    ComPtr<ID3D12Resource> DX12Renderer::CreateGpuBuffer(const void* data, UINT64 size,
        D3D12_RESOURCE_STATES finalState, ComPtr<ID3D12Resource>& outUploadBuffer)
    {
        CD3DX12_HEAP_PROPERTIES heapDesc(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
        ComPtr<ID3D12Resource> buffer;

        HR_FALLBACK(m_device->CreateCommittedResource(
            &heapDesc,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&buffer)
        ), nullptr, "[DX12Renderer] Failed to create GPU buffer");

        CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
        HR_FALLBACK(m_device->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&outUploadBuffer)
        ), nullptr, "[DX12Renderer] Failed to create upload buffer");

        CD3DX12_RANGE readRange(0, 0);
        void* mappedData = nullptr;
        HR_FALLBACK(outUploadBuffer->Map(0, &readRange, &mappedData), nullptr,
            "[DX12Renderer] Failed to map upload buffer");
        memcpy(mappedData, data, static_cast<size_t>(size));
        outUploadBuffer->Unmap(0, nullptr);

        // DX12 Does it automatically, but explicitly writing it to remember
        CD3DX12_RESOURCE_BARRIER toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        m_commandList->ResourceBarrier(1, &toCopyDest);

        m_commandList->CopyBufferRegion(buffer.Get(), 0, outUploadBuffer.Get(), 0, size);

        // This one is never automatic
        CD3DX12_RESOURCE_BARRIER toReadable = CD3DX12_RESOURCE_BARRIER::Transition(
            buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, finalState);
        m_commandList->ResourceBarrier(1, &toReadable);

        return buffer;
    }

    void DX12Renderer::CreateSrvHeap()
    {
        // Shader visible, currently just the one slot for the block texture array
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
        };

        HR_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)),
            "[DX12Renderer] Failed to create SRV descriptor heap");
        SetDebugName(m_srvHeap.Get(), L"SRV Heap");
        LOGI("[DX12Renderer] SRV descriptor heap created successfully");
    }

    void DX12Renderer::CreateTextureArray()
    {
        // Layer order defines the indices the mesher packs into the quads
        const char* layerPaths[] = {
            "assets/textures/grass_top.png",
            "assets/textures/grass_side.png",
            "assets/textures/dirt.png",
            "assets/textures/stone.png",
            "assets/textures/sand.png",
            "assets/textures/wooden_plank.png",
            "assets/textures/water.png",
            "assets/textures/bedrock.png",
            "assets/textures/ice.png",
            "assets/textures/sectorr_grass_top.png",
            "assets/textures/sectorr_grass_side.png",
            "assets/textures/sectorr_dirt.png",
            "assets/textures/sectorr_stone.png",
            "assets/textures/sectorr_sand.png",
            "assets/textures/sectorr_water.png",
            "assets/textures/utopia_sand.png",
            "assets/textures/utopia_silt.png",
            "assets/textures/utopia_water.png"
        };
        const UINT layerCount = _countof(layerPaths);

        stbi_set_flip_vertically_on_load(true);

        std::vector<unsigned char*> images(layerCount, nullptr);
        int width = 0;
        int height = 0;

        for (UINT i = 0; i < layerCount; ++i)
        {
            int w = 0, h = 0, comp = 0;
            images[i] = stbi_load(layerPaths[i], &w, &h, &comp, 4);
            if (!images[i])
            {
                LOGE("[DX12Renderer] Failed to load texture layer: %s", layerPaths[i]);
                for (UINT j = 0; j < i; ++j) stbi_image_free(images[j]);
                return;
            }
            if (i == 0)
            {
                width = w;
                height = h;
            }
            else if (w != width || h != height)
            {
                LOGE("[DX12Renderer] Texture layer %u size mismatch", i);
                for (UINT j = 0; j <= i; ++j) stbi_image_free(images[j]);
                return;
            }
        }

        CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            static_cast<UINT64>(width),
            static_cast<UINT>(height),
            static_cast<UINT16>(layerCount),
            1);

        HRESULT hr = m_device->CreateCommittedResource(
            &defaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_textureArray));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create texture array");
            for (UINT i = 0; i < layerCount; ++i) stbi_image_free(images[i]);
            return;
        }

        // Rows have to be padded to 256 bytes and each slice aligned to 512, so
        // the staging buffer is bigger than the raw pixels. UpdateSubresources
        // works all of that out from the footprints.
        const UINT64 uploadSize = GetRequiredIntermediateSize(m_textureArray.Get(), 0, layerCount);

        CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

        ComPtr<ID3D12Resource> uploadBuffer;
        hr = m_device->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create texture upload buffer");
            for (UINT i = 0; i < layerCount; ++i) stbi_image_free(images[i]);
            return;
        }

        // Every array slice is its own subresource
        std::vector<D3D12_SUBRESOURCE_DATA> subresources(layerCount);
        for (UINT i = 0; i < layerCount; ++i)
        {
            subresources[i].pData = images[i];
            subresources[i].RowPitch = static_cast<LONG_PTR>(width) * 4;
            subresources[i].SlicePitch = subresources[i].RowPitch * height;
        }

        m_commandAllocators[m_currentFrame]->Reset();
        m_commandList->Reset(m_commandAllocators[m_currentFrame].Get(), nullptr);

        UpdateSubresources(
            m_commandList.Get(),
            m_textureArray.Get(),
            uploadBuffer.Get(),
            0, 0, layerCount,
            subresources.data());

        CD3DX12_RESOURCE_BARRIER toReadable = CD3DX12_RESOURCE_BARRIER::Transition(
            m_textureArray.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &toReadable);

        m_commandList->Close();
        ID3D12CommandList* lists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(lists), lists);
        WaitForGpu();

        for (UINT i = 0; i < layerCount; ++i)
        {
            stbi_image_free(images[i]);
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2DArray = {
                .MostDetailedMip = 0,
                .MipLevels = 1,
                .FirstArraySlice = 0,
                .ArraySize = layerCount
            }
        };

        m_device->CreateShaderResourceView(
            m_textureArray.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

        SetDebugName(m_textureArray.Get(), L"Block Textures");
        LOGI("[DX12Renderer] Texture array created: %d layers of %dx%d", layerCount, width, height);
    }

    void DX12Renderer::ProcessMeshDeletions()
    {
        for (MeshId id : m_deletionQueue->takeMeshes())
        {
            auto it = m_meshes.find(id);
            if (it == m_meshes.end())
            {
                continue;
            }
            if (it->second.buffer)
            {
                m_deferredReleases[m_currentFrame].push_back(std::move(it->second.buffer));
            }
            m_meshes.erase(it);
        }
    }

    void DX12Renderer::WaitForGpu()
    {
        const UINT64 value = m_fenceValues[m_currentFrame];
        m_commandQueue->Signal(m_fence.Get(), value);
        m_fence->SetEventOnCompletion(value, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
        m_fenceValues[m_currentFrame]++;
    }

    void DX12Renderer::MoveToNextFrame()
    {
        const UINT64 currentValue = m_fenceValues[m_currentFrame];
        m_commandQueue->Signal(m_fence.Get(), currentValue);

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

        // Only wait if the GPU is still busy with the frame we are about to reuse
        if (m_fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
        {
            m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_fenceValues[m_currentFrame] = currentValue + 1;
    }
}