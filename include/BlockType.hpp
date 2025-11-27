#ifndef BLOCK_TYPE_HPP
#define BLOCK_TYPE_HPP

#include <cstdint>

enum class BlockType : uint8_t
{
    AIR = 0,
    DIRT,
    STONE,
    GRASS
};

#endif // BLOCK_TYPE_HPP
