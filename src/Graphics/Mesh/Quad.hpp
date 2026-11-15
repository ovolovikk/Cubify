#pragma once

#include "PrecompilerHeader.hpp"

struct Quad
{
    uint32_t packed; // x(5) | y(9) | z(5) | normal_index(3) | layer(10), numbers are in bits for each value
};
