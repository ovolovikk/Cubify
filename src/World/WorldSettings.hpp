#pragma once

#include <glm/vec3.hpp>
#include "World/WorldType.hpp"

struct WorldSettings
{
    float fogDensity;
    float fogPower;
    glm::vec3 skyColor;
    glm::vec3 fogColor;

    static WorldSettings getForWorldType(WorldType type)
    {
        switch (type)
        {
        case WorldType::MINECRAFT:
            return {
                0.003f,
                1.2f,
                glm::vec3(0.28f, 0.66f, 1.0f),
                glm::vec3(0.6f, 0.8f, 1.0f)
            };

        case WorldType::SECTORR:
            return {
                0.001f,
                1.2f,
                glm::vec3(0.55f, 0.35f, 0.45f),
                glm::vec3(0.6f, 0.4f, 0.5f)
            };

        case WorldType::UTOPIA:
            return {
                0.006f,
                1.8f, 
                glm::vec3(0.25f, 0.28f, 0.32f),
                glm::vec3(0.35f, 0.40f, 0.45f)
            };

        default:
            return getForWorldType(WorldType::MINECRAFT);
        }
    }
};
