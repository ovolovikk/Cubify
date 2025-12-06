#include "TerrainGenerator.hpp"

#include "Chunk.hpp"

#define FNL_IMPL
#include "FastNoiseLite.h"

TerrainGenerator::TerrainGenerator(int seed_)
    : seed(seed_)
{
    noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.seed = seed;
    noise.frequency = 0.02f;
}

void TerrainGenerator::generateChunkTerrain(Chunk* chunk)
{
    if (!chunk) return;
    for (int lx = 0; lx < CHUNK_SIZE; ++lx)
    {
        for (int lz = 0; lz < CHUNK_SIZE; ++lz)
        {
            float globalX = static_cast<float>(chunk->getChunkX() * CHUNK_SIZE + lx);
            float globalZ = static_cast<float>(chunk->getChunkZ() * CHUNK_SIZE + lz);

            // get normalized noise value
            float noiseValue = fnlGetNoise2D(&noise, globalX, globalZ);
            int height = 32 + static_cast<int>(noiseValue * 16.0f);

            if (height < 0) height = 0;
            if (height >= CHUNK_HEIGHT) height = CHUNK_HEIGHT - 1;

            for (int ly = 0; ly < CHUNK_HEIGHT; ++ly)
            {
                if (ly > height)
                {
                    chunk->setBlock(lx, ly, lz, BlockType::AIR);
                }
                else if (ly == height)
                {
                    chunk->setBlock(lx, ly, lz, BlockType::GRASS);
                }
                else
                {
                    chunk->setBlock(lx, ly, lz, BlockType::DIRT);
                }
            }
        }
    }
}
