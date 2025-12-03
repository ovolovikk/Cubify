#include "Chunk.hpp"

#define FNL_IMPL
#include "FastNoiseLite.h"

Chunk::Chunk(int x, int z) : chunkX(x), chunkZ(z)
{
    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.seed = 12345678;
    noise.frequency = 0.02f;

    for(int lx = 0; lx < CHUNK_SIZE; ++lx)
    {
        for(int lz = 0; lz < CHUNK_SIZE; ++lz)
        {
            float globalX = static_cast<float>(chunkX * CHUNK_SIZE + lx);
            float globalZ = static_cast<float>(chunkZ * CHUNK_SIZE + lz);
            
            // get normalized noise value
            float noiseValue = fnlGetNoise2D(&noise, globalX, globalZ);
            int height = 32 + static_cast<int>(noiseValue * 16.0f);
            
            if (height < 0) height = 0;
            if (height >= CHUNK_HEIGHT) height = CHUNK_HEIGHT - 1;

            for(int ly = 0; ly < CHUNK_HEIGHT; ++ly)
            {
                if (ly > height)
                {
                    blocks[lx][ly][lz] = BlockType::AIR;
                }
                else if (ly == height)
                {
                    blocks[lx][ly][lz] = BlockType::GRASS;
                }
                else
                {
                    blocks[lx][ly][lz] = BlockType::DIRT;
                }
            }
        }
    }
}

BlockType Chunk::getBlock(int x, int y, int z) const
{
    if(x < 0 || x >= CHUNK_SIZE ||
       y < 0 || y >= CHUNK_HEIGHT ||
       z < 0 || z >= CHUNK_SIZE)
    {
        return BlockType::AIR;
    }
    return blocks[x][y][z];
}

bool Chunk::isBlockAir(int x, int y, int z) const
{
    return getBlock(x, y, z) == BlockType::AIR;
}


void Chunk::constructMesh(const ChunkNeighbors& neighbors)
{
    quads.clear();

    // Helper to check neighbors
    auto getBlockAt = [&](int x, int y, int z) -> BlockType
    {
        if (y < 0 || y >= CHUNK_HEIGHT) return BlockType::AIR;

        if (x < 0)
        {
            if (neighbors.left) return neighbors.left->getBlock(x + CHUNK_SIZE, y, z);
            return BlockType::AIR;
        }
        if (x >= CHUNK_SIZE)
        {
            if (neighbors.right) return neighbors.right->getBlock(x - CHUNK_SIZE, y, z);
            return BlockType::AIR;
        }
        if (z < 0)
        {
            if (neighbors.back) return neighbors.back->getBlock(x, y, z + CHUNK_SIZE);
            return BlockType::AIR;
        }
        if (z >= CHUNK_SIZE)
        {
            if (neighbors.front) return neighbors.front->getBlock(x, y, z - CHUNK_SIZE);
            return BlockType::AIR;
        }
        return blocks[x][y][z];
    };

    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < CHUNK_SIZE; ++z)
            {
                BlockType type = blocks[x][y][z];
                if (type == BlockType::AIR) continue;

                float layerTop = 2.0f;    // Default Dirt
                float layerSide = 2.0f;
                float layerBottom = 2.0f;

                if (type == BlockType::GRASS) {
                    layerTop = 0.0f;      // Grass Top
                    layerSide = 1.0f;     // Grass Side
                    layerBottom = 2.0f;   // Dirt (bottom of grass block)
                }

                // left
                if (getBlockAt(x - 1, y, z) == BlockType::AIR) {
                    addQuad((float)x, (float)y, (float)z, layerSide, 0, false);
                }
                // right
                if (getBlockAt(x + 1, y, z) == BlockType::AIR) {
                    addQuad((float)(x + 1), (float)y, (float)z, layerSide, 0, true);
                }

                // bottom
                if (getBlockAt(x, y - 1, z) == BlockType::AIR) {
                    addQuad((float)x, (float)y, (float)z, layerBottom, 1, false);
                }
                // top
                if (getBlockAt(x, y + 1, z) == BlockType::AIR) {
                    addQuad((float)x, (float)(y + 1), (float)z, layerTop, 1, true);
                }

                // back
                if (getBlockAt(x, y, z - 1) == BlockType::AIR) {
                    addQuad((float)x, (float)y, (float)z, layerSide, 2, false);
                }
                // front
                if (getBlockAt(x, y, z + 1) == BlockType::AIR) {
                    addQuad((float)x, (float)y, (float)(z + 1), layerSide, 2, true);
                }
            }
        }
    }
    
    dirty = false;
}

void Chunk::addQuad(float x, float y, float z,
                    float layer,
                    int perpendicular_axis,
                    bool back_face)
{ 
    uint32_t ux = static_cast<uint32_t>(x);
    uint32_t uy = static_cast<uint32_t>(y);
    uint32_t uz = static_cast<uint32_t>(z);

    uint32_t packed_pos = (ux & 0x3FF) | ((uy & 0x3FF) << 10) | ((uz & 0x3FF) << 20);

    uint32_t normal_index = perpendicular_axis * 2 + (back_face ? 0 : 1);
    
    uint32_t ulayer = static_cast<uint32_t>(layer);
    uint32_t packed_data = (ulayer & 0x3FF) | ((normal_index & 0x7) << 10);

    quads.push_back({packed_pos, packed_data});
}
