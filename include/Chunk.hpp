#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "BlockType.hpp"
#include <vector>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 2;

class Chunk
{
public:
    Chunk();

    void constructMesh();
    
    const std::vector<float>& getVertices() const { return vertices; }
    const std::vector<float>& getTexCoords() const { return texCoords; }
    size_t getVertexCount() const { return vertex_count; }
    
private:
    BlockType blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

    // the mesh stored there
    std::vector<float> vertices;
    std::vector<float> texCoords;
    size_t vertex_count;

    int chunkX, chunkZ;

    bool isBlockAir(int x, int y, int z) const;

    float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f; 

    // helpers to add specific faces
    void addFaceFront(float x, float y, float z, float layer);
    void addFaceBack(float x, float y, float z, float layer);
    void addFaceLeft(float x, float y, float z, float layer);
    void addFaceRight(float x, float y, float z, float layer);
    void addFaceTop(float x, float y, float z, float layer);
    void addFaceBottom(float x, float y, float z, float layer);
};

#endif // CHUNK_HPP
