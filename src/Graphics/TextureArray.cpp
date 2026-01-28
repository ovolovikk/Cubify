#include "Graphics/TextureArray.hpp"

#include "Graphics/Renderer.hpp"
#include "Core/Logging/Log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureArray::TextureArray(const std::vector<std::string>& layer_paths)
{
    LOGI("[TextureArray] Number of textures: %d", layer_paths.size());
    layers_count = static_cast<int>(layer_paths.size());
    if (layers_count == 0) return;

    int components_count = 0;
    std::vector<unsigned char*> images(layers_count, nullptr);

    // for all layers validate 16x16 is given
    components_count = 4;
    for (int i = 0; i < layers_count; ++i)
    {
        int w = 0, h = 0, comp = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(layer_paths[i].c_str(), &w, &h, &comp, 4);
        if (!data) {
            LOGE("[TextureArray] Failed to load texture layer: %s", layer_paths[i].c_str());
            for (int j = 0; j < i; ++j) stbi_image_free(images[j]);
            return;
        }
        if (i == 0) {
            p_x = w;
            p_y = h;
        } else {
            if (w != p_x || h != p_y) {
                LOGE("[TextureArray] layer %d size mismatch (%dx%d vs %dx%d)", i, w, h, p_x, p_y);
                stbi_image_free(data);
                for (int j = 0; j < i; ++j) stbi_image_free(images[j]);
                return;
            }
        }
        images[i] = data;
    }

    glGenTextures(1, &texture_ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_ID);

    GLint internal_format = (components_count == 4) ? GL_RGBA8 : GL_RGBA;
    GLenum source_format = (components_count == 4) ? GL_RGBA : GL_RGB;

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internal_format, 
                p_x, p_y, layers_count,
                0, source_format, GL_UNSIGNED_BYTE, nullptr);
    
    for ( int i = 0; i < layers_count;++i)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
                        p_x, p_y, 1,
                        source_format, GL_UNSIGNED_BYTE, images[i]);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANISO);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    for (int i = 0; i < layers_count; ++i) {
        stbi_image_free(images[i]);
    }
}

TextureArray::~TextureArray()
{
    glDeleteTextures(1, &texture_ID);
}

void TextureArray::bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_ID);
}

void TextureArray::unbind() const
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
