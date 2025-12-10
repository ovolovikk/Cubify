#include "TerrainGenerator.hpp"

#include "Chunk.hpp"
#include "Utils/NoiseGenerator.hpp"

#define FNL_IMPL
#include "Helpers/FastNoiseLite.h"

void TerrainGenerator::GenerateChunkTerrain(Chunk* chunk)
{
    if (!chunk) return;
    for (int lx = 0; lx < CHUNK_SIZE; ++lx)
    {
        for (int lz = 0; lz < CHUNK_SIZE; ++lz)
        {
            NoiseGenerator noise_g;
            int height = noise_g.getBLockHeight(chunk, lx, lz);

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
