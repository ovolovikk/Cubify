#pragma once

#include "FastNoiseLite.h"

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

    static constexpr auto MIN_HEIGHT = 30;
    static constexpr auto AMPLITUDE = 80;

    static constexpr auto FREQUENCY = 0.005f;
    static constexpr auto OCTAVES = 4;
    static constexpr auto LACUNARITY = 2.0f;
    static constexpr auto GAIN = 0.5f;
};
