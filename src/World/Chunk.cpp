#include "World/Chunk.hpp"

#include "Helpers/Quad.hpp"

Chunk::Chunk(int x,int z) :chunkX(x), chunkZ(z)
{
    for (int i = 0; i < CHUNK_SIZE; ++i)
        for (int j = 0; j < CHUNK_HEIGHT; ++j)
            for (int k = 0; k < CHUNK_SIZE; ++k) 
                blocks[i][j][k] = BlockType::AIR;
}

Chunk::~Chunk()
{
    if(mesh.SSBO != 0) {
        glDeleteBuffers(1, &mesh.SSBO);
    }
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
    dirty = true;
}

void Chunk::clearQuads()
{
    quads.clear();
    dirty = true;
}
