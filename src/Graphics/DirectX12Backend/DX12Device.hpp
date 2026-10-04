#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Graphics/IGraphicsDevice.hpp"
#include "Graphics/IPipeline.hpp"
#include "Graphics/ISwapChain.hpp"
#include "Graphics/ITexture.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12Pipeline;
    class DX12SwapChain;
    class DX12Texture;

    class DX12Device : public IGraphicsDevice
    {
    public:
        DX12Device();
        ~DX12Device() override;

        std::unique_ptr<IRendererBackend> createRenderer(void* windowHandle, int width, int height) override;

        // Return nullptr if creation fails, e.g. a shader that doesn't compile
        std::unique_ptr<DX12Pipeline> createPipeline(const PipelineDesc& desc);
        std::unique_ptr<DX12SwapChain> createSwapChain(const SwapChainDesc& desc);
        std::unique_ptr<DX12Texture> createTexture(const TextureDesc& desc);

        void flush();

        IDXGIFactory7* factory() const;
        ID3D12Device2* device() const;
        ID3D12CommandQueue* commandQueue() const;
        ID3D12RootSignature* rootSignature() const;

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
        ComPtr<ID3D12RootSignature> m_rootSignature; // shared by every pipeline

        ComPtr<ID3D12Fence> m_flushFence;
        UINT64 m_flushValue = 0;
    };
}
