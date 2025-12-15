#ifndef TERRAIN_GENERATOR_HPP
#define TERRAIN_GENERATOR_HPP

#include "Utils/NoiseGenerator.hpp"

class Chunk;

class TerrainGenerator
{
public:
	TerrainGenerator();
	
	TerrainGenerator(const TerrainGenerator&) = delete;
	TerrainGenerator& operator=(const TerrainGenerator&) = delete;

	void GenerateChunkTerrain(Chunk* chunk);

private:
	NoiseGenerator noise_gen;
    static constexpr auto WATER_LEVEL = 45;
};

#endif // TERRAIN_GENERATOR_HPP
