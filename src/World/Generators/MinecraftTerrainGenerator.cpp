#include "World/Generators/MinecraftTerrainGenerator.hpp"

#include "Core/BlockType.hpp"
#include "World/Chunk.hpp"

MinecraftTerrainGenerator::MinecraftTerrainGenerator(int seed) 
    : TerrainGenerator(seed)
{
}

void MinecraftTerrainGenerator::generateChunkTerrain(Chunk* chunk)
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
                    if (ly == 0)
                    {
                        type = BlockType::BEDROCK;
                    }
                    else if (ly < height - 3)
                    {
                        type = BlockType::STONE;
                    }
                    else
                    {
                        if (height <= MINECRAFT_WATER_LEVEL + 2)
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
                else if (ly <= MINECRAFT_WATER_LEVEL)
                {
                    type = BlockType::WATER;
                }

                chunk->setBlock(lx, ly, lz, type);
            }
        }
    }
}
