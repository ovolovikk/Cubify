#include "World.hpp"
#include <glm/gtc/matrix_transform.hpp>

World::World()
{
}

World::~World()
{
}

long long World::getChunkId(int x, int z) const
{
    return (static_cast<long long>(x) << 32) | (static_cast<unsigned int>(z));
}

void World::addChunk(int x, int z)
{
    long long id = getChunkId(x, z);
    if (chunks.find(id) == chunks.end())
    {
        chunks[id] = std::make_unique<Chunk>();
        chunks[id]->constructMesh();
    }
}

Chunk* World::getChunk(int x, int z)
{
    long long id = getChunkId(x, z);
    auto it = chunks.find(id);
    if (it != chunks.end())
    {
        return it->second.get();
    }
    return nullptr;
}
