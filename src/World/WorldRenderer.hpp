#pragma once

class IRendererBackend;
class World;
class ChunkManager;
class Chunk;
struct Frustum;

class WorldRenderer
{
public:
    explicit WorldRenderer(IRendererBackend& renderer);

    void draw(World& world, const Frustum& frustum);
    void prepareAllChunks(World& world);

private:
    void uploadIfDirty(Chunk& chunk, ChunkManager& chunkManager);

    IRendererBackend& m_renderer;
};
