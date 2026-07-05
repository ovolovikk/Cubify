#pragma once

#include "World/Generators/ITerrainGenerator.hpp"
#include "World/WorldType.hpp"

class TerrainGeneratorFactory
{
public:
    static std::unique_ptr<ITerrainGenerator> create(WorldType type, int seed = 1337);
    
    TerrainGeneratorFactory() = delete;
};
