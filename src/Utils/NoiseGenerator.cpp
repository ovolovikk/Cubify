#include "Utils/NoiseGenerator.hpp"

#include "World/Chunk.hpp"

NoiseGenerator::NoiseGenerator(int seed_) : seed(seed_)
{
    noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.seed = seed;
    noise.frequency = 0.02f;
}

int NoiseGenerator::getBLockHeight(Chunk* chunk, int lx, int lz) const
{
    float globalX = static_cast<float>(chunk->getChunkX() * CHUNK_SIZE + lx);
    float globalZ = static_cast<float>(chunk->getChunkZ() * CHUNK_SIZE + lz);

    // get normalized noise value
    float noiseValue = fnlGetNoise2D(&noise, globalX, globalZ);
    int height = 32 + static_cast<int>(noiseValue * 16.0f);

    if (height < 0) height = 0;
    if (height >= CHUNK_HEIGHT) height = CHUNK_HEIGHT - 1;

    return height;
}