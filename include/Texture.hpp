#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>

class Texture
{
    Texture(const std::string& texture_path);
    ~Texture();

private:
    GLuint texture_ID;
    unsigned char* texture_data;
};

#endif // TEXTURE_HPP
