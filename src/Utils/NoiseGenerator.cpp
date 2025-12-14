#include "Utils/NoiseGenerator.hpp"

#include "World/Chunk.hpp"

NoiseGenerator::NoiseGenerator(int seed_) : seed(seed_)
{
    noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.fractal_type = FNL_FRACTAL_FBM;
    noise.octaves = 4;
    noise.lacunarity = 2.f;
    noise.gain = 0.5f;
    noise.seed = seed;
    noise.frequency = 0.005f;
}

int NoiseGenerator::getBLockHeight(Chunk* chunk, int lx, int lz) const
{
    float globalX = static_cast<float>(chunk->getChunkX() * CHUNK_SIZE + lx);
    float globalZ = static_cast<float>(chunk->getChunkZ() * CHUNK_SIZE + lz);

    // get normalized noise value
    float noiseValue = fnlGetNoise2D(&noise, globalX, globalZ);
    float n = (noiseValue + 1.0f) * 0.5f;

    n = n * n * n;

    int minHeight = 30;
    int amplitude = 80;

    int height = minHeight + static_cast<int>(n * amplitude);

    if (height < 0) height = 0;
    if (height >= CHUNK_HEIGHT) height = CHUNK_HEIGHT - 1;

    return height;
}