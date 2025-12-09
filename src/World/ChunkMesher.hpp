#ifndef CHUNK_MESHER_HPP
#define CHUNK_MESHER_HPP

#include "World/Chunk.hpp"

struct ChunkNeighbors {
    Chunk* left = nullptr;
    Chunk* right = nullptr;
    Chunk* back = nullptr;
    Chunk* front = nullptr;
};

class ChunkMesher
{
public:
    ChunkMesher() = delete;
    ChunkMesher(const ChunkMesher&) = delete;
    ChunkMesher& operator=(const ChunkMesher&) = delete;

    static void generateMesh(Chunk& chunk, const ChunkNeighbors& neighbors);

private:
    static void addQuad(Chunk& chunk, float x, float y, float z,
        float layer,
        int perpendicular_axis,
        bool back_face);
};

#endif // !CHUNK_MESHER_HPP
