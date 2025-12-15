#define FNL_IMPL
#include "Utils/NoiseGenerator.hpp"

#include "World/Chunk.hpp"

NoiseGenerator::NoiseGenerator(int seed_) : seed(seed_)
{
    noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.fractal_type = FNL_FRACTAL_FBM;
    noise.octaves = OCTAVES;
    noise.lacunarity = LACUNARITY;
    noise.gain = GAIN;
    noise.seed = seed;
    noise.frequency = FREQUENCY;
}

int NoiseGenerator::getBLockHeight(Chunk* chunk, int lx, int lz) const
{
    float globalX = static_cast<float>(chunk->getChunkX() * CHUNK_SIZE + lx);
    float globalZ = static_cast<float>(chunk->getChunkZ() * CHUNK_SIZE + lz);

    // get normalized noise value
    float noiseValue = fnlGetNoise2D(&noise, globalX, globalZ);
    float n = (noiseValue + 1.0f) * 0.5f;

    n = n * n * n;

    int height = MIN_HEIGHT + static_cast<int>(n * AMPLITUDE);

    if (height < 0) height = 0;
    if (height >= CHUNK_HEIGHT) height = CHUNK_HEIGHT - 1;

    return height;
}