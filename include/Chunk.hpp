#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "BlockType.hpp"

#include <GL/glew.h>
#include <vector>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 2;

class Chunk
{
public:
    Chunk();
    ~Chunk();

    void constructMesh();
    void render();
private:
    BlockType blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

    // the mesh stored there
    std::vector<float> vertices;
    std::vector<float> texCoords;

    GLuint VAO, VBO, uvVBO;
    size_t vertex_count;

    int chunkX, chunkZ;

    bool isBlockAir(int x, int y, int z) const;

    // helpers to add specific faces
    void addFaceFront(int x, int y, int z, float u, float v);
    void addFaceBack(int x, int y, int z, float u, float v);
    void addFaceLeft(int x, int y, int z, float u, float v);
    void addFaceRight(int x, int y, int z, float u, float v);
    void addFaceTop(int x, int y, int z, float u, float v);
    void addFaceBottom(int x, int y, int z, float u, float v);
};

#endif // CHUNK_HPP
