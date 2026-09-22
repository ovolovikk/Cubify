#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Graphics/DirectX12Backend/DX12Device.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12SwapChain
    {
    public:
        static constexpr UINT FRAME_COUNT = 2;

        DX12SwapChain(DX12Device& device, void* windowHandle, int width, int height);

        void resize(int width, int height);
        void present();

        UINT currentBackBufferIndex() const;
        ID3D12Resource* backBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE rtv() const;
        D3D12_CPU_DESCRIPTOR_HANDLE dsv() const;
        int width() const;
        int height() const;

    private:
        void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* queue, void* windowHandle);
        void CreateRtvHeap();
        void CreateRenderTargets();
        void CreateDsvHeap();
        void CreateDepthStencil();

        ComPtr<ID3D12Device2> m_device; // shared with DX12Device
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
        ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        ComPtr<ID3D12Resource> m_depthStencil;

        UINT m_rtvDescriptorSize = 0;
        UINT m_swapChainFlags = 0;
        UINT m_currentBuffer = 0;
        int m_width = 0;
        int m_height = 0;
    };
}
