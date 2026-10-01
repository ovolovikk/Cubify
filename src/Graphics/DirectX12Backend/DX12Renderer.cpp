#include "DX12Renderer.hpp"

#include "d3dx12.h"
#include "Graphics/DirectX12Backend/DX12Common.hpp"
#include "Graphics/DirectX12Backend/DX12Pipeline.hpp"
#include "Graphics/DirectX12Backend/DX12Screenshot.hpp"
#include "Graphics/DirectX12Backend/DX12SwapChain.hpp"
#include "Graphics/DirectX12Backend/DX12Texture.hpp"
#include "Logging/Log.hpp"
#include "stb_image.h"

namespace Cubify::DX12
{
    static ID3D12PipelineState* ToPipelineState(const IPipeline* pipeline)
    {
        return pipeline ? static_cast<const DX12Pipeline*>(pipeline)->pipelineState() : nullptr;
    }

    // Safe: DX12Device only ever creates DX12SwapChains
    static DX12SwapChain& ToDX12SwapChain(ISwapChain& swapChain)
    {
        return static_cast<DX12SwapChain&>(swapChain);
    }

    static DX12Texture& ToDX12Texture(ITexture& texture)
    {
        return static_cast<DX12Texture&>(texture);
    }

    // Layer order defines the indices the mesher packs into the quads
    static constexpr const char* BLOCK_TEXTURE_PATHS[] = {
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

    // Frees the decoded pixels once the texture has been created from them
    struct BlockTextureLayers
    {
        std::vector<unsigned char*> pixels;
        int width = 0;
        int height = 0;

        ~BlockTextureLayers()
        {
            for (unsigned char* layer : pixels)
            {
                stbi_image_free(layer);
            }
        }
    };

    static bool LoadBlockTextures(BlockTextureLayers& layers)
    {
        stbi_set_flip_vertically_on_load(true);

        for (const char* path : BLOCK_TEXTURE_PATHS)
        {
            int w = 0, h = 0, comp = 0;
            unsigned char* pixels = stbi_load(path, &w, &h, &comp, 4);
            if (!pixels)
            {
                LOGE("[DX12Renderer] Failed to load texture layer: %s", path);
                return false;
            }

            layers.pixels.push_back(pixels);
            if (layers.pixels.size() == 1)
            {
                layers.width = w;
                layers.height = h;
            }
            else if (w != layers.width || h != layers.height)
            {
                LOGE("[DX12Renderer] Texture layer size mismatch: %s", path);
                return false;
            }
        }

        return true;
    }

    DX12Renderer::DX12Renderer(DX12Device& device, void* windowHandle, int width, int height)
        : m_device(device.device())
        , m_commandQueue(device.commandQueue())
        , m_rootSignature(device.rootSignature())
    {
        m_swapChain = device.createSwapChain({
            .windowHandle = windowHandle,
            .width = width,
            .height = height,
        });
        m_currentFrame = ToDX12SwapChain(*m_swapChain).currentBackBufferIndex();
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
        m_meshStore = std::make_unique<DX12MeshStore>(m_device.Get());

        BlockTextureLayers blockTextures;
        if (LoadBlockTextures(blockTextures))
        {
            TextureDesc textureDesc{
                .debugName = "Block Textures",
                .width = blockTextures.width,
                .height = blockTextures.height,
            };
            textureDesc.layers.assign(blockTextures.pixels.begin(), blockTextures.pixels.end());
            m_blockTextures = device.createTexture(textureDesc);
        }
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
        if (width == m_swapChain->width() && height == m_swapChain->height())
        {
            return;
        }

        // Careful about back buffers still having work
        WaitForGpu();

        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            m_fenceValues[i] = m_fenceValues[m_currentFrame];
        }

        m_swapChain->resize(width, height);
        m_currentFrame = ToDX12SwapChain(*m_swapChain).currentBackBufferIndex();
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
        m_meshStore->processDeletions(m_deferredReleases[m_currentFrame]);

        DX12SwapChain& swapChain = ToDX12SwapChain(*m_swapChain);

        // Update the back buffer state to be writable before rendering
        CD3DX12_RESOURCE_BARRIER toRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
            swapChain.backBuffer(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        m_commandList->ResourceBarrier(1, &toRenderTarget);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain.rtv();
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = swapChain.dsv();
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        const float clearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        CD3DX12_VIEWPORT viewport(0.0f, 0.0f,
            static_cast<float>(swapChain.width()), static_cast<float>(swapChain.height()));
        CD3DX12_RECT scissor(0, 0, swapChain.width(), swapChain.height());
        m_commandList->RSSetViewports(1, &viewport);
        m_commandList->RSSetScissorRects(1, &scissor);

        if (m_rootSignature)
        {
            m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
            m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            if (m_blockTextures)
            {
                DX12Texture& textures = ToDX12Texture(*m_blockTextures);

                // The heap must be bound before the table that points into it
                ID3D12DescriptorHeap* heaps[] = { textures.srvHeap() };
                m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
                m_commandList->SetGraphicsRootDescriptorTable(
                    DX12Pipeline::ROOT_PARAM_TEXTURES, textures.srv());
            }
        }
    }

    void DX12Renderer::endFrame()
    {
        CD3DX12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(
            ToDX12SwapChain(*m_swapChain).backBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        );

        m_commandList->ResourceBarrier(1, &toPresent);
        m_commandList->Close();
        m_commandListOpen = false;
        ID3D12CommandList* commandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        m_swapChain->present();

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

        const bool saved = SaveBackBufferToPng(m_device.Get(), m_commandQueue.Get(), m_commandList.Get(),
            ToDX12SwapChain(*m_swapChain).backBuffer(), filePath);

        m_commandList->Reset(m_commandAllocators[m_currentFrame].Get(), ToPipelineState(m_solidPipeline.get()));
        return saved;
    }

    void DX12Renderer::setWorldSettings(const WorldSettings& settings) {}

    void DX12Renderer::uploadMesh(MeshHandle& mesh, const std::vector<Quad>& quads)
    {
        if (!quads.empty() && !m_commandListOpen)
        {
            LOGE("[DX12Renderer] uploadMesh called outside of a frame, skipping");
            return;
        }

        m_meshStore->upload(mesh, quads, m_commandList.Get(), m_deferredReleases[m_currentFrame]);
    }

    void DX12Renderer::draw(MeshId mesh, const glm::mat4& model)
    {
        const DX12MeshStore::DrawInfo info = m_meshStore->drawInfo(mesh);
        if (info.quadCount == 0)
        {
            return;
        }

        m_commandList->SetGraphicsRoot32BitConstants(
            DX12Pipeline::ROOT_PARAM_MODEL, DX12Pipeline::MATRIX_CONSTANT_COUNT, &model, 0);
        m_commandList->SetGraphicsRootShaderResourceView(DX12Pipeline::ROOT_PARAM_QUADS, info.address);

        // One instance per quad, the shader builds its six corners
        m_commandList->DrawInstanced(VERTICES_PER_QUAD, info.quadCount, 0, 0);
    }

    // DX12 Initialization starts there

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

        m_currentFrame = ToDX12SwapChain(*m_swapChain).currentBackBufferIndex();

        // Only wait if the GPU is still busy with the frame we are about to reuse
        if (m_fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
        {
            m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_fenceValues[m_currentFrame] = currentValue + 1;
    }
}