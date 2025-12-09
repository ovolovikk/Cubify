#include "Player/PlayerCollision.hpp"

#include <cmath>
#include <algorithm>

#include "World/World.hpp"
#include "World/Chunk.hpp"
#include "Helpers/BlockType.hpp"

// works for global coordinates
static BlockType getBlock(World& world, int x, int y, int z)
{
    int chunk_x = static_cast<int>(std::floor(x / (float)CHUNK_SIZE));
    int chunk_z = static_cast<int>(std::floor(z / (float)CHUNK_SIZE));

    Chunk* chunk = world.getChunk(chunk_x, chunk_z);
    if(!chunk) return BlockType::AIR;

    int local_x = x - chunk_x * CHUNK_SIZE;
    int local_z = z - chunk_z * CHUNK_SIZE;

    if (y < 0 || y >= CHUNK_HEIGHT) return BlockType::AIR;

    return chunk->getBlock(local_x, y, local_z);
}

PlayerCollision::PlayerCollision(World& world) : world(world)
{
}

bool PlayerCollision::checkCollision(const AABB& box, const vec3& pos)
{
    vec3 min = pos + box.min;
    vec3 max = pos + box.max;

    int min_x = static_cast<int>(std::floor(min.x));
    int max_x = static_cast<int>(std::floor(max.x));
    int min_y = static_cast<int>(std::floor(min.y));
    int max_y = static_cast<int>(std::floor(max.y));
    int min_z = static_cast<int>(std::floor(min.z));
    int max_z = static_cast<int>(std::floor(max.z));

    for(int x = min_x; x <= max_x;++x)
        for(int y = min_y; y <= max_y;++y)
            for(int z = min_z; z <= max_z;++z)
                if (getBlock(world, x, y, z) != BlockType::AIR) return true;

    return false;
}

vec3 PlayerCollision::resolve(const vec3& pos, const vec3& vel, const AABB& box)
{
    vec3 current_pos = pos;
    grounded = false;
    
    // Y axis
    current_pos.y += vel.y;
    if (checkCollision(box, current_pos))
    {
        // falling
        if(vel.y < 0)
        {
            grounded = true;
            current_pos.y = std::floor(current_pos.y + box.min.y) + 1.f - box.min.y;
        }

        if (checkCollision(box, current_pos))
        {
            current_pos.y = pos.y;
        }
    } else
        // jumping
        {
            current_pos.y = std::floor(current_pos.y + box.max.y) - box.max.y - 0.001f; 
            if (checkCollision(box, current_pos))
            {
                current_pos.y = pos.y;
            }
        }
    
    // X axis
    current_pos.x += vel.x;
    if (checkCollision(box, current_pos))
    {
        current_pos.x = pos.x;
    }

    // Z axis
    current_pos.z += vel.z;
    if (checkCollision(box, current_pos))
    {
        current_pos.z = pos.z;
    }
    
    return current_pos;
}