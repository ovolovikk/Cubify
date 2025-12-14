#include "ChunkManager.hpp"

#include "World/ChunkMesher.hpp"

#include <cmath>

ChunkManager::ChunkManager() = default;

ChunkManager::~ChunkManager() = default;

long long ChunkManager::getChunkId(int x, int z) const
{
    return (static_cast<long long>(x) << 32) | (static_cast<unsigned int>(z));
}

Chunk* ChunkManager::getChunk(int x, int z)
{
    return const_cast<Chunk*>(static_cast<const ChunkManager*>(this)->getChunk(x, z));
}

const Chunk* ChunkManager::getChunk(int x, int z) const
{
    long long id = getChunkId(x, z);
    auto it = chunks.find(id);
    if(it != chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ChunkManager::addChunk(int x, int z)
{
    long long id = getChunkId(x, z);
    if (chunks.find(id) == chunks.end())
    {
        auto chunk = std::make_unique<Chunk>(x, z);
        Chunk* chunkPtr = chunk.get();
        terrain_generator.GenerateChunkTerrain(chunkPtr);
        
        chunks[id] = std::move(chunk);

        ChunkNeighbors neighbors;
        neighbors.left = getChunk(x - 1, z);
        neighbors.right = getChunk(x + 1, z);
        neighbors.back = getChunk(x, z - 1);
        neighbors.front = getChunk(x, z + 1);

        ChunkMesher::generateMesh(*chunkPtr, neighbors);
    }
}

void ChunkManager::removeChunk(int x, int z)
{
    long long id = getChunkId(x, z);
    chunks.erase(id);
}

void ChunkManager::setBlock(int x, int y, int z, BlockType type)
{
    int chunk_x = static_cast<int>(std::floor(x / (float)CHUNK_SIZE));
    int chunk_z = static_cast<int>(std::floor(z / (float)CHUNK_SIZE));

    Chunk* chunk = getChunk(chunk_x, chunk_z);
    if (chunk)
    {
        int lx = x - chunk_x * CHUNK_SIZE;
        int lz = z - chunk_z * CHUNK_SIZE;
        chunk->setBlock(lx, y , lz, type);

        // update neighbors if block at the edge
        if (lx == 0) {
            Chunk* left = getChunk(chunk_x - 1, chunk_z);
            if (left) left->setDirty(true);
        }
        if (lx == CHUNK_SIZE - 1) {
            Chunk* right = getChunk(chunk_x + 1, chunk_z);
            if (right) right->setDirty(true);
        }
        if (lz == 0) {
            Chunk* back = getChunk(chunk_x, chunk_z - 1);
            if (back) back->setDirty(true);
        }
        if (lz == CHUNK_SIZE - 1) {
            Chunk* front = getChunk(chunk_x, chunk_z + 1);
            if (front) front->setDirty(true);
        }
    }
}

void ChunkManager::update(glm::vec3 player_pos)
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
