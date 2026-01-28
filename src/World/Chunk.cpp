#include "World/Chunk.hpp"

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
    if(transparentMesh.SSBO != 0) {
        glDeleteBuffers(1, &transparentMesh.SSBO);
    }
}

void Chunk::setBlock(int x, int y, int z, BlockType type)
{
    if (isValidCoordinates(x, y, z)) {
        blocks[x][y][z] = type;
        dirty = true;
        unsaved_changes = true;
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

void Chunk::addQuad(const Quad& quad)
{
    quads.push_back(quad);
    dirty = true;
}

void Chunk::addTransparentQuad(const Quad& quad)
{
    transparentQuads.push_back(quad);
    dirty = true;
}

void Chunk::clearQuads()
{
    quads.clear();
    transparentQuads.clear();
    dirty = true;
}

AABB Chunk::getAABB() const
{
    AABB box;
    float worldX = static_cast<float>(chunkX * CHUNK_SIZE);
    float worldZ = static_cast<float>(chunkZ * CHUNK_SIZE);

    box.min = glm::vec3(worldX, 0.f, worldZ);
    box.max = glm::vec3(worldX + CHUNK_SIZE, CHUNK_HEIGHT, worldZ + CHUNK_SIZE);

    return box;
}

bool Chunk::isValidCoordinates(int x, int y, int z) const
{
    return (x >= 0 && x < CHUNK_SIZE &&
        y >= 0 && y < CHUNK_HEIGHT &&
        z >= 0 && z < CHUNK_SIZE);
}
