#include "Texture.hpp"

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::string& texture_path)
{
    int p_x = 32;
    int p_y = 32;
    int components_count = 3;
    stbi_set_flip_vertically_on_load(true);
    texture_data = stbi_load(texture_path.c_str(), &p_x, &p_y, &components_count, 3);
    if(texture_data == nullptr)
    {
        printf("couldn't initialize texture_data\n");
        return;
    }

    glGenTextures(1, &texture_ID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, p_x, p_y, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(texture_data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &texture_ID);
}