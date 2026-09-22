#include "DX12SwapChain.hpp"

#include "d3dx12.h"
#include "Graphics/DirectX12Backend/DX12Common.hpp"

namespace Cubify::DX12
{
    DX12SwapChain::DX12SwapChain(DX12Device& device, void* windowHandle, int width, int height)
        : m_device(device.device()), m_width(width), m_height(height)
    {
        CreateSwapChain(device.factory(), device.commandQueue(), windowHandle);
        CreateRtvHeap();
        CreateRenderTargets();
        CreateDsvHeap();
        CreateDepthStencil();
    }

    // The caller has to wait for the GPU before resizing
    void DX12SwapChain::resize(int width, int height)
    {
        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            m_renderTargets[i].Reset();
        }

        m_swapChain->ResizeBuffers(
            FRAME_COUNT,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            BACK_BUFFER_FORMAT,
            m_swapChainFlags);

        m_currentBuffer = m_swapChain->GetCurrentBackBufferIndex();
        m_width = width;
        m_height = height;

        // RTVs need to be recreated
        CreateRenderTargets();

        m_depthStencil.Reset();
        CreateDepthStencil();
    }

    void DX12SwapChain::present()
    {
        UINT presentFlags = (m_swapChainFlags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
            ? DXGI_PRESENT_ALLOW_TEARING
            : 0;

        HRESULT presentResult = m_swapChain->Present(0, presentFlags);
        if (FAILED(presentResult))
        {
            LOGE("[DX12SwapChain] Failed to present swap chain");
            LOGE("[DX12SwapChain] hr = 0x%08X", presentResult);
        }

        m_currentBuffer = m_swapChain->GetCurrentBackBufferIndex();
    }

    UINT DX12SwapChain::currentBackBufferIndex() const
    {
        return m_currentBuffer;
    }

    ID3D12Resource* DX12SwapChain::backBuffer() const
    {
        return m_renderTargets[m_currentBuffer].Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12SwapChain::rtv() const
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            static_cast<INT>(m_currentBuffer), m_rtvDescriptorSize);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12SwapChain::dsv() const
    {
        return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    int DX12SwapChain::width() const
    {
        return m_width;
    }

    int DX12SwapChain::height() const
    {
        return m_height;
    }

    void DX12SwapChain::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* queue, void* windowHandle)
    {
        HWND hwnd = static_cast<HWND>(windowHandle);

        BOOL allowTearing = FALSE;
        factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        m_swapChainFlags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        DXGI_SWAP_CHAIN_DESC1 desc{
            .Width = static_cast<UINT>(m_width),
            .Height = static_cast<UINT>(m_height),
            .Format = BACK_BUFFER_FORMAT,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = FRAME_COUNT,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .Flags = m_swapChainFlags
        };

        ComPtr<IDXGISwapChain1> oldSwapChain;
        HR_CHECK(factory->CreateSwapChainForHwnd(
            queue, hwnd, &desc, nullptr, nullptr, &oldSwapChain
        ), "[DX12SwapChain] Failed to create swap chain");

        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        HR_CHECK(oldSwapChain.As(&m_swapChain),
            "[DX12SwapChain] Failed to query IDXGISwapChain3");

        m_currentBuffer = m_swapChain->GetCurrentBackBufferIndex();
        LOGI("[DX12SwapChain] Swap chain created successfully");
    }

    void DX12SwapChain::CreateRtvHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = FRAME_COUNT,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        HR_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap)),
            "[DX12SwapChain] Failed to create RTV descriptor heap");
        SetDebugName(m_rtvHeap.Get(), L"RTV Heap");
        LOGI("[DX12SwapChain] RTV descriptor heap created successfully");

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    void DX12SwapChain::CreateRenderTargets()
    {
        // create view for each buffer in swap chain
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT i = 0; i < FRAME_COUNT; ++i)
        {
            HR_CHECK(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])),
                "[DX12SwapChain] Failed to get swap chain buffer %u", i);

            std::wstring rtName = L"Back Buffer " + std::to_wstring(i);
            SetDebugName(m_renderTargets[i].Get(), rtName.c_str());

            m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
        LOGI("[DX12SwapChain] Render targets created successfully");
    }

    void DX12SwapChain::CreateDsvHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        HR_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_dsvHeap)),
            "[DX12SwapChain] Failed to create DSV descriptor heap");
        SetDebugName(m_dsvHeap.Get(), L"DSV Heap");
        LOGI("[DX12SwapChain] DSV descriptor heap created successfully");
    }

    void DX12SwapChain::CreateDepthStencil()
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
            "[DX12SwapChain] Failed to create depth stencil buffer");

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{
            .Format = DEPTH_FORMAT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Flags = D3D12_DSV_FLAG_NONE
        };

        m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        SetDebugName(m_depthStencil.Get(), L"Depth Buffer");
        LOGI("[DX12SwapChain] Depth stencil buffer created successfully");
    }
}
