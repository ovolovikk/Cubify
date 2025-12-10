#ifndef TERRAIN_GENERATOR_HPP
#define TERRAIN_GENERATOR_HPP

#include "Helpers/FastNoiseLite.h"

class Chunk;

class TerrainGenerator
{
public:
	TerrainGenerator() = default;
	
	TerrainGenerator(const TerrainGenerator&) = delete;
	TerrainGenerator& operator=(const TerrainGenerator&) = delete;

	void GenerateChunkTerrain(Chunk* chunk);

private:
	fnl_state noise;

};

#endif // TERRAIN_GENERATOR_HPP
