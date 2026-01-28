#include "World/Generators/TerrainGeneratorFactory.hpp"

#include "World/Generators/MinecraftTerrainGenerator.hpp"
#include "World/Generators/EdmundsTerrainGenerator.hpp"

std::unique_ptr<ITerrainGenerator> TerrainGeneratorFactory::create(WorldType type, int seed)
{
    switch (type)
    {
    case WorldType::EDMUNDS:
        return std::make_unique<EdmundsTerrainGenerator>(seed);
    
    case WorldType::MANN:
        // TODO: Implement MannTerrainGenerator
        return std::make_unique<MinecraftTerrainGenerator>(seed);
    
    case WorldType::MILLER:
        // TODO: Implement MillerTerrainGenerator
        return std::make_unique<MinecraftTerrainGenerator>(seed);
    
    case WorldType::MINECRAFT:
    default:
        return std::make_unique<MinecraftTerrainGenerator>(seed);
    }
}
