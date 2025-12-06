#ifndef CHUNK_MESHER_HPP
#define CHUNK_MESHER_HPP

#include "Chunk.hpp"

struct ChunkNeighbors {
    const Chunk* left = nullptr;
    const Chunk* right = nullptr;
    const Chunk* back = nullptr;
    const Chunk* front = nullptr;
};

class ChunkMesher
{
public:
    static void generateMesh(Chunk& chunk, const ChunkNeighbors& neighbors);

private:
    static void addQuad(Chunk& chunk, float x, float y, float z,
        float layer,
        int perpendicular_axis,
        bool back_face);
};

#endif // !CHUNK_MESHER_HPP
