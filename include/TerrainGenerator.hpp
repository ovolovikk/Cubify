#ifndef TERRAIN_GENERATOR_HPP
#define TERRAIN_GENERATOR_HPP

#include "FastNoiseLite.h"

class Chunk;

class TerrainGenerator
{
public:
	TerrainGenerator(int seed_ = 12345678);
	
	void generateChunkTerrain(Chunk* chunk);
private:
	int seed;
	fnl_state noise;
};

#endif // TERRAIN_GENERATOR_HPP
