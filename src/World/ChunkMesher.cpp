#include "World/ChunkMesher.hpp"

#include "World/Chunk.hpp"

namespace {
    struct BlockTexture {
        float top, side, bottom;
    };

    constexpr BlockTexture BLOCK_TEXTURES[] = {
        {0.0f, 0.0f, 0.0f}, // AIR
        {2.0f, 2.0f, 2.0f}, // DIRT
        {3.0f, 3.0f, 3.0f}, // STONE
        {0.0f, 1.0f, 2.0f}, // GRASS
        {4.0f, 4.0f, 4.0f}, // SAND
        {5.0f, 5.0f, 5.0f}, // WOODEN_PLANK
        {6.0f, 6.0f, 6.0f}, // WATER
        {7.0f, 7.0f, 7.0f}  // BEDROCK
    };
}

void ChunkMesher::generateMesh(Chunk& chunk, const ChunkNeighbors& neighbors)
{
    chunk.clearQuads();

    // helper to check neighbors
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
            return chunk.getBlock(x, y, z);
        };

    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < CHUNK_SIZE; ++z)
            {
                BlockType type = chunk.getBlock(x, y, z);
                if (type == BlockType::AIR) continue;
                
                const auto& textures = BLOCK_TEXTURES[static_cast<size_t>(type)];
                float layerTop = textures.top;
                float layerSide = textures.side;
                float layerBottom = textures.bottom;
                
                // left
                if (getBlockAt(x - 1, y, z) == BlockType::AIR) {
                    addQuad(chunk, (float)x, (float)y, (float)z, layerSide, 0, false);
                }
                // right
                if (getBlockAt(x + 1, y, z) == BlockType::AIR) {
                    addQuad(chunk, (float)(x + 1), (float)y, (float)z, layerSide, 0, true);
                }

                // bottom
                if (getBlockAt(x, y - 1, z) == BlockType::AIR) {
                    addQuad(chunk, (float)x, (float)y, (float)z, layerBottom, 1, false);
                }
                // top
                if (getBlockAt(x, y + 1, z) == BlockType::AIR) {
                    addQuad(chunk, (float)x, (float)(y + 1), (float)z, layerTop, 1, true);
                }

                // back
                if (getBlockAt(x, y, z - 1) == BlockType::AIR) {
                    addQuad(chunk, (float)x, (float)y, (float)z, layerSide, 2, false);
                }
                // front
                if (getBlockAt(x, y, z + 1) == BlockType::AIR) {
                    addQuad(chunk, (float)x, (float)y, (float)(z + 1), layerSide, 2, true);
                }
            }
        }
    }
}

void ChunkMesher::addQuad(Chunk& chunk, float x, float y, float z,
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

    chunk.addQuad({ packed_pos, packed_data });
}
