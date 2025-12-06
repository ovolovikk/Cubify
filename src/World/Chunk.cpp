#include "World/Chunk.hpp"

#include "Graphics/Quad.hpp"

Chunk::Chunk(int x,int z) :chunkX(x), chunkZ(z)
{
    std::fill(&blocks[0][0][0], &blocks[0][0][0] + sizeof(blocks), BlockType::AIR);
}

void Chunk::setBlock(int x, int y, int z, BlockType type)
{
    if (isValidCoordinates(x, y, z)) {
        blocks[x][y][z] = type;
        dirty = true;
    }
}

BlockType Chunk::getBlock(int x, int y, int z) const
{
    if(!isValidCoordinates(x, y, z)) {
       return BlockType::AIR;
    }
    return blocks[x][y][z];
}

bool Chunk::isBlockAir(int x, int y, int z) const
{
    return getBlock(x, y, z) == BlockType::AIR;
}

bool Chunk::isValidCoordinates(int x, int y, int z) const
{
    return (x >= 0 && x < CHUNK_SIZE &&
        y >= 0 && y < CHUNK_HEIGHT &&
        z >= 0 && z < CHUNK_SIZE);
}

void Chunk::addQuad(const Quad& quad)
{
    quads.push_back(quad);
}

void Chunk::clearQuads()
{
    quads.clear();
}
