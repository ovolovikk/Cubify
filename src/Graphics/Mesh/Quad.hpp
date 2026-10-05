#pragma once

#include "PrecompilerHeader.hpp"

// Pix suggested make this structure rounded to 16 bytes
struct Quad
{
    uint32_t packed_position; // x(10) | y(10) | z(10)
    uint32_t packed_data;     // layer(10) | normal_index(3)
    uint32_t padding1;
    uint32_t padding2;
};
