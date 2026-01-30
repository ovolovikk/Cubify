#include "World/Generators/UtopiaTerrainGenerator.hpp"

#include "Core/BlockType.hpp"
#include "World/Chunk.hpp"

UtopiaTerrainGenerator::UtopiaTerrainGenerator(int seed) 
    : TerrainGenerator(seed)
{
    m_seabedNoise = fnlCreateState();
    m_seabedNoise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    m_seabedNoise.fractal_type = FNL_FRACTAL_FBM;
    m_seabedNoise.octaves = 2;
    m_seabedNoise.lacunarity = 2.0f;
    m_seabedNoise.gain = 0.5f;
    m_seabedNoise.seed = seed;
    m_seabedNoise.frequency = SEABED_FREQUENCY;
    
    m_rippleNoise = fnlCreateState();
    m_rippleNoise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    m_rippleNoise.fractal_type = FNL_FRACTAL_RIDGED;
    m_rippleNoise.octaves = 2;
    m_rippleNoise.lacunarity = 2.5f;
    m_rippleNoise.gain = 0.6f;
    m_rippleNoise.seed = seed + 123;
    m_rippleNoise.frequency = RIPPLE_FREQUENCY;

    m_siltNoise = fnlCreateState();
    m_siltNoise.noise_type = FNL_NOISE_CELLULAR;
    m_siltNoise.cellular_return_type = FNL_CELLULAR_RETURN_TYPE_DISTANCE2DIV;
    m_siltNoise.seed = seed + 456;
    m_siltNoise.frequency = 0.02f;
}

void UtopiaTerrainGenerator::generateChunkTerrain(Chunk* chunk)
{
    if (!chunk) return;

    for (int lx = 0; lx < CHUNK_SIZE; ++lx)
    {
        for (int lz = 0; lz < CHUNK_SIZE; ++lz)
        {
            float globalX = static_cast<float>(chunk->getChunkX() * CHUNK_SIZE + lx);
            float globalZ = static_cast<float>(chunk->getChunkZ() * CHUNK_SIZE + lz);
            
            float seabedNoise = fnlGetNoise2D(&m_seabedNoise, globalX, globalZ);
            seabedNoise = (seabedNoise + 1.0f) * 0.5f; // [0, 1]
            
            float rippleNoise = fnlGetNoise2D(&m_rippleNoise, globalX, globalZ);
            rippleNoise = (rippleNoise + 1.0f) * 0.5f; // [0, 1]
            
            int seabedHeight = BASE_SEABED + static_cast<int>(seabedNoise * SEABED_AMPLITUDE);
            seabedHeight += static_cast<int>(rippleNoise * 2.0f);
            
            int waterLevel = seabedHeight + WATER_DEPTH;
            
            float siltValue = fnlGetNoise2D(&m_siltNoise, globalX, globalZ);
            bool isSilt = siltValue > -0.2f;

            for (int ly = 0; ly < CHUNK_HEIGHT; ++ly)
            {
                BlockType type = BlockType::AIR;

                if (ly == 0)
                {
                    type = BlockType::BEDROCK;
                }
                else if (ly <= seabedHeight)
                {
                    if (ly == seabedHeight)
                    {
                        type = isSilt ? BlockType::UTOPIA_SILT : BlockType::UTOPIA_SAND;
                    }
                    else
                    {
                        type = BlockType::UTOPIA_SAND;
                    }
                }
                else if (ly <= waterLevel)
                {
                    type = BlockType::UTOPIA_WATER;
                }
                
                chunk->setBlock(lx, ly, lz, type);
            }
        }
    }
}