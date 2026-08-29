#include "DX12Renderer.hpp"

#include "d3dx12.h"
#include <dxcapi.h>
#include "Core/Logging/Log.hpp"
#include "stb_image.h"

#define HR_CHECK(expr, ...)                                 \
    {                                                       \
        HRESULT hrResult = (expr);                          \
        if (FAILED(hrResult))                               \
        {                                                   \
            LOGE(__VA_ARGS__);                              \
            LOGE("[DX12Renderer] hr = 0x%08X", hrResult);   \
            return;                                         \
        }                                                   \
    }

#define HR_FALLBACK(expr, returnVal, ...)                   \
    {                                                       \
        HRESULT hrResult = (expr);                          \
        if (FAILED(hrResult))                               \
        {                                                   \
            LOGE(__VA_ARGS__);                              \
            LOGE("[DX12Renderer] hr = 0x%08X", hrResult);   \
            return returnVal;                               \
        }                                                   \
    }

namespace Cubify::DX12
{
    static ComPtr<IDxcBlob> CompileShader(const wchar_t* path, const wchar_t* entry, const wchar_t* target)
    {
        static ComPtr<IDxcUtils> utils;
        static ComPtr<IDxcCompiler3> compiler;
        static ComPtr<IDxcIncludeHandler> includeHandler;
        
        if(!utils)
        {
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
			utils->CreateDefaultIncludeHandler(&includeHandler);
        }

        ComPtr<IDxcBlobEncoding> sourceBlob;
		HR_FALLBACK(utils->LoadFile(path, nullptr, &sourceBlob), nullptr,
			"[DX12Renderer] Failed to load shader file: %ls", path);

		std::vector<LPCWSTR> arguments = {
            path,
			L"-E", entry,
			L"-T", target,
			// Lets the stage files pull in common.hlsli by bare name
			L"-I", L"shaders/dx12",
		};

#if defined (_DEBUG)
		arguments.push_back(L"-Zi");
		arguments.push_back(L"-Qembed_debug");
        arguments.push_back(L"-Od");
#else
		arguments.push_back(L"-Qstrip_reflect");
		arguments.push_back(L"-O3");
#endif
        DxcBuffer sourceBuffer{
            .Ptr = sourceBlob->GetBufferPointer(),
            .Size = sourceBlob->GetBufferSize(),
            .Encoding = DXC_CP_ACP
        };

        ComPtr<IDxcResult> result;
        HR_FALLBACK(compiler->Compile(
            &sourceBuffer,
            arguments.data(),
            static_cast<UINT32>(arguments.size()),
            includeHandler.Get(),
            IID_PPV_ARGS(&result)
        ), nullptr, "[DX12Renderer] Internal DXC compiler error.");

        ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

        if (errors != nullptr && errors->GetStringLength() > 0)
        {
            LOGE("Shader compile log (%ls):\n%s\n", entry, errors->GetStringPointer());
        }

        HRESULT status;
        result->GetStatus(&status);
        if (FAILED(status))
        {
            return nullptr;
        }

        ComPtr<IDxcBlob> shader;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);

        return shader;
    }

    DX12Renderer::DX12Renderer(void* windowHandle, int width, int height)
        : m_deletionQueue(std::make_shared<DX12DeletionQueue>()), m_width(width), m_height(height)
    {
        CreateDebugController();
        CreateFactory();
        SelectAdapter();
        CreateDevice();
        CreateCommandQueue();
        CreateSwapChain(windowHandle, width, height);
        CreateRtvHeap();
        CreateRenderTargets();
        CreateDsvHeap();
        CreateDepthStencil();
        CreateCommandObjects();
        CreateFence();
        CreateRootSignature();
        CreatePipelineState();
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
            DXGI_FORMAT_R8G8B8A8_UNORM,
            m_swapChainFlags);

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();
        m_width = width;
        m_height = height;

        // RTVs need to be recreated
        CreateRenderTargets();

        m_depthStencil.Reset();
        CreateDepthStencil();
    }

    void DX12Renderer::onResize(int width, int height)
    {
        resize(width, height);
    }

    void DX12Renderer::beginFrame()
    {
        if (m_commandListOpen)
        {
            return;
        }

        m_commandAllocators[m_currentFrame].Get()->Reset();
		m_commandList->Reset(m_commandAllocators[m_currentFrame].Get(), m_pipelineState.Get());
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
                    ROOT_PARAM_TEXTURES, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
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
        if (m_transparentPipelineState)
        {
            m_commandList->SetPipelineState(m_transparentPipelineState.Get());
        }
    }

    void DX12Renderer::endTransparentPass()
    {
        if (m_pipelineState)
        {
            m_commandList->SetPipelineState(m_pipelineState.Get());
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
            m_commandList->SetGraphicsRoot32BitConstants(ROOT_PARAM_VIEW_PROJ, MATRIX_CONSTANT_COUNT, &m_viewProj, 0);
        }
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
            ROOT_PARAM_MODEL, MATRIX_CONSTANT_COUNT, &model, 0);
        m_commandList->SetGraphicsRootShaderResourceView(
            ROOT_PARAM_QUADS, it->second.buffer->GetGPUVirtualAddress());

        // One instance per quad, the shader builds its six corners
        m_commandList->DrawInstanced(VERTICES_PER_QUAD, it->second.quadCount, 0, 0);
    }

    void DX12Renderer::SetDebugName(ID3D12Object* object, const wchar_t* name)
    {
        if (object)
        {
            object->SetName(name);
        }
    }

    // DX12 Initialization starts there

    void DX12Renderer::CreateDebugController()
    {
#if defined(_DEBUG)
        HR_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)),
            "[DX12Renderer] Failed to get D3D12 debug interface");
        m_debugController->EnableDebugLayer();
#endif
    }

    void DX12Renderer::CreateFactory()
    {
        UINT factoryFlags = 0;
#if defined(_DEBUG)
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        HR_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)),
            "[DX12Renderer] Failed to create DXGI factory");
        LOGI("[DX12Renderer] DXGI factory created successfully");
    }

    void DX12Renderer::SelectAdapter()
    {
        for (UINT i = 0;
            SUCCEEDED(m_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)));
            ++i)
        {
            DXGI_ADAPTER_DESC3 desc;
            if (SUCCEEDED(m_adapter->GetDesc3(&desc)))
            {
                LOGI("[DX12Renderer] Adapter %u: %ls, VRAM: %zu MB", i, desc.Description, desc.DedicatedVideoMemory / (1024 * 1024));
            }
            if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                LOGI("[DX12Renderer] Selected adapter: %ls", desc.Description);
                return;
            }
        }
        LOGE("[DX12Renderer] Failed to find a suitable adapter");
        return;
    }

    void DX12Renderer::CreateDevice()
    {
        HR_CHECK(D3D12CreateDevice(
            m_adapter.Get(),
            D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&m_device)
        ), "[DX12Renderer] Failed to create D3D12 device");
        SetDebugName(m_device.Get(), L"Device");
        LOGI("[DX12Renderer] D3D12 device created successfully");
    }

    void DX12Renderer::CreateCommandQueue()
    {
        D3D12_COMMAND_QUEUE_DESC desc{
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        };

        HR_CHECK(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue)),
            "[DX12Renderer] Failed to create command queue");
        SetDebugName(m_commandQueue.Get(), L"Direct Queue");
        LOGI("[DX12Renderer] Command queue created successfully");
    }

    void DX12Renderer::CreateSwapChain(void* windowHandle, int width, int height)
    {
        HWND hwnd = static_cast<HWND>(windowHandle);

        BOOL allowTearing = FALSE;
        m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        m_swapChainFlags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        DXGI_SWAP_CHAIN_DESC1 desc{
            .Width = static_cast<UINT>(width),
            .Height = static_cast<UINT>(height),
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
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

    void DX12Renderer::CreateRootSignature()
    {
        // 35/64 DWORD space used, a root SRV costs 2 and a table costs 1
        CD3DX12_ROOT_PARAMETER1 params[4]{};
        params[ROOT_PARAM_VIEW_PROJ].InitAsConstants(MATRIX_CONSTANT_COUNT, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        params[ROOT_PARAM_MODEL].InitAsConstants(MATRIX_CONSTANT_COUNT, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        // Root descriptor takes a GPU address directly, so no descriptor heap is
        // needed for the geometry buffer
        params[ROOT_PARAM_QUADS].InitAsShaderResourceView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

        CD3DX12_DESCRIPTOR_RANGE1 textureRange;
        textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
        params[ROOT_PARAM_TEXTURES].InitAsDescriptorTable(
            1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_STATIC_SAMPLER_DESC sampler(
            0,
            D3D12_FILTER_MIN_MAG_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
		desc.Init_1_1(_countof(params), params, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

		ComPtr<ID3DBlob> serialized;
        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DX12SerializeVersionedRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_1, &serialized, &errors);
        if (FAILED(hr) && errors)
        {
			LOGE("[DX12Renderer] Root signature error: %s", static_cast<const char*>(errors->GetBufferPointer()));
        }
        HR_CHECK(hr, "[DX12Renderer] Failed to initialize Root Signature");

        HR_CHECK(m_device->CreateRootSignature(
            0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)),
            "[DX12Renderer] Failed to create root signature");
		SetDebugName(m_rootSignature.Get(), L"Main Root Signature");
		LOGI("[DX12Renderer] Root signature created successfully");
    }

    void DX12Renderer::CreatePipelineState()
    {
        ComPtr<IDxcBlob> vs = CompileShader(L"shaders/dx12/vertex_shader.hlsl", L"VSMain", L"vs_6_0");
		ComPtr<IDxcBlob> ps = CompileShader(L"shaders/dx12/pixel_shader.hlsl", L"PSMain", L"ps_6_0");
		if (!vs || !ps)
		{
			LOGE("[DX12Renderer] Skipping PSO creation: shader compilation failed");
			return;
		}

		CD3DX12_RASTERIZER_DESC rasterizer(D3D12_DEFAULT);
        rasterizer.FrontCounterClockwise = TRUE;
        rasterizer.CullMode = D3D12_CULL_MODE_BACK;

		CD3DX12_DEPTH_STENCIL_DESC1 depthStencil(D3D12_DEFAULT);
		depthStencil.DepthEnable = TRUE;

        D3D12_RT_FORMAT_ARRAY rtvFormats{
            .RTFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
            .NumRenderTargets = 1
        };

        CD3DX12_PIPELINE_STATE_STREAM1 stream;
        stream.pRootSignature = m_rootSignature.Get();
        stream.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
        stream.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
        stream.RasterizerState = rasterizer;
        stream.DepthStencilState = depthStencil;
        stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        stream.RTVFormats = rtvFormats;
        stream.DSVFormat = DEPTH_FORMAT;
        stream.SampleDesc = DXGI_SAMPLE_DESC{ .Count = 1, .Quality = 0 };

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{
            .SizeInBytes = sizeof(stream),
            .pPipelineStateSubobjectStream = &stream
        };

        HR_CHECK(m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)),
            "[DX12Renderer] Failed to create pipeline state");
        SetDebugName(m_pipelineState.Get(), L"Solid PSO");
        LOGI("[DX12Renderer] Pipeline state created successfully");

        // Transparent variant. Luna chapter 9, 10 to experiment
        CD3DX12_BLEND_DESC blend(D3D12_DEFAULT);
        blend.RenderTarget[0].BlendEnable = TRUE;
        blend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

        CD3DX12_DEPTH_STENCIL_DESC1 transparentDepth(D3D12_DEFAULT);
        transparentDepth.DepthEnable = TRUE;
        transparentDepth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

        CD3DX12_RASTERIZER_DESC transparentRasterizer(rasterizer);
        transparentRasterizer.CullMode = D3D12_CULL_MODE_NONE;

        stream.BlendState = blend;
        stream.DepthStencilState = transparentDepth;
        stream.RasterizerState = transparentRasterizer;

        HR_CHECK(m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_transparentPipelineState)),
            "[DX12Renderer] Failed to create transparent pipeline state");
        SetDebugName(m_transparentPipelineState.Get(), L"Transparent Water PSO");
        LOGI("[DX12Renderer] Transparent pipeline state created successfully");
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