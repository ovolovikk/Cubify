#include "Player/PlayerCollision.hpp"

#include "PrecompilerHeader.hpp"

#include "World/World.hpp"
#include "Core/BlockType.hpp"

PlayerCollision::PlayerCollision(World& world) : world(world), grounded(false)
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
            for(int z = min_z; z <= max_z;++z) {
                BlockType block = world.getBlock(x, y, z);
                if (block != BlockType::AIR && 
                    block != BlockType::WATER && 
                    block != BlockType::SECTORR_WATER &&
                    block != BlockType::UTOPIA_WATER) return true;
            }

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
        else if (vel.y > 0)
        {
            // jumping (hit ceiling)
            current_pos.y = std::floor(current_pos.y + box.max.y) - box.max.y - 0.001f; 
        }

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
