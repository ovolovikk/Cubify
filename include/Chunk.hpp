#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "BlockType.hpp"
#include <vector>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 64;

class Chunk; // Forward declaration

struct ChunkNeighbors {
    const Chunk* left = nullptr;
    const Chunk* right = nullptr;
    const Chunk* back = nullptr;
    const Chunk* front = nullptr;
};

struct Quad 
{
    float x, y, z;
    float w, h;
    float layer;
    int perpendicular_axis; // 0-x, 1-y, 2-z
    int back_face;
};

class Chunk
{
public:
    Chunk(int x, int z);

    void constructMesh(const ChunkNeighbors& neighbors);
    
    bool isDirty() const { return dirty; }
    void setDirty(bool d) { dirty = d; }
    
    const std::vector<Quad>& getQuads() const { return quads; }
    size_t getQuadCount() const { return quads.size(); }
    
private:
    BlockType blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];

    // the mesh stored there
    std::vector<Quad> quads;
    bool dirty = false;

    int chunkX, chunkZ;

    bool isBlockAir(int x, int y, int z) const;
    BlockType getBlock(int x, int y, int z) const;

    float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f; 

    // helpers to add specific faces
    void addQuad(float x, float y, float z,
                 float w, float h,
                 float layer,
                 int perpendicular_axis,
                 bool back_face);
};

#endif // CHUNK_HPP
