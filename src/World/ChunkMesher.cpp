#include "World/ChunkMesher.hpp"

namespace {
    struct BlockTexture {
        uint32_t top, side, bottom;
    };

    constexpr BlockTexture BLOCK_TEXTURES[] = {
        {0, 0, 0},    // AIR
        {2, 2, 2},    // DIRT
        {3, 3, 3},    // STONE
        {0, 1, 2},    // GRASS
        {4, 4, 4},    // SAND
        {5, 5, 5},    // WOODEN_PLANK
        {6, 6, 6},    // WATER
        {7, 7, 7},    // BEDROCK
        {8, 8, 8},    // ICE
        {9, 10, 11},  // SECTORR_GRASS
        {11, 11, 11}, // SECTORR_DIRT
        {12, 12, 12}, // SECTORR_STONE
        {13, 13, 13}, // SECTORR_SAND
        {14, 14, 14}, // SECTORR_WATER
        {15, 15, 15}, // UTOPIA_SAND
        {16, 16, 16}, // UTOPIA_SILT
        {17, 17, 17}  // UTOPIA_WATER
    };
    static_assert(std::size(BLOCK_TEXTURES) == 17);
    
    bool isTransparentBlock(BlockType type) {
        return type == BlockType::WATER || type == BlockType::SECTORR_WATER || type == BlockType::UTOPIA_WATER;
    }
}

void ChunkMesher::generateMesh(Chunk& chunk, const ChunkNeighbors& neighbors)
{
    chunk.clearQuads();

    // helper to check if neighbor is air or transparent (for water to show through)
    auto shouldRenderFace = [&](int x, int y, int z, BlockType currentType) -> bool
        {
            BlockType neighbor = BlockType::AIR;
            if (y < 0 || y >= CHUNK_HEIGHT) return true;

            if (x < 0)
            {
                if (neighbors.left) neighbor = neighbors.left->getBlock(x + CHUNK_SIZE, y, z);
            }
            else if (x >= CHUNK_SIZE)
            {
                if (neighbors.right) neighbor = neighbors.right->getBlock(x - CHUNK_SIZE, y, z);
            }
            else if (z < 0)
            {
                if (neighbors.back) neighbor = neighbors.back->getBlock(x, y, z + CHUNK_SIZE);
            }
            else if (z >= CHUNK_SIZE)
            {
                if (neighbors.front) neighbor = neighbors.front->getBlock(x, y, z - CHUNK_SIZE);
            }
            else
            {
                neighbor = chunk.getBlock(x, y, z);
            }
            
            // Air neighbor - always render face
            if (neighbor == BlockType::AIR)
            {
                return true;
            }
            return isTransparentBlock(currentType) != isTransparentBlock(neighbor);
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
                uint32_t layerTop = textures.top;
                uint32_t layerSide = textures.side;
                uint32_t layerBottom = textures.bottom;
                
                bool transparent = isTransparentBlock(type);
                
                // left
                if (shouldRenderFace(x - 1, y, z, type)) {
                    addQuad(chunk, x, y, z, layerSide, 0, false, transparent);
                }
                // right
                if (shouldRenderFace(x + 1, y, z, type)) {
                    addQuad(chunk, x + 1, y, z, layerSide, 0, true, transparent);
                }

                // bottom
                if (shouldRenderFace(x, y - 1, z, type)) {
                    addQuad(chunk, x, y, z, layerBottom, 1, false, transparent);
                }
                // top
                if (shouldRenderFace(x, y + 1, z, type)) {
                    addQuad(chunk, x, y + 1, z, layerTop, 1, true, transparent);
                }

                // back
                if (shouldRenderFace(x, y, z - 1, type)) {
                    addQuad(chunk, x, y, z, layerSide, 2, false, transparent);
                }
                // front
                if (shouldRenderFace(x, y, z + 1, type)) {
                    addQuad(chunk, x, y, z + 1, layerSide, 2, true, transparent);
                }
            }
        }
    }
}

void ChunkMesher::addQuad(Chunk& chunk, uint32_t x, uint32_t y, uint32_t z,
    uint32_t layer,
    int perpendicular_axis,
    bool back_face,
    bool transparent)
{
    uint32_t packed_pos = (x & 0x3FF) | ((y & 0x3FF) << 10) | ((z & 0x3FF) << 20);

    uint32_t normal_index = perpendicular_axis * 2 + (back_face ? 0 : 1);

    uint32_t packed_data = (layer & 0x3FF) | ((normal_index & 0x7) << 10);

    if (transparent) {
        chunk.addTransparentQuad({ packed_pos, packed_data });
    } else {
        chunk.addQuad({ packed_pos, packed_data });
    }
}
