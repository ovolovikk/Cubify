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

    const float inset = 0.005f; // prevent atlas bleeding
    const float tileSize = 0.5f; // each tile is 0.5x0.5 for 2x2 atlas

    // helpers to add specific faces
    void addFaceFront(float x, float y, float z, float u, float v);
    void addFaceBack(float x, float y, float z, float u, float v);
    void addFaceLeft(float x, float y, float z, float u, float v);
    void addFaceRight(float x, float y, float z, float u, float v);
    void addFaceTop(float x, float y, float z, float u, float v);
    void addFaceBottom(float x, float y, float z, float u, float v);
};

#endif // CHUNK_HPP
