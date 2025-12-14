#include "World/World.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <cmath>
#include <random>

#include "World/ChunkMesher.hpp"
#include "Helpers/Frustum.hpp"

World::World()
{
}

World::~World() = default;

long long World::getChunkId(int x, int z) const
{
    return (static_cast<long long>(x) << 32) | (static_cast<unsigned int>(z));
}

void World::addChunk(int x, int z)
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

void World::setBlock(int x, int y, int z, BlockType type)
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

void World::rayCastBreakBlock(glm::vec3 origin, glm::vec3 direction, float max_distance)
{
    float step = 0.05f;
    glm::vec3 pos = origin;
    glm::vec3 dir = glm::normalize(direction);

    for (float d = 0; d < max_distance; d+= step)
    {
        pos += dir * step;

        int x = static_cast<int>(std::floor(pos.x));
        int y = static_cast<int>(std::floor(pos.y));
        int z = static_cast<int>(std::floor(pos.z));

        int chunkX = static_cast<int>(std::floor(x / (float)CHUNK_SIZE));
        int chunkZ = static_cast<int>(std::floor(z / (float)CHUNK_SIZE));

        Chunk* chunk = getChunk(chunkX, chunkZ);
        if (chunk)
        {
            int localX = x - chunkX * CHUNK_SIZE;
            int localZ = z - chunkZ * CHUNK_SIZE;

            if (chunk->getBlock(localX, y, localZ) != BlockType::AIR)
            {
                setBlock(x, y, z, BlockType::AIR);
                chunk->setDirty(true);
                return;
            }
        }
    }
}

glm::vec3 World::getSpawnPoint()
{
    int targetX = 8;
    int targetZ = 8;

    int chunkX = targetX / CHUNK_SIZE;
    int chunkZ = targetZ / CHUNK_SIZE;
    addChunk(chunkX, chunkZ);
    
    Chunk* chunk = getChunk(chunkX, chunkZ);
    float spawnY = (float)CHUNK_HEIGHT + 2.0f;

    if (chunk) {
        int localX = targetX % CHUNK_SIZE;
        int localZ = targetZ % CHUNK_SIZE;
        
        for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
            if (chunk->getBlock(localX, y, localZ) != BlockType::AIR) {
                spawnY = (float)y + 2.0f;
                break;
            }
        }
    }
    
    return glm::vec3((float)targetX, spawnY, (float)targetZ);
}

void World::draw(Renderer& renderer, const Frustum& frustum)
{
    for(auto& [id, chunk]: chunks)
    {
        int x = static_cast<int>(id >> 32);
        int z = static_cast<int>(id & 0xFFFFFFFF);

        vec3 min(x * CHUNK_SIZE, 0, z * CHUNK_SIZE);
        vec3 max((x + 1) * CHUNK_SIZE, CHUNK_HEIGHT, (z + 1) * CHUNK_SIZE);

        // radar frustum approach
        if(!frustum.isBoxVisible(min, max)) continue;

        if (chunk->isDirty()) {
            ChunkNeighbors neighbors;
            neighbors.left = getChunk(x - 1, z);
            neighbors.right = getChunk(x + 1, z);
            neighbors.back = getChunk(x, z - 1);
            neighbors.front = getChunk(x, z + 1);

            ChunkMesher::generateMesh(*chunk, neighbors);

            renderer.uploadMesh(chunk->getMesh(), chunk->getQuads());
            chunk->setDirty(false);
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x * CHUNK_SIZE, 0, z * CHUNK_SIZE));
        renderer.draw(chunk->getMesh(), model);
    }
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
