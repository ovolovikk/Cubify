#pragma once

#include <memory>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Graphics/DirectX12Backend/DX12Device.hpp"
#include "Graphics/DirectX12Backend/DX12MeshStore.hpp"
#include "Graphics/DirectX12Backend/DX12Pipeline.hpp"
#include "Graphics/DirectX12Backend/DX12SwapChain.hpp"
#include "Graphics/DirectX12Backend/DX12Texture.hpp"
#include "Graphics/IRendererBackend.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12Renderer : public IRendererBackend
    {
    public:
        DX12Renderer(DX12Device& device, void* windowHandle, int width, int height);
        ~DX12Renderer() override;

        DX12Renderer(const DX12Renderer&) = delete;
        DX12Renderer& operator=(const DX12Renderer&) = delete;

        // ---- IRendererBackend ----
        void resize(int width, int height) override;

        void beginFrame() override;
        void endFrame() override;
        void beginTransparentPass() override;
        void endTransparentPass() override;

        void setViewProjection(const glm::mat4& view, const glm::mat4& projection) override;
        void setWorldSettings(const WorldSettings& settings) override;

        void uploadMesh(MeshHandle& mesh, const std::vector<Quad>& quads) override;
        void draw(MeshId mesh, const glm::mat4& model) override;

        bool captureBackBufferInsideFrame(const char* filePath) override;

    private:
        void CreateCommandObjects();
        void CreateFence();
        // per-frame CPU<->GPU synchronization
        void MoveToNextFrame();

        static constexpr UINT FRAME_COUNT = DX12SwapChain::FRAME_COUNT;
        static constexpr UINT VERTICES_PER_QUAD = 6;

        DX12Device& m_device;

        std::unique_ptr<DX12SwapChain> m_swapChain;

        ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_COUNT];
        ComPtr<ID3D12GraphicsCommandList> m_commandList;

        std::unique_ptr<DX12MeshStore> m_meshStore;
        std::unique_ptr<DX12Texture> m_blockTextures;

        // Resources the GPU may still be reading. Each slot is emptied only when
        // that frame comes around again, by which point its fence has passed.
        std::vector<ComPtr<ID3D12Resource>> m_deferredReleases[FRAME_COUNT];

        std::unique_ptr<DX12Pipeline> m_solidPipeline;
        std::unique_ptr<DX12Pipeline> m_transparentPipeline;

        ComPtr<ID3D12Fence>  m_fence;
        HANDLE m_fenceEvent = nullptr;
        UINT64 m_fenceValues[FRAME_COUNT] = {};

        UINT m_currentFrame = 0;
    };
}
