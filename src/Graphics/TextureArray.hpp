#ifndef TEXTURE_ARRAY_HPP
#define TEXTURE_ARRAY_HPP

#include <GL/glew.h>

#include <string>
#include <vector>

class TextureArray
{
public:
    TextureArray(const std::vector<std::string>& layer_paths);
    ~TextureArray();

    TextureArray(const TextureArray&) = delete;
    TextureArray& operator=(const TextureArray&) = delete;

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    int getWidth() const { return p_x; }
    int getHeight() const { return p_y; }
    int getLayersCount() const { return layers_count; }
private:
    GLuint texture_ID;
    int p_x, p_y, layers_count;
};

#endif // TEXTURE_ARRAY_HPP
