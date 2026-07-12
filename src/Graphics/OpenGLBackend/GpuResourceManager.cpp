#include "Graphics/OpenGLBackend/GpuResourceManager.hpp"

#include <GL/glew.h>

MeshId GpuResourceManager::create()
{
    static uint32_t nextId = 1;
    MeshId id{nextId++};
    m_meshes.emplace(id, GpuMesh{});
    return id;
}

void GpuResourceManager::upload(MeshId id, const std::vector<Quad>& quads)
{
    auto it = m_meshes.find(id);
    if (it == m_meshes.end())
    {
        return;
    }

    GpuMesh& mesh = it->second;

    if (quads.empty())
    {
        mesh.quadCount = 0;
        return;
    }

    if (mesh.handle == 0)
    {
        glGenBuffers(1, &mesh.handle);
    }

    mesh.quadCount = quads.size();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh.handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, quads.size() * sizeof(Quad), quads.data(), GL_STATIC_DRAW);
}

const GpuResourceManager::GpuMesh* GpuResourceManager::find(MeshId id) const
{
    auto it = m_meshes.find(id);
    return it == m_meshes.end() ? nullptr : &it->second;
}

void GpuResourceManager::enqueueDeletion(MeshId id)
{
    if (id == InvalidMeshId)
    {
        return;
    }

    m_pendingDeletions.push_back(id);
}

void GpuResourceManager::processPendingDeletions()
{
    for (MeshId id : m_pendingDeletions)
    {
        auto it = m_meshes.find(id);
        if (it == m_meshes.end())
        {
            continue;
        }
        if (it->second.handle != 0)
        {
            glDeleteBuffers(1, &it->second.handle);
        }
        m_meshes.erase(it);
    }
    m_pendingDeletions.clear();
}

GpuResourceManager::~GpuResourceManager()
{
    for (auto& [id, mesh] : m_meshes)
    {
        if (mesh.handle != 0)
        {
            glDeleteBuffers(1, &mesh.handle);
        }
    }
}
