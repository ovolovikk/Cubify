#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Graphics/IGraphicsDevice.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12Device : public IGraphicsDevice
    {
    public:
        DX12Device();

        GraphicsApi api() const override { return GraphicsApi::DirectX12; }

        IDXGIFactory7* factory() const { return m_factory.Get(); }
        ID3D12Device2* device() const { return m_device.Get(); }
        ID3D12CommandQueue* commandQueue() const { return m_commandQueue.Get(); }

    private:
        void CreateDebugController();
        void CreateFactory();
        void SelectAdapter();
        void CreateDevice();
        void CreateCommandQueue();

        ComPtr<ID3D12Debug> m_debugController;
        ComPtr<IDXGIFactory7> m_factory;
        ComPtr<IDXGIAdapter4> m_adapter;
        ComPtr<ID3D12Device2> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
    };
}
