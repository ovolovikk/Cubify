#include "World/Generators/TerrainGeneratorFactory.hpp"

#include "World/Generators/MinecraftTerrainGenerator.hpp"
#include "World/Generators/SectorRTerrainGenerator.hpp"
#include "World/Generators/UtopiaTerrainGenerator.hpp"

std::unique_ptr<ITerrainGenerator> TerrainGeneratorFactory::create(WorldType type, int seed)
{
    switch (type)
    {
    case WorldType::SECTORR:
        return std::make_unique<SectorRTerrainGenerator>(seed);
    
    case WorldType::UTOPIA:
        return std::make_unique<UtopiaTerrainGenerator>(seed);
    
    case WorldType::MINECRAFT:
    default:
        return std::make_unique<MinecraftTerrainGenerator>(seed);
    }
}
