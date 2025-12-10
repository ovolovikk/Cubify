#ifndef NOISE_GENERATOR_HPP
#define NOISE_GENERATOR_HPP

#include "Helpers/FastNoiseLite.h"

class Chunk;

class NoiseGenerator
{
public:
    NoiseGenerator(int seed_ = 1);

    NoiseGenerator(const NoiseGenerator&) = delete;
    NoiseGenerator& operator=(const NoiseGenerator&) = delete;

    int getBLockHeight(Chunk* chunk, int lx, int lz) const;

private:
    int seed;
    fnl_state noise;
};

#endif // NOISE_GENERATOR_HPP
