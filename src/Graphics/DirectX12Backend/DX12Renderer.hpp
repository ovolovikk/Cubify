#pragma once

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Graphics/IRendererBackend.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    // DirectX 12 implementation of IRendererBackend. Raw DX12 lives here, behind
    // the single interface — no extra abstraction layer. Everything is stubbed
    // for now; fill the pipeline in one method at a time.
    class DX12Renderer : public IRendererBackend
    {
    public:
        DX12Renderer(void* windowHandle, int width, int height);
        ~DX12Renderer() override;

        DX12Renderer(const DX12Renderer&) = delete;
        DX12Renderer& operator=(const DX12Renderer&) = delete;

        // ---- IRendererBackend ----
        void resize(int width, int height) override;
        void onResize(int width, int height) override;

        void beginFrame() override;
        void endFrame() override;
        void beginTransparentPass() override;
        void endTransparentPass() override;

        void setViewProjection(const glm::mat4& view, const glm::mat4& projection) override;
        void setWorldSettings(const WorldSettings& settings) override;

        void uploadMesh(MeshHandle& mesh, const std::vector<Quad>& quads) override;
        void draw(MeshId mesh, const glm::mat4& model) override;

    private:
        // ---- one-time init: device -> queue -> swapchain -> targets -> sync ----
        void CreateDebugController();
        void CreateFactory();
        void SelectAdapter();
        void CreateDevice();
        void CreateCommandQueue();
        void CreateSwapChain(void* windowHandle, int width, int height);
        void CreateRtvHeap();
        void CreateRenderTargets();
        void CreateCommandObjects();
        void CreateFence();
        void CreateRootSignature();
        void CreatePipelineState();

        // ---- per-frame CPU<->GPU synchronization ----
        void WaitForGpu();
        void MoveToNextFrame();

        static constexpr UINT FRAME_COUNT = 2;

        ComPtr<IDXGIFactory7> m_factory;
		ComPtr<IDXGIAdapter4> m_adapter;
        ComPtr<ID3D12Debug> m_debugController;
        ComPtr<ID3D12Device2> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
        ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_COUNT];
        ComPtr<ID3D12GraphicsCommandList> m_commandList;

        ComPtr<ID3D12RootSignature> m_rootSignature;
        ComPtr<ID3D12PipelineState> m_pipelineState;

        ComPtr<ID3D12Fence>  m_fence;
        HANDLE m_fenceEvent = nullptr;
        UINT64 m_fenceValues[FRAME_COUNT] = {};

        UINT m_rtvDescriptorSize = 0;
        UINT m_frameIndex = 0;
        UINT m_swapChainFlags = 0;
        int  m_width = 0;
        int  m_height = 0;
    };
}
