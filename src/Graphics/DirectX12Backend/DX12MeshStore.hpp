#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <d3d12.h>
#include <wrl/client.h>

#include "Graphics/DirectX12Backend/DX12DeletionQueue.hpp"
#include "Graphics/Mesh/MeshHandle.hpp"
#include "Graphics/Mesh/MeshId.hpp"
#include "Graphics/Mesh/Quad.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    // Chunk geometry buffers, keyed by the MeshId the world holds on to
    class DX12MeshStore
    {
    public:
        using DeferredReleases = std::vector<ComPtr<ID3D12Resource>>;

        struct DrawInfo
        {
            D3D12_GPU_VIRTUAL_ADDRESS address = 0;
            UINT quadCount = 0;
        };

        explicit DX12MeshStore(ID3D12Device2* device);

        // Records the copy into commandList, so the caller has to be inside a frame
        void upload(MeshHandle& mesh, const std::vector<Quad>& quads,
            ID3D12GraphicsCommandList* commandList, DeferredReleases& deferredReleases);
        void processDeletions(DeferredReleases& deferredReleases);

        DrawInfo drawInfo(MeshId mesh) const;

    private:
        ComPtr<ID3D12Resource> CreateGpuBuffer(const void* data, UINT64 size,
            D3D12_RESOURCE_STATES finalState, ID3D12GraphicsCommandList* commandList,
            ComPtr<ID3D12Resource>& outUploadBuffer);

        struct GpuMesh
        {
            ComPtr<ID3D12Resource> buffer;
            UINT quadCount = 0;
        };

        ComPtr<ID3D12Device2> m_device; // shared with DX12Device
        std::unordered_map<MeshId, GpuMesh> m_meshes;
        std::shared_ptr<DX12DeletionQueue> m_deletionQueue;
        uint32_t m_nextMeshId = 1;
    };
}
