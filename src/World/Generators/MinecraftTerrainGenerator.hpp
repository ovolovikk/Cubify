#pragma once

#include "World/Generators/TerrainGenerator.hpp"

class Chunk;

class MinecraftTerrainGenerator : public TerrainGenerator
{
public:
    explicit MinecraftTerrainGenerator(int seed = 1337);
    ~MinecraftTerrainGenerator() override = default;

    void generateChunkTerrain(Chunk* chunk) override;

private:
    static constexpr int MINECRAFT_WATER_LEVEL = 40;
};
