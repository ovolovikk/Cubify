#include "World/ChunkMesher.hpp"

#include "World/Chunk.hpp"

namespace {
    struct BlockTexture {
        float top, side, bottom;
    };

    constexpr BlockTexture BLOCK_TEXTURES[] = {
        {0.0f, 0.0f, 0.0f},   // AIR
        {2.0f, 2.0f, 2.0f},   // DIRT
        {3.0f, 3.0f, 3.0f},   // STONE
        {0.0f, 1.0f, 2.0f},   // GRASS
        {4.0f, 4.0f, 4.0f},   // SAND
        {5.0f, 5.0f, 5.0f},   // WOODEN_PLANK
        {6.0f, 6.0f, 6.0f},   // WATER
        {7.0f, 7.0f, 7.0f},   // BEDROCK
        {8.0f, 8.0f, 8.0f},   // ICE
        {9.0f, 10.0f, 11.0f}, // SECTORR_GRASS
        {11.0f, 11.0f, 11.0f},// SECTORR_DIRT
        {12.0f, 12.0f, 12.0f},// SECTORR_STONE
        {13.0f, 13.0f, 13.0f},// SECTORR_SAND
        {14.0f, 14.0f, 14.0f},// SECTORR_WATER
        {15.0f, 15.0f, 15.0f},// UTOPIA_SAND
        {16.0f, 16.0f, 16.0f},// UTOPIA_SILT
        {17.0f, 17.0f, 17.0f} // UTOPIA_WATER
    };
    
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
            if (neighbor == BlockType::AIR) return true;
            
            // Water next to water - don't render (hide internal faces)
            if (isTransparentBlock(currentType) && isTransparentBlock(neighbor)) return false;
            
            // Solid block next to water - render the solid face
            if (!isTransparentBlock(currentType) && isTransparentBlock(neighbor)) return true;
            
            // Water next to solid - render water face
            if (isTransparentBlock(currentType) && !isTransparentBlock(neighbor)) return true;
            
            return false;
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
                
                bool transparent = isTransparentBlock(type);
                
                // left
                if (shouldRenderFace(x - 1, y, z, type)) {
                    addQuad(chunk, (float)x, (float)y, (float)z, layerSide, 0, false, transparent);
                }
                // right
                if (shouldRenderFace(x + 1, y, z, type)) {
                    addQuad(chunk, (float)(x + 1), (float)y, (float)z, layerSide, 0, true, transparent);
                }

                // bottom
                if (shouldRenderFace(x, y - 1, z, type)) {
                    addQuad(chunk, (float)x, (float)y, (float)z, layerBottom, 1, false, transparent);
                }
                // top
                if (shouldRenderFace(x, y + 1, z, type)) {
                    addQuad(chunk, (float)x, (float)(y + 1), (float)z, layerTop, 1, true, transparent);
                }

                // back
                if (shouldRenderFace(x, y, z - 1, type)) {
                    addQuad(chunk, (float)x, (float)y, (float)z, layerSide, 2, false, transparent);
                }
                // front
                if (shouldRenderFace(x, y, z + 1, type)) {
                    addQuad(chunk, (float)x, (float)y, (float)(z + 1), layerSide, 2, true, transparent);
                }
            }
        }
    }
}

void ChunkMesher::addQuad(Chunk& chunk, float x, float y, float z,
    float layer,
    int perpendicular_axis,
    bool back_face,
    bool transparent)
{
    uint32_t ux = static_cast<uint32_t>(x);
    uint32_t uy = static_cast<uint32_t>(y);
    uint32_t uz = static_cast<uint32_t>(z);

    uint32_t packed_pos = (ux & 0x3FF) | ((uy & 0x3FF) << 10) | ((uz & 0x3FF) << 20);

    uint32_t normal_index = perpendicular_axis * 2 + (back_face ? 0 : 1);

    uint32_t ulayer = static_cast<uint32_t>(layer);
    uint32_t packed_data = (ulayer & 0x3FF) | ((normal_index & 0x7) << 10);

    if (transparent) {
        chunk.addTransparentQuad({ packed_pos, packed_data });
    } else {
        chunk.addQuad({ packed_pos, packed_data });
    }
}
