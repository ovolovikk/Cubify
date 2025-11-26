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

    GLuint VAO, VBO;
    size_t vertex_count;

    int chunkX, chunkZ;

    bool isBlockAir(int x, int y, int z) const;

    // helpers to add specific faces
    void addFaceFront(int x, int y, int z);
    void addFaceBack(int x, int y, int z);
    void addFaceLeft(int x, int y, int z);
    void addFaceRight(int x, int y, int z);
    void addFaceTop(int x, int y, int z);
    void addFaceBottom(int x, int y, int z);
};

#endif // CHUNK_HPP
