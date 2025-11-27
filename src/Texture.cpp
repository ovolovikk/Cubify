#include "Texture.hpp"

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::string& texture_path)
{
    int p_x = 0;
    int p_y = 0;
    int components_count = 0;
    stbi_set_flip_vertically_on_load(true);
    texture_data = stbi_load(texture_path.c_str(), &p_x, &p_y, &components_count, 0);
    if(texture_data == nullptr)
    {
        printf("stbi_load failed for '%s': %s\n", texture_path.c_str(), stbi_failure_reason());
        return;
    }

    glGenTextures(1, &texture_ID);
    float aniso =0.0f;
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); // setting anisotropic filtering
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, p_x, p_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);


    // reduce angle-dependening artifactrs
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // avoiding sampling atlas parts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(texture_data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &texture_ID);
}

void Texture::bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}