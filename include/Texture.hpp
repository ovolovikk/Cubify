#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>
#include <GL/glew.h>

class Texture
{
public:
    Texture(const std::string& texture_path);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

private:
    GLuint texture_ID;
    unsigned char* texture_data;
};

#endif // TEXTURE_HPP
