#include "DX12Device.hpp"

#include "Graphics/DirectX12Backend/DX12Common.hpp"
#include "Graphics/DirectX12Backend/DX12Pipeline.hpp"

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

    std::unique_ptr<IPipeline> DX12Device::createPipeline(const PipelineDesc& desc)
    {
        auto pipeline = std::make_unique<DX12Pipeline>(m_device.Get(), m_rootSignature.Get(), desc);
        if (!pipeline->pipelineState())
        {
            return nullptr;
        }
        return pipeline;
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
        LOGI("[DX12Device] Command queue created successfully");
    }
}
