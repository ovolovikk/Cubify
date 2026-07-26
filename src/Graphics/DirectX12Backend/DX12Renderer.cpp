#include "DX12Renderer.hpp"

#include "d3dx12.h"
#include <dxcapi.h>
#include "Core/Logging/Log.hpp"

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
		HRESULT hr = utils->LoadFile(path, nullptr, &sourceBlob);
        if (FAILED(hr))
		{
			LOGE("[DX12Renderer] Failed to load shader file: %ls", path);
			return nullptr;
		}

		std::vector<LPCWSTR> arguments = {
            path,
			L"-E", entry,
			L"-T", target,
			L"-Qstrip_reflect",
		};

#if defined (_DEBUG)
		arguments.push_back(L"-Zi");
        arguments.push_back(L"-Od");
#else
		arguments.push_back(L"-O3");
#endif
        DxcBuffer sourceBuffer{
            .Ptr = sourceBlob->GetBufferPointer(),
            .Size = sourceBlob->GetBufferSize(),
            .Encoding = DXC_CP_ACP
        };

        ComPtr<IDxcResult> result;
        hr = compiler->Compile(
            &sourceBuffer,
            arguments.data(),
            static_cast<UINT32>(arguments.size()),
            includeHandler.Get(),
            IID_PPV_ARGS(&result)
        );

        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Internal DXC compiler error.\n");
            return nullptr;
        }
        
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
        : m_width(width), m_height(height)
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
            m_fenceValues[i] = m_fenceValues[m_frameIndex];
        }

        m_swapChain->ResizeBuffers(
            FRAME_COUNT,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            DXGI_FORMAT_R8G8B8A8_UNORM,
            m_swapChainFlags);

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
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
		m_commandAllocators[m_frameIndex]->Reset();
		m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get());

		// Update the back buffer state to be writable before rendering
        CD3DX12_RESOURCE_BARRIER toRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &toRenderTarget);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        const float clearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
        CD3DX12_RECT scissor(0, 0, m_width, m_height);
        m_commandList->RSSetViewports(1, &viewport);
        m_commandList->RSSetScissorRects(1, &scissor);

        // temp just draw triangle
        if (m_rootSignature && m_pipelineState)
        {
            m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
            m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_commandList->DrawInstanced(3, 1, 0, 0);
        }
    }

    void DX12Renderer::endFrame()
    {
        CD3DX12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier(1, &toPresent);
        m_commandList->Close();

		ID3D12CommandList* lists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

        // Vsync on
        m_swapChain->Present(1, 0);

        MoveToNextFrame();
    }

    void DX12Renderer::beginTransparentPass() {}
    void DX12Renderer::endTransparentPass() {}

    void DX12Renderer::setViewProjection(const glm::mat4& view, const glm::mat4& projection) {}
    void DX12Renderer::setWorldSettings(const WorldSettings& settings) {}

    void DX12Renderer::uploadMesh(MeshHandle& mesh, const std::vector<Quad>& quads) {}
    void DX12Renderer::draw(MeshId mesh, const glm::mat4& model) {}


    // DX12 Initialization starts there

    void DX12Renderer::CreateDebugController()
    {
#if defined(_DEBUG)
        HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to get D3D12 debug interface");
            return;
        }
        m_debugController->EnableDebugLayer();
#endif
    }

    void DX12Renderer::CreateFactory()
    {
        UINT factoryFlags = 0;
#if defined(_DEBUG)
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create DXGI factory");
        }
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
        HRESULT hr = D3D12CreateDevice(
            m_adapter.Get(),
            D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&m_device)
        );
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create D3D12 device");
            return;
        }
        LOGI("[DX12Renderer] D3D12 device created successfully");
    }

    void DX12Renderer::CreateCommandQueue()
    {
        D3D12_COMMAND_QUEUE_DESC desc{
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        };

        HRESULT hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create command queue");
            return;
        }
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
        HRESULT hr = m_factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(), hwnd, &desc, nullptr, nullptr, &oldSwapChain
        );
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create swap chain");
            return;
        }

        m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        if (SUCCEEDED(oldSwapChain.As(&m_swapChain)))
        {
            m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
        }
        LOGI("[DX12Renderer] Swap chain created successfully");
    }

    void DX12Renderer::CreateRtvHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = FRAME_COUNT,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create RTV descriptor heap");
            return;
        }
        LOGI("[DX12Renderer] RTV descriptor heap created successfully");

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    void DX12Renderer::CreateRenderTargets()
    {
        // create view for each buffer in swap chain
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            HRESULT hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
            if (FAILED(hr))
            {
                LOGE("[DX12Renderer] Failed to get swap chain buffer %u", i);
                return;
            }

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

        HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_dsvHeap));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create DSV descriptor heap");
            return;
        }
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

        HRESULT hr = m_device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_depthStencil));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create depth stencil buffer");
            return;
        }

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{
            .Format = DEPTH_FORMAT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Flags = D3D12_DSV_FLAG_NONE
        };

        m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        LOGI("[DX12Renderer] Depth stencil buffer created successfully");
    }

    void DX12Renderer::CreateCommandObjects()
    {

        // Command Allocator
        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            HRESULT hr = m_device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
            if (FAILED(hr))
            {
                LOGE("[DX12Renderer] Failed to create command allocator %u", i);
                return;
            }
        }

        // Command List
        HRESULT hr = m_device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_commandAllocators[m_frameIndex].Get(),
            nullptr,
            IID_PPV_ARGS(&m_commandList));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create command list");
            return;
        }

        m_commandList->Close();
        LOGI("[DX12Renderer] Command objects created successfully");
    }

    void DX12Renderer::CreateFence()
    {
        HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create fence");
            return;
        }
        m_fenceValues[m_frameIndex] = 1;
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            LOGE("[DX12Renderer] Failed to create fence event");
        }
        LOGI("[DX12Renderer] Fence created successfully");
    }

    void DX12Renderer::CreateRootSignature()
    {
        // For now root signature is empty
        // because i've made resources stored in shader
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
		desc.Init_1_1(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

		ComPtr<ID3DBlob> serialized;
        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DX12SerializeVersionedRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_1, &serialized, &errors);
        if (FAILED(hr))
        {
			LOGE("[DX12Renderer] Failed to initialize Root Signature: %s", static_cast<const char*>(errors->GetBufferPointer()));
            return;
        }

        hr = m_device->CreateRootSignature(
            0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature));
		if (FAILED(hr))
		{
			LOGE("[DX12Renderer] Failed to create root signature");
			return;
		}
		LOGI("[DX12Renderer] Root signature created successfully");
    }

    void DX12Renderer::CreatePipelineState()
    {
        ComPtr<IDxcBlob> vs = CompileShader(L"shaders/dx12/shader.hlsl", L"VSMain", L"vs_6_0");
		ComPtr<IDxcBlob> ps = CompileShader(L"shaders/dx12/shader.hlsl", L"PSMain", L"ps_6_0");
		if (!vs || !ps)
		{
			LOGE("[DX12Renderer] Skipping PSO creation: shader compilation failed");
			return;
		}

		CD3DX12_RASTERIZER_DESC rasterizer(D3D12_DEFAULT);
        rasterizer.CullMode = D3D12_CULL_MODE_NONE;

		CD3DX12_DEPTH_STENCIL_DESC1 depthStencil(D3D12_DEFAULT);
		depthStencil.DepthEnable = FALSE;

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

        HRESULT hr = m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState));
        if (FAILED(hr))
        {
            LOGE("[DX12Renderer] Failed to create pipeline state");
            return;
        }
        LOGI("[DX12Renderer] Pipeline state created successfully");
    }

    void DX12Renderer::WaitForGpu()
    {
        const UINT64 value = m_fenceValues[m_frameIndex];
        m_commandQueue->Signal(m_fence.Get(), value);

        m_fence->SetEventOnCompletion(value, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);

        m_fenceValues[m_frameIndex]++;
    }

    void DX12Renderer::MoveToNextFrame()
    {
        const UINT64 currentValue = m_fenceValues[m_frameIndex];
        m_commandQueue->Signal(m_fence.Get(), currentValue);

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Only wait if the GPU is still busy with the frame we are about to reuse
        if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
        {
            m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_fenceValues[m_frameIndex] = currentValue + 1;
    }
}