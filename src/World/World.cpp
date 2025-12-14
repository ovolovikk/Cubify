#include "World/World.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <cmath>
#include <random>

#include "World/ChunkMesher.hpp"
#include "Helpers/Frustum.hpp"

World::World()
{
    chunk_manager = std::make_unique<ChunkManager>();
}

World::~World() = default;

void World::setBlock(int x, int y, int z, BlockType type)
{
    chunk_manager->setBlock(x, y, z, type);
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

        Chunk* chunk = chunk_manager->getChunk(chunkX, chunkZ);
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

    chunk_manager->update(glm::vec3(targetX, 100, targetZ));

    int chunkX = targetX / CHUNK_SIZE;
    int chunkZ = targetZ / CHUNK_SIZE;
    
    Chunk* chunk = chunk_manager->getChunk(chunkX, chunkZ);
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
    for(auto& [id, chunk]: chunk_manager->getChunks())
    {
        int x = static_cast<int>(id >> 32);
        int z = static_cast<int>(id & 0xFFFFFFFF);

        vec3 min(x * CHUNK_SIZE, 0, z * CHUNK_SIZE);
        vec3 max((x + 1) * CHUNK_SIZE, CHUNK_HEIGHT, (z + 1) * CHUNK_SIZE);

        // radar frustum approach
        if(!frustum.isBoxVisible(min, max)) continue;

        if (chunk->isDirty()) {
            ChunkNeighbors neighbors;
            neighbors.left = chunk_manager->getChunk(x - 1, z);
            neighbors.right = chunk_manager->getChunk(x + 1, z);
            neighbors.back =chunk_manager->getChunk(x, z - 1);
            neighbors.front = chunk_manager->getChunk(x, z + 1);

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
    chunk_manager->update(player_pos);
}
