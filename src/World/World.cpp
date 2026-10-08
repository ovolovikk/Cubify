#include "World/World.hpp"

#include "PrecompilerHeader.hpp"

#include <glm/vec3.hpp>

#include "Core/BlockType.hpp"

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

std::optional<int> World::findTopSolidY(int x, int z) const
{
    // TODO: think of way to impl this with ranges
    for (int y = CHUNK_HEIGHT - 1; y >= 0; --y)
        if (isSolid(getBlock(x, y, z)))
            return y;
    return std::nullopt;
}

glm::vec3 World::getSpawnPoint()
{
    static constexpr int SPAWN_X = 8;
    static constexpr int SPAWN_Z = 8;

    chunk_manager->ensureChunkLoaded(
        static_cast<int>(std::floor(SPAWN_X / (float)CHUNK_SIZE)),
        static_cast<int>(std::floor(SPAWN_Z / (float)CHUNK_SIZE)));

    // Water has no collision, so an ocean column spawns on the sea floor
    std::optional<int> top = findTopSolidY(SPAWN_X, SPAWN_Z);

    // Feet stand on top of that block, in the middle of the column: the
    // player box is 0.6 wide and would otherwise straddle four columns whose
    // heights were never checked
    float feetY = top ? *top + 1.0f : (float)CHUNK_HEIGHT;

    return glm::vec3(SPAWN_X + 0.5f, feetY, SPAWN_Z + 0.5f);
}

void World::update(glm::vec3 player_pos)
{
    chunk_manager->update(player_pos);
}
