#ifndef WORLD_SETTINGS_HPP
#define WORLD_SETTINGS_HPP

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

        case WorldType::EDMUNDS:
            return {
                0.001f,
                1.2f,
                glm::vec3(0.45f, 0.55f, 0.65f),
                glm::vec3(0.5f, 0.6f, 0.7f)
            };

        case WorldType::MANN:
            return {
                0.015f,
                2.0f,
                glm::vec3(0.7f, 0.75f, 0.8f),
                glm::vec3(0.85f, 0.88f, 0.92f)
            };

        case WorldType::MILLER:
            return {
                0.012f,
                1.3f,
                glm::vec3(0.3f, 0.35f, 0.4f),
                glm::vec3(0.4f, 0.45f, 0.5f)
            };

        default:
            return getForWorldType(WorldType::MINECRAFT);
        }
    }
};

#endif // WORLD_SETTINGS_HPP
