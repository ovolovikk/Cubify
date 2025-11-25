#include "Chunk.hpp"

Chunk::Chunk()
{
    for(int x = 0;x < CHUNK_SIZE;++x)
        for(int y = 0;y < CHUNK_HEIGHT;++y)
            for(int z = 0;z < CHUNK_SIZE;++z)
            {
                blocks[x][y][z] = BlockType::STONE;
            }
}

Chunk::~Chunk()
{

}

void Chunk::constructMesh()
{
    for(int x = 0;x < CHUNK_SIZE;++x)
        for(int y = 0;y < CHUNK_HEIGHT;++y)
            for(int z = 0;z < CHUNK_SIZE;++z)
            {
                if(blocks[x][y][z] == BlockType::AIR) continue;

                // check all 6 sides if neighbor is air => draw the face pointing to it
                if(isBlockAir(x, y, z + 1)) addFaceFront(x, y, z);
                if(isBlockAir(x, y, z - 1)) addFaceBack(x, y, z);
                if(isBlockAir(x - 1, y, z)) addFaceLeft(x, y, z);
                if(isBlockAir(x + 1, y, z)) addFaceRight(x, y, z);
                if(isBlockAir(x, y + 1, z)) addFaceTop(x, y, z);
                if(isBlockAir(x, y - 1, z)) addFaceBottom(x, y, z);
            } 
}

void Chunk::render()
{

}

bool Chunk::isBlockAir(int x, int y, int z)
{
    if(x < 0 || x >= CHUNK_SIZE ||
       y < 0 || y >= CHUNK_HEIGHT ||
       z < 0 || z >= CHUNK_SIZE)
    {
        return true;
    }

    return blocks[x][y][z] == BlockType::AIR;
}

void Chunk::addFaceFront(int x, int y, int z)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right 
    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // top left
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
}