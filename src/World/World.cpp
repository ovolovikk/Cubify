#include "World/World.hpp"

#include "PrecompilerHeader.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include "Core/BlockType.hpp"
#include "World/ChunkMesher.hpp"
#include "Math/Frustum.hpp"

World::World(WorldType worldType)
{
    chunk_manager = std::make_unique<ChunkManager>(worldType);
}

World::~World() = default;

BlockType World::getBlock(int x, int y, int z) const
{
    return chunk_manager->getBlock(x, y, z);
}

void World::setBlock(int x, int y, int z, BlockType type)
{
    chunk_manager->setBlock(x, y, z, type);
}

RayCastResult World::rayCast(glm::vec3 origin, glm::vec3 direction, float max_distance) const
{
    glm::vec3 pos = origin;
    glm::vec3 dir = glm::normalize(direction);
    
    glm::ivec3 last_pos = glm::ivec3(std::floor(pos.x), std::floor(pos.y), std::floor(pos.z));

    for (float d = 0; d < max_distance; d+= RAYCAST_STEP)
    {
        pos += dir * RAYCAST_STEP;

        int x = static_cast<int>(std::floor(pos.x));
        int y = static_cast<int>(std::floor(pos.y));
        int z = static_cast<int>(std::floor(pos.z));

        if (x == last_pos.x && y == last_pos.y && z == last_pos.z)
            continue;

        int chunkX = static_cast<int>(std::floor(x / (float)CHUNK_SIZE));
        int chunkZ = static_cast<int>(std::floor(z / (float)CHUNK_SIZE));

        const Chunk* chunk = chunk_manager->getChunk(chunkX, chunkZ);
        if (chunk)
        {
            int localX = x - chunkX * CHUNK_SIZE;
            int localZ = z - chunkZ * CHUNK_SIZE;

            if (chunk->getBlock(localX, y, localZ) != BlockType::AIR)
            {
                return {true, glm::ivec3(x, y, z), last_pos};
            }
        }
        
        last_pos = glm::ivec3(x, y, z);
    }
    return {false, glm::ivec3(0), glm::ivec3(0)};
}

void World::rayCastBreakBlock(glm::vec3 origin, glm::vec3 direction, float max_distance)
{
    RayCastResult result = rayCast(origin, direction, max_distance);
    if (result.success) {
        if (getBlock(result.block_position.x, result.block_position.y, result.block_position.z) != BlockType::BEDROCK)
            setBlock(result.block_position.x, result.block_position.y, result.block_position.z, BlockType::AIR);
    }
}

void World::rayCastPlaceBlock(glm::vec3 origin, glm::vec3 direction, float max_distance, BlockType type)
{
    RayCastResult result = rayCast(origin, direction, max_distance);
    if (result.success) {
        // Check for block not being inside of a player
        int playerBlockX = static_cast<int>(std::floor(origin.x));
        int playerBlockY = static_cast<int>(std::floor(origin.y));
        int playerBlockZ = static_cast<int>(std::floor(origin.z));
        
        int placeX = result.previous_position.x;
        int placeY = result.previous_position.y;
        int placeZ = result.previous_position.z;
        
        if (placeX == playerBlockX && placeZ == playerBlockZ) {
            if (placeY == playerBlockY || placeY == playerBlockY - 1) {
                return;
            }
        }
        
        setBlock(placeX, placeY, placeZ, type);
    }
}

glm::vec3 World::getSpawnPoint()
{
    int targetX = 8;
    int targetZ = 8;

    int chunkX = targetX / CHUNK_SIZE;
    int chunkZ = targetZ / CHUNK_SIZE;
    
    // Force load the spawn chunk immediately
    chunk_manager->ensureChunkLoaded(chunkX, chunkZ);
    
    Chunk* chunk = chunk_manager->getChunk(chunkX, chunkZ);
    float spawnY = (float)CHUNK_HEIGHT + 2.0f;

    if (chunk) {
        int localX = targetX % CHUNK_SIZE;
        int localZ = targetZ % CHUNK_SIZE;
        
        for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
            BlockType block = chunk->getBlock(localX, y, localZ);
            if (block != BlockType::AIR && 
                block != BlockType::WATER && 
                block != BlockType::SECTORR_WATER &&
                block != BlockType::UTOPIA_WATER) {
                spawnY = (float)y + 2.0f;
                break;
            }
        }
    }
    
    return glm::vec3((float)targetX, spawnY, (float)targetZ);
}

void World::draw(IRendererBackend& renderer, const Frustum& frustum)
{
    // First pass: draw all opaque geometry
    for (auto& [id, chunk] : chunk_manager->getChunks())
    {
        if (!frustum.intersectsChunk(*chunk))
        {
            continue;
        }

        if (chunk->isDirty())
        {
            int x = chunk->getChunkX();
            int z = chunk->getChunkZ();

            ChunkNeighbors neighbors;
            neighbors.left = chunk_manager->getChunk(x - 1, z);
            neighbors.right = chunk_manager->getChunk(x + 1, z);
            neighbors.back = chunk_manager->getChunk(x, z - 1);
            neighbors.front = chunk_manager->getChunk(x, z + 1);

            ChunkMesher::generateMesh(*chunk, neighbors);
            renderer.uploadMesh(chunk->getMesh(), chunk->getQuads());
            renderer.uploadMesh(chunk->getTransparentMesh(), chunk->getTransparentQuads());

            chunk->setDirty(false);
        }

        if (chunk->getMesh().quadCount > 0)
        {
            glm::vec3 worldPos(
                chunk->getChunkX() * CHUNK_SIZE,
                0.f,
                chunk->getChunkZ() * CHUNK_SIZE
            );

            glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
            renderer.draw(chunk->getMesh(), model);
        }
    }
    
    // Second pass: draw transparent geometry (water) with depth write disabled
    renderer.beginTransparentPass();
    for (auto& [id, chunk] : chunk_manager->getChunks())
    {
        if (!frustum.intersectsChunk(*chunk))
        {
            continue;
        }

        if (chunk->getTransparentMesh().quadCount > 0)
        {
            glm::vec3 worldPos(
                chunk->getChunkX() * CHUNK_SIZE,
                0.f,
                chunk->getChunkZ() * CHUNK_SIZE
            );

            glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
            renderer.draw(chunk->getTransparentMesh(), model);
        }
    }
    renderer.endTransparentPass();
}

void World::update(glm::vec3 player_pos)
{
    chunk_manager->update(player_pos);
}

void World::prepareAllChunks(IRendererBackend& renderer)
{
    for (auto& [id, chunk] : chunk_manager->getChunks())
    {
        if (chunk->isDirty())
        {
            int x = chunk->getChunkX();
            int z = chunk->getChunkZ();

            ChunkNeighbors neighbors;
            neighbors.left = chunk_manager->getChunk(x - 1, z);
            neighbors.right = chunk_manager->getChunk(x + 1, z);
            neighbors.back = chunk_manager->getChunk(x, z - 1);
            neighbors.front = chunk_manager->getChunk(x, z + 1);

            ChunkMesher::generateMesh(*chunk, neighbors);
            renderer.uploadMesh(chunk->getMesh(), chunk->getQuads());
            renderer.uploadMesh(chunk->getTransparentMesh(), chunk->getTransparentQuads());

            chunk->setDirty(false);
        }
    }
}
