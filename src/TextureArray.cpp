#include "TextureArray.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureArray::TextureArray(const std::vector<std::string>& layer_paths)
{
    layers_count = layer_paths.size();
    if (layers_count == 0) return;

    int components_count = 0;
    std::vector<unsigned char*> images(layers_count, nullptr);

    // for all layers validate 16x16 is given
    for (int i = 0; i < layers_count; ++i)
    {
        int w = 0, h = 0, comp = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(layer_paths[i].c_str(), &w, &h, &comp, 0);
        if (!data) {
            printf("Failed to load texture layer: %s\n", layer_paths[i].c_str());
            for (int j = 0; j < i; ++j) stbi_image_free(images[j]);
            return;
        }
        if (i == 0) {
            p_x = w;
            p_y = h;
            components_count = comp;
        } else {
            if (w != p_x || h != p_y || comp != components_count) {
                printf("layer %d size mismatch.", i);
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
    
    GLfloat Aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &Aniso);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, Aniso);

    // reduce angle-dependening artifactrs
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // avoiding sampling atlas parts
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
