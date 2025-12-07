#ifndef MESH_HPP
#define MESH_HPP

#include <GL/glew.h>
#include <cstddef>

struct Mesh {
    GLuint SSBO = 0;
    size_t quadCount = 0;
};

#endif // MESH_HPP
