#include "WorldRenderer.hpp"

#include "PrecompilerHeader.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/IRendererBackend.hpp"
#include "Math/Frustum.hpp"
#include "World/ChunkMesher.hpp"
#include "World/World.hpp"

namespace
{
glm::mat4 getChunkModelMatrix(const Chunk& chunk)
{
    glm::vec3 worldPos(
        chunk.getChunkX() * CHUNK_SIZE,
        0.f,
        chunk.getChunkZ() * CHUNK_SIZE
    );
    return glm::translate(glm::mat4(1.0f), worldPos);
}
}

WorldRenderer::WorldRenderer(IRendererBackend& renderer)
    : m_renderer(renderer)
{
}

void WorldRenderer::uploadIfDirty(Chunk& chunk, ChunkManager& chunkManager)
{
    if (!chunk.isDirty())
    {
        return;
    }

    int x = chunk.getChunkX();
    int z = chunk.getChunkZ();

    ChunkNeighbors neighbors;
    neighbors.left = chunkManager.getChunk(x - 1, z);
    neighbors.right = chunkManager.getChunk(x + 1, z);
    neighbors.back = chunkManager.getChunk(x, z - 1);
    neighbors.front = chunkManager.getChunk(x, z + 1);

    ChunkMesher::generateMesh(chunk, neighbors);
    m_renderer.uploadMesh(chunk.getMeshHandle(), chunk.getQuads());
    m_renderer.uploadMesh(chunk.getTransparentMeshHandle(), chunk.getTransparentQuads());

    chunk.setDirty(false);
}

void WorldRenderer::draw(World& world, const Frustum& frustum)
{
    ChunkManager& chunkManager = world.GetChunkManager();

    // First pass: draw all opaque geometry
    for (auto& [id, chunk] : chunkManager.getChunks())
    {
        if (!frustum.intersectsChunk(*chunk))
        {
            continue;
        }

        uploadIfDirty(*chunk, chunkManager);
        m_renderer.draw(chunk->getMeshId(), getChunkModelMatrix(*chunk));
    }

    // Second pass: draw transparent geometry (water) with depth write disabled
    m_renderer.beginTransparentPass();
    for (auto& [id, chunk] : chunkManager.getChunks())
    {
        if (!frustum.intersectsChunk(*chunk))
        {
            continue;
        }

        m_renderer.draw(chunk->getTransparentMeshId(), getChunkModelMatrix(*chunk));
    }
    m_renderer.endTransparentPass();
}

void WorldRenderer::prepareAllChunks(World& world)
{
    ChunkManager& chunkManager = world.GetChunkManager();

    for (auto& [id, chunk] : chunkManager.getChunks())
    {
        uploadIfDirty(*chunk, chunkManager);
    }
}
