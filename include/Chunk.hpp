#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "BlockType.hpp"
#include <vector>
#include <cstdint>

struct Quad;

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 64;

class Chunk
{
public:
    Chunk(int x, int z);   

    void setBlock(int x, int y, int z, BlockType type);
    BlockType getBlock(int x, int y, int z) const;
    bool isBlockAir(int x, int y, int z) const;

    const std::vector<Quad>& getQuads() const { return quads; }
    void addQuad(const Quad& quad);
    void clearQuads();

    bool isDirty() const { return dirty; }
    void setDirty(bool d) { dirty = d; }
    int getChunkX() const { return chunkX; }
    int getChunkZ() const { return chunkZ; }
    bool isValidCoordinates(int x, int y, int z) const;

private:
    BlockType blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    std::vector<Quad> quads;

    bool dirty = false;
    int chunkX, chunkZ;
};

#endif // CHUNK_HPP
