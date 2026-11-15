#include "DX12MeshStore.hpp"

#include <directx/d3dx12.h>
#include "Graphics/DirectX12Backend/DX12Common.hpp"

namespace Cubify::DX12
{
    DX12MeshStore::DX12MeshStore(ID3D12Device2* device)
        : m_device(device), m_deletionQueue(std::make_shared<DX12DeletionQueue>())
    {
        m_meshes.emplace_back();
    }

    void DX12MeshStore::upload(MeshHandle& mesh, const std::vector<Quad>& quads,
        ID3D12GraphicsCommandList* commandList, DeferredReleases& deferredReleases)
    {
        if (quads.empty())
        {
            // Chunk became empty. Keep the id, just stop drawing it.
            m_meshes[static_cast<size_t>(mesh.id())].quadCount = 0;
            return;
        }

        if (!mesh.isValid())
        {
            if (m_freeMeshIds.empty())
            {
                const MeshId newId{ static_cast<uint32_t>(m_meshes.size()) };
                m_freeMeshIds.push_back(newId);
                m_meshes.emplace_back();
            }
            mesh = MeshHandle(m_freeMeshIds.back(), m_deletionQueue);
            m_freeMeshIds.pop_back();
        }

        ComPtr<ID3D12Resource> uploadBuffer;
        ComPtr<ID3D12Resource> buffer = CreateGpuBuffer(
            quads.data(),
            quads.size() * sizeof(Quad),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            commandList,
            uploadBuffer);
        if (!buffer)
        {
            return;
        }

        GpuMesh& gpuMesh = m_meshes[static_cast<size_t>(mesh.id())];

        // Remeshing a chunk replaces its buffer, but frames in flight may still
        // be reading the old one
        if (gpuMesh.buffer)
        {
            deferredReleases.push_back(std::move(gpuMesh.buffer));
        }

        gpuMesh.buffer = std::move(buffer);
        gpuMesh.quadCount = static_cast<UINT>(quads.size());

        // The copy is only recorded, not executed, so staging has to survive
        // until this frame is done
        deferredReleases.push_back(std::move(uploadBuffer));
    }

    void DX12MeshStore::processDeletions(DeferredReleases& deferredReleases)
    {
        for (MeshId id : m_deletionQueue->takeMeshes())
        {
            GpuMesh& gpuMesh = m_meshes[static_cast<size_t>(id)];
            if (gpuMesh.buffer)
            {
                deferredReleases.push_back(std::move(gpuMesh.buffer));
            }
            gpuMesh.quadCount = 0;
            m_freeMeshIds.push_back(id);
        }
    }

    DX12MeshStore::DrawInfo DX12MeshStore::drawInfo(MeshId mesh) const
    {
        const GpuMesh& gpuMesh = m_meshes[static_cast<size_t>(mesh)];
        if (gpuMesh.quadCount == 0 || !gpuMesh.buffer)
        {
            return {};
        }

        return DrawInfo{ gpuMesh.buffer->GetGPUVirtualAddress(), gpuMesh.quadCount };
    }

    ComPtr<ID3D12Resource> DX12MeshStore::CreateGpuBuffer(const void* data, UINT64 size,
        D3D12_RESOURCE_STATES finalState, ID3D12GraphicsCommandList* commandList,
        ComPtr<ID3D12Resource>& outUploadBuffer)
    {
        CD3DX12_HEAP_PROPERTIES heapDesc(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
        ComPtr<ID3D12Resource> buffer;

        HR_FALLBACK(m_device->CreateCommittedResource(
            &heapDesc,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&buffer)
        ), nullptr, "[DX12MeshStore] Failed to create GPU buffer");

        CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
        HR_FALLBACK(m_device->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&outUploadBuffer)
        ), nullptr, "[DX12MeshStore] Failed to create upload buffer");

        CD3DX12_RANGE readRange(0, 0);
        void* mappedData = nullptr;
        HR_FALLBACK(outUploadBuffer->Map(0, &readRange, &mappedData), nullptr,
            "[DX12MeshStore] Failed to map upload buffer");
        memcpy(mappedData, data, static_cast<size_t>(size));
        outUploadBuffer->Unmap(0, nullptr);

        // DX12 Does it automatically, but explicitly writing it to remember
        CD3DX12_RESOURCE_BARRIER toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        commandList->ResourceBarrier(1, &toCopyDest);

        commandList->CopyBufferRegion(buffer.Get(), 0, outUploadBuffer.Get(), 0, size);

        // This one is never automatic
        CD3DX12_RESOURCE_BARRIER toReadable = CD3DX12_RESOURCE_BARRIER::Transition(
            buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, finalState);
        commandList->ResourceBarrier(1, &toReadable);

        return buffer;
    }
}
