#include "BlockTextures.hpp"

#include "Logging/Log.hpp"
#include "stb_image.h"

// Preserve layer order, see meshing
static constexpr const char* BLOCK_TEXTURE_PATHS[] = {
    "assets/textures/grass_top.png",
    "assets/textures/grass_side.png",
    "assets/textures/dirt.png",
    "assets/textures/stone.png",
    "assets/textures/sand.png",
    "assets/textures/wooden_plank.png",
    "assets/textures/water.png",
    "assets/textures/bedrock.png",
    "assets/textures/ice.png",
    "assets/textures/sectorr_grass_top.png",
    "assets/textures/sectorr_grass_side.png",
    "assets/textures/sectorr_dirt.png",
    "assets/textures/sectorr_stone.png",
    "assets/textures/sectorr_sand.png",
    "assets/textures/sectorr_water.png",
    "assets/textures/utopia_sand.png",
    "assets/textures/utopia_silt.png",
    "assets/textures/utopia_water.png"
};

BlockTextureLayers::~BlockTextureLayers()
{
    for (auto* layer : pixels)
    {
        stbi_image_free(layer);
    }
}

bool LoadBlockTextures(BlockTextureLayers& layers)
{
    stbi_set_flip_vertically_on_load(true);

    for (const auto* path : BLOCK_TEXTURE_PATHS)
    {
        int width = 0;
        int height = 0;
        auto* pixels = stbi_load(path, &width, &height, nullptr, 4);
        if (!pixels)
        {
            LOGE("[BlockTextures] Failed to load texture layer: %s", path);
            return false;
        }

        layers.pixels.push_back(pixels);
        if (layers.pixels.size() == 1)
        {
            layers.width = width;
            layers.height = height;
        }
        else if (width != layers.width || height != layers.height)
        {
            LOGE("[BlockTextures] Texture layer size mismatch: %s", path);
            return false;
        }
    }

    return true;
}
