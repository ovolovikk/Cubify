#ifndef QUAD_HPP
#define QUAD_HPP

#include <cstdint>

struct Quad
{
    uint32_t packed_position; // x(10) | y(10) | z(10)
    uint32_t packed_data;     // layer(10) | normal_index(3)
};

#endif // QUAD_HPP
