#ifndef BLOCK_TYPE_HPP
#define BLOCK_TYPE_HPP

#include <cstdint>

// Using scope enum for specifying the type
enum class BlockType : uint8_t
{
    AIR = 0,
    DIRT,
    STONE,
    GRASS,
    SAND,
    WOODEN_PLANK,
    WATER,
    BEDROCK,
    ICE,
    EDMUNDS_GRASS,
    EDMUNDS_DIRT,
    EDMUNDS_STONE,
    EDMUNDS_SAND,
    EDMUNDS_WATER
};

#endif // BLOCK_TYPE_HPP
