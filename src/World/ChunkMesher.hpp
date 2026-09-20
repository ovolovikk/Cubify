#pragma once

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
    static void addQuad(Chunk& chunk, uint32_t x, uint32_t y, uint32_t z,
        uint32_t layer,
        int perpendicular_axis,
        bool back_face,
        bool transparent = false);
};
