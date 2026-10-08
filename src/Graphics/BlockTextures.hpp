#pragma once

#include <vector>

struct BlockTextureLayers
{
    std::vector<unsigned char*> pixels;
    int width = 0;
    int height = 0;

    BlockTextureLayers() = default;
    ~BlockTextureLayers();

    BlockTextureLayers(const BlockTextureLayers&) = delete;
    BlockTextureLayers& operator=(const BlockTextureLayers&) = delete;
};

bool LoadBlockTextures(BlockTextureLayers& layers);
