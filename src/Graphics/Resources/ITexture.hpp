#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct TextureDesc
{
    std::string debugName;
    int width = 0;
    int height = 0;
    std::vector<const uint8_t*> layers; // RGBA8 pixels, one entry per array layer
};

class ITexture
{
public:
    virtual ~ITexture() = default;
};
