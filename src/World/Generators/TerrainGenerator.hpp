#ifndef TERRAIN_GENERATOR_HPP
#define TERRAIN_GENERATOR_HPP

#include "World/Generators/ITerrainGenerator.hpp"
#include "Utils/NoiseGenerator.hpp"

class Chunk;

class TerrainGenerator : public ITerrainGenerator
{
public:
    explicit TerrainGenerator(int seed = 1337);
    virtual ~TerrainGenerator() = default;

    void generateChunkTerrain(Chunk* chunk) override = 0;

protected:
    NoiseGenerator noise_gen;
    
    static constexpr int WATER_LEVEL = 40;
};

#endif // TERRAIN_GENERATOR_HPP
