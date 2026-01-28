#ifndef EDMUNDS_TERRAIN_GENERATOR_HPP
#define EDMUNDS_TERRAIN_GENERATOR_HPP

#include "World/Generators/TerrainGenerator.hpp"
#include "FastNoiseLite.h"

class Chunk;

class EdmundsTerrainGenerator : public TerrainGenerator
{
public:
    explicit EdmundsTerrainGenerator(int seed = 2048);
    ~EdmundsTerrainGenerator() override = default;

    void generateChunkTerrain(Chunk* chunk) override;

private:
    fnl_state m_noise;
    fnl_state m_rockNoise;
    
    static constexpr int MIN_HEIGHT = 30;
    static constexpr int AMPLITUDE = 50;
    static constexpr int EDMUNDS_WATER_LEVEL = 32;
    static constexpr float FREQUENCY = 0.003f;
    static constexpr int OCTAVES = 3;
    static constexpr float LACUNARITY = 2.0f;
    static constexpr float GAIN = 0.4f;
};

#endif // EDMUNDS_TERRAIN_GENERATOR_HPP
