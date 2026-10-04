#include "DX12Device.hpp"

#include "Graphics/DirectX12Backend/DX12Common.hpp"
#include "Graphics/DirectX12Backend/DX12Pipeline.hpp"
#include "Graphics/DirectX12Backend/DX12Renderer.hpp"
#include "Graphics/DirectX12Backend/DX12SwapChain.hpp"
#include "Graphics/DirectX12Backend/DX12Texture.hpp"

namespace Cubify::DX12
{
    DX12Device::DX12Device()
    {
        CreateDebugController();
        CreateFactory();
        SelectAdapter();
        CreateDevice();
        CreateCommandQueue();
        m_rootSignature = DX12Pipeline::CreateRootSignature(m_device.Get());
    }

    DX12Device::~DX12Device()
    {
        m_flushFence.Reset();
        m_rootSignature.Reset();
        m_commandQueue.Reset();
        m_device.Reset();
        m_adapter.Reset();
        m_factory.Reset();
        m_debugController.Reset();

        LogLiveObjects();
    }

    std::unique_ptr<IRendererBackend> DX12Device::createRenderer(void* windowHandle, int width, int height)
    {
        return std::make_unique<DX12Renderer>(*this, windowHandle, width, height);
    }

    std::unique_ptr<DX12Pipeline> DX12Device::createPipeline(const PipelineDesc& desc)
    {
        auto pipeline = std::make_unique<DX12Pipeline>(m_device.Get(), m_rootSignature.Get(), desc);
        if (!pipeline->pipelineState())
        {
            return nullptr;
        }
        return pipeline;
    }

    std::unique_ptr<DX12SwapChain> DX12Device::createSwapChain(const SwapChainDesc& desc)
    {
        auto swapChain = std::make_unique<DX12SwapChain>(*this, desc);
        if (!swapChain->backBuffer())
        {
            return nullptr;
        }
        return swapChain;
    }

    std::unique_ptr<DX12Texture> DX12Device::createTexture(const TextureDesc& desc)
    {
        auto texture = std::make_unique<DX12Texture>(*this, desc);
        if (!texture->isValid())
        {
            return nullptr;
        }
        return texture;
    }

    void DX12Device::flush()
    {
        const UINT64 value = ++m_flushValue;
        m_commandQueue->Signal(m_flushFence.Get(), value);
        // Docs says that with no even it will block until fence reaches the value
        m_flushFence->SetEventOnCompletion(value, nullptr);
    }

    IDXGIFactory7* DX12Device::factory() const
    {
        return m_factory.Get();
    }

    ID3D12Device2* DX12Device::device() const
    {
        return m_device.Get();
    }

    ID3D12CommandQueue* DX12Device::commandQueue() const
    {
        return m_commandQueue.Get();
    }

    ID3D12RootSignature* DX12Device::rootSignature() const
    {
        return m_rootSignature.Get();
    }

    void DX12Device::CreateDebugController()
    {
#if defined(_DEBUG)
        HR_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)),
            "[DX12Device] Failed to get D3D12 debug interface");
        m_debugController->EnableDebugLayer();
#endif
    }

    void DX12Device::CreateFactory()
    {
        UINT factoryFlags = 0;
#if defined(_DEBUG)
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        HR_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)),
            "[DX12Device] Failed to create DXGI factory");
        LOGI("[DX12Device] DXGI factory created successfully");
    }

    void DX12Device::SelectAdapter()
    {
        for (UINT i = 0;
            SUCCEEDED(m_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)));
            ++i)
        {
            DXGI_ADAPTER_DESC3 desc{};
            if (SUCCEEDED(m_adapter->GetDesc3(&desc)))
            {
                LOGI("[DX12Device] Adapter %u: %ls, VRAM: %zu MB", i, desc.Description, desc.DedicatedVideoMemory / (1024 * 1024));
            }
            if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                LOGI("[DX12Device] Selected adapter: %ls", desc.Description);
                return;
            }
        }
        // CI runners have no GPU at all. WARP is the software adapter that ships
        // with Windows and supports feature level 12_1, but it's slow
        LOGW("[DX12Device] No hardware adapter found, falling back to WARP");
        if (SUCCEEDED(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&m_adapter))))
        {
            if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                LOGI("[DX12Device] Selected adapter: WARP software renderer");
                return;
            }
        }

        LOGE("[DX12Device] Failed to find a suitable adapter");
    }

    void DX12Device::CreateDevice()
    {
        HR_CHECK(D3D12CreateDevice(
            m_adapter.Get(),
            D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&m_device)
        ), "[DX12Device] Failed to create D3D12 device");
        SetDebugName(m_device.Get(), L"Device");
        LOGI("[DX12Device] D3D12 device created successfully");

#if defined(_DEBUG)
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (IsDebuggerPresent() && SUCCEEDED(m_device.As(&infoQueue)))
        {
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            // Warnings are intentially ignored to not bother with filtering for now
            // TODO: Maybe add them in future
        }
#endif
    }

    void DX12Device::CreateCommandQueue()
    {
        D3D12_COMMAND_QUEUE_DESC desc{
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        };

        HR_CHECK(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue)),
            "[DX12Device] Failed to create command queue");
        SetDebugName(m_commandQueue.Get(), L"Direct Queue");

        HR_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_flushFence)),
            "[DX12Device] Failed to create flush fence");
        LOGI("[DX12Device] Command queue created successfully");
    }
}
