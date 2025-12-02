#include "World.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <cmath>

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
        chunks[id] = std::make_unique<Chunk>(x, z);
        chunks[id]->constructMesh();
    }
}

void World::removeChunk(int x, int z)
{
    long long id = getChunkId(x, z);
    chunks.erase(id);
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

void World::update(glm::vec3 player_pos)
{
    int playerChunkX = static_cast<int>(floor(player_pos.x / CHUNK_SIZE));
    int playerChunkZ = static_cast<int>(floor(player_pos.z / CHUNK_SIZE));

    // load chunks
    for(int x = playerChunkX - render_distance; x <= playerChunkX + render_distance; ++x)
    {
        for (int z = playerChunkZ - render_distance; z <= playerChunkZ + render_distance; ++z)
        {
            addChunk(x, z);
        }
    }

    // unload chunks
    for(auto it = chunks.begin();it != chunks.end();)
    {
        long long id = it->first;

        int x = static_cast<int>(id >> 32);
        int z = static_cast<int>(id & 0xFFFFFFFF);

        if(abs(x - playerChunkX) > render_distance || abs(z - playerChunkZ) > render_distance)
        {
            it = chunks.erase(it);
        } else ++it;

    }
}
