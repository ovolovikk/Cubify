#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Graphics/IGpuResourceQueue.hpp"
#include "Graphics/MeshId.hpp"
#include "Graphics/Quad.hpp"

class GpuResourceManager : public IGpuResourceQueue
{
public:
    struct GpuMesh
    {
        uint32_t handle = 0;
        size_t quadCount = 0;
    };

    GpuResourceManager() = default;
    ~GpuResourceManager() override;

    GpuResourceManager(const GpuResourceManager&) = delete;
    GpuResourceManager& operator=(const GpuResourceManager&) = delete;

    MeshId create();
    void upload(MeshId id, const std::vector<Quad>& quads);
    const GpuMesh* find(MeshId id) const;
    void processPendingDeletions();
    void enqueueDeletion(MeshId id) override;

private:
    std::unordered_map<MeshId, GpuMesh> m_meshes;
    std::vector<MeshId> m_pendingDeletions;
};
