#include "World/TerrainGenerator.hpp"

#include "World/Chunk.hpp"

TerrainGenerator::TerrainGenerator() : noise_gen(1337)
{
}

void TerrainGenerator::GenerateChunkTerrain(Chunk* chunk)
{
    if (!chunk) return;
    for (int lx = 0; lx < CHUNK_SIZE; ++lx)
    {
        for (int lz = 0; lz < CHUNK_SIZE; ++lz)
        {
            int height = noise_gen.getBLockHeight(chunk, lx, lz);

            for (int ly = 0; ly < CHUNK_HEIGHT; ++ly)
            {
                BlockType type = BlockType::AIR;

                if (ly <= height)
                {
                    if (ly < height - 3)
                    {
                        type = BlockType::STONE;
                    }
                    else
                    {
                        if (height <= WATER_LEVEL + 2)
                        {
                            type = BlockType::SAND;
                        }
                        else
                        {
                            if (ly == height) type = BlockType::GRASS;
                            else type = BlockType::DIRT;
                        }
                    }
                }
                else if (ly <= WATER_LEVEL)
                {
                    type = BlockType::WATER;
                }

                if (type != BlockType::AIR)
                {
                    chunk->setBlock(lx, ly, lz, type);
                }
                else
                {
                    // Ensure air is set if we are overwriting or initializing
                    chunk->setBlock(lx, ly, lz, BlockType::AIR);
                }
            }
        }
    }
}
