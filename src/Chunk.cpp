#include "Chunk.hpp"

#define FNL_IMPL
#include "FastNoiseLite.h"

Chunk::Chunk(int x, int z) : chunkX(x), chunkZ(z)
{
    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.seed = 12345678;
    noise.frequency = 0.02f;

    for(var lx = 0; lx < CHUNK_SIZE; ++lx)
    {
        for(var lz = 0; lz < CHUNK_SIZE; ++lz)
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

void Chunk::constructMesh()
{
    vertices.clear();
    texCoords.clear();

    for(int x = 0;x < CHUNK_SIZE;++x)
        for(int y = 0;y < CHUNK_HEIGHT;++y)
            for(int z = 0;z < CHUNK_SIZE;++z)
            {
                if(blocks[x][y][z] == BlockType::AIR) continue;

                float layer = 0.0f;

                // Grass Top: 0, Grass Side: 1, Dirt: 2
                
                if(isBlockAir(x, y, z + 1)) { // Front
                    if (blocks[x][y][z] == BlockType::GRASS) layer = 1.0f; // Side
                    else layer = 2.0f; // Dirt
                    addFaceFront(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), layer);
                }
                if(isBlockAir(x, y, z - 1)) { // Back
                    if (blocks[x][y][z] == BlockType::GRASS) layer = 1.0f; // Side
                    else layer = 2.0f; // Dirt
                    addFaceBack(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), layer);
                }
                if(isBlockAir(x - 1, y, z)) { // Left
                    if (blocks[x][y][z] == BlockType::GRASS) layer = 1.0f; // Side
                    else layer = 2.0f; // Dirt
                    addFaceLeft(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), layer);
                }
                if(isBlockAir(x + 1, y, z)) { // Right
                    if (blocks[x][y][z] == BlockType::GRASS) layer = 1.0f; // Side
                    else layer = 2.0f; // Dirt
                    addFaceRight(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), layer);
                }
                if(isBlockAir(x, y + 1, z)) { // Top
                    if (blocks[x][y][z] == BlockType::GRASS) layer = 0.0f; // Top
                    else layer = 2.0f; // Dirt
                    addFaceTop(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), layer);
                }
                if(isBlockAir(x, y - 1, z)) { // Bottom
                    layer = 2.0f; // Dirt for both
                    addFaceBottom(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), layer);
                }
            }
    
    vertex_count = vertices.size() / 3;
}

bool Chunk::isBlockAir(int x, int y, int z) const
{
    if(x < 0 || x >= CHUNK_SIZE ||
       y < 0 || y >= CHUNK_HEIGHT ||
       z < 0 || z >= CHUNK_SIZE)
    {
        return true;
    }

    return blocks[x][y][z] == BlockType::AIR;
}


// (0;0;0) is bottom back left corner
void Chunk::addFaceFront(float x, float y, float z, float layer)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right 
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // top left
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
}

void Chunk::addFaceBack(float x, float y, float z, float layer)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // bottom right
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);

    // second triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
}

void Chunk::addFaceLeft(float x, float y, float z, float layer)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    // second triangle
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
}

void Chunk::addFaceRight(float x, float y, float z, float layer)
{
    // first triangle
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // bottom right
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top left
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
}

void Chunk::addFaceTop(float x, float y, float z, float layer)
{
    // first triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom right
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom right
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
}

void Chunk::addFaceBottom(float x, float y, float z, float layer)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(1.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // top right
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);

    // second triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // top left
    texCoords.push_back(0.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(0.0f);  texCoords.push_back(1.0f);  texCoords.push_back(layer);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // top right
    texCoords.push_back(1.0f);  texCoords.push_back(0.0f);  texCoords.push_back(layer);
}

