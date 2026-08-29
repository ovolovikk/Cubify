#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Graphics/DirectX12Backend/DX12DeletionQueue.hpp"
#include "Graphics/IRendererBackend.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

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

        bool captureBackbuffer(const char* filePath) override;

    private:
        void CreateDebugController();
        void CreateFactory();
        void SelectAdapter();
        void CreateDevice();
        void CreateCommandQueue();
        void CreateSwapChain(void* windowHandle, int width, int height);
        void CreateRtvHeap();
        void CreateRenderTargets();
        void CreateDsvHeap();
        void CreateDepthStencil();
        void CreateCommandObjects();
        void CreateFence();
        void CreateRootSignature();
        void CreatePipelineState();
        void CreateSrvHeap();
        void CreateTextureArray();

        ComPtr<ID3D12Resource> CreateGpuBuffer(const void* data, UINT64 size,
            D3D12_RESOURCE_STATES finalState, ComPtr<ID3D12Resource>& outUploadBuffer);
        void ProcessMeshDeletions();

        void SetDebugName(ID3D12Object* object, const wchar_t* name);

        // per-frame CPU<->GPU synchronization
        void WaitForGpu();
        void MoveToNextFrame();

        static constexpr UINT FRAME_COUNT = 2;
        static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;
        static constexpr UINT ROOT_PARAM_VIEW_PROJ = 0;
        static constexpr UINT ROOT_PARAM_MODEL = 1;
        static constexpr UINT ROOT_PARAM_QUADS = 2;
        static constexpr UINT ROOT_PARAM_TEXTURES = 3;
        static constexpr UINT MATRIX_CONSTANT_COUNT = 16;
        static constexpr UINT VERTICES_PER_QUAD = 6;

        struct GpuMesh
        {
            ComPtr<ID3D12Resource> buffer;
            UINT quadCount = 0;
        };

        glm::mat4 m_viewProj{ 1.0f };

        ComPtr<IDXGIFactory7> m_factory;
		ComPtr<IDXGIAdapter4> m_adapter;
        ComPtr<ID3D12Debug> m_debugController;
        ComPtr<ID3D12Device2> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
        ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        ComPtr<ID3D12Resource> m_depthStencil;
        ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        ComPtr<ID3D12Resource> m_textureArray;
        ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_COUNT];
        ComPtr<ID3D12GraphicsCommandList> m_commandList;

        std::unordered_map<MeshId, GpuMesh> m_meshes;
        std::shared_ptr<DX12DeletionQueue> m_deletionQueue;
        uint32_t m_nextMeshId = 1;

        // Resources the GPU may still be reading. Each slot is emptied only when
        // that frame comes around again, by which point its fence has passed.
        std::vector<ComPtr<ID3D12Resource>> m_deferredReleases[FRAME_COUNT];

        ComPtr<ID3D12RootSignature> m_rootSignature;
        
        ComPtr<ID3D12PipelineState> m_pipelineState;
        ComPtr<ID3D12PipelineState> m_transparentPipelineState; // has no depth test

        ComPtr<ID3D12Fence>  m_fence;
        HANDLE m_fenceEvent = nullptr;
        UINT64 m_fenceValues[FRAME_COUNT] = {};

        UINT m_rtvDescriptorSize = 0;
        UINT m_currentFrame = 0;
        UINT m_swapChainFlags = 0;
        bool m_commandListOpen = false;
        int  m_width = 0;
        int  m_height = 0;
    };
}
