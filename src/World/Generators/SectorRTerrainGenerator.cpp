#include "World/Generators/SectorRTerrainGenerator.hpp"

#include "Core/BlockType.hpp"
#include "World/Chunk.hpp"

SectorRTerrainGenerator::SectorRTerrainGenerator(int seed) 
    : TerrainGenerator(seed)
{
    m_noise = fnlCreateState();
    m_noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    m_noise.fractal_type = FNL_FRACTAL_FBM;
    m_noise.octaves = OCTAVES;
    m_noise.lacunarity = LACUNARITY;
    m_noise.gain = GAIN;
    m_noise.seed = seed;
    m_noise.frequency = FREQUENCY;
    
    m_rockNoise = fnlCreateState();
    m_rockNoise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    m_rockNoise.fractal_type = FNL_FRACTAL_FBM;
    m_rockNoise.octaves = 2;
    m_rockNoise.lacunarity = 2.0f;
    m_rockNoise.gain = 0.5f;
    m_rockNoise.seed = seed + 777;
    m_rockNoise.frequency = 0.02f;
}

void SectorRTerrainGenerator::generateChunkTerrain(Chunk* chunk)
{
    if (!chunk) return;

    for (int lx = 0; lx < CHUNK_SIZE; ++lx)
    {
        for (int lz = 0; lz < CHUNK_SIZE; ++lz)
        {
            float globalX = static_cast<float>(chunk->getChunkX() * CHUNK_SIZE + lx);
            float globalZ = static_cast<float>(chunk->getChunkZ() * CHUNK_SIZE + lz);

            float noiseValue = fnlGetNoise2D(&m_noise, globalX, globalZ);
            float n = (noiseValue + 1.0f) * 0.5f; // [0, 1]
            
            n = n * n;
            
            int height = MIN_HEIGHT + static_cast<int>(n * AMPLITUDE);
            
            if (height < 1) height = 1;
            if (height >= CHUNK_HEIGHT) height = CHUNK_HEIGHT - 1;

            for (int ly = 0; ly < CHUNK_HEIGHT; ++ly)
            {
                BlockType type = BlockType::AIR;

                if (ly <= height)
                {
                    if (ly == 0)
                    {
                        type = BlockType::BEDROCK;
                    }
                    else if (ly < height - 4)
                    {
                        type = BlockType::SECTORR_STONE;
                    }
                    else if (ly < height)
                    {
                        if (height <= SECTORR_WATER_LEVEL + 2)
                            type = BlockType::SECTORR_SAND;
                        else
                            type = BlockType::SECTORR_DIRT;
                    }
                    else
                    {
                        if (height <= SECTORR_WATER_LEVEL + 2)
                        {
                            type = BlockType::SECTORR_SAND;
                        }
                        else
                        {
                            float rockValue = fnlGetNoise2D(&m_rockNoise, globalX, globalZ);
                            bool isRocky = rockValue > 0.15f;
                            
                            if (isRocky)
                                type = BlockType::SECTORR_STONE;
                            else
                                type = BlockType::SECTORR_GRASS;
                        }
                    }
                }
                else if (ly <= SECTORR_WATER_LEVEL)
                {
                    type = BlockType::SECTORR_WATER;
                }

                chunk->setBlock(lx, ly, lz, type);
            }
        }
    }
}
