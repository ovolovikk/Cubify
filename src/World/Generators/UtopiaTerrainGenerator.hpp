#ifndef UTOPIA_TERRAIN_GENERATOR_HPP
#define UTOPIA_TERRAIN_GENERATOR_HPP

#include "World/Generators/TerrainGenerator.hpp"
#include "FastNoiseLite.h"

class Chunk;

class UtopiaTerrainGenerator : public TerrainGenerator
{
public:
    explicit UtopiaTerrainGenerator(int seed = 65386);
    ~UtopiaTerrainGenerator() override = default;

    void generateChunkTerrain(Chunk* chunk) override;

private:
    fnl_state m_seabedNoise;
    fnl_state m_rippleNoise;
    fnl_state m_siltNoise;
    
    static constexpr int BASE_SEABED = 3;
    static constexpr int SEABED_AMPLITUDE = 4;
    static constexpr int WATER_DEPTH = 6;
    static constexpr float SEABED_FREQUENCY = 0.008f;
    static constexpr float RIPPLE_FREQUENCY = 0.05f;
};

#endif // UTOPIA_TERRAIN_GENERATOR_HPP