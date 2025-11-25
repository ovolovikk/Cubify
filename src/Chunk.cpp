#include "Chunk.hpp"

Chunk::Chunk()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    for(int x = 0;x < CHUNK_SIZE;++x)
    {
        for(int y = 0;y < CHUNK_HEIGHT;++y)
        {
            for(int z = 0;z < CHUNK_SIZE;++z)
            {
                blocks[x][y][z] = BlockType::STONE;
            }
        }
    }
}

Chunk::~Chunk()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
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
    
    vertex_count = vertices.size() / 3;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

void Chunk::render()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
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


// (0;0;0) is bottom back left corner
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

void Chunk::addFaceBack(int x, int y, int z)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // bottom right
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right

    // second triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
}

void Chunk::addFaceLeft(int x, int y, int z)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    
    // second triangle
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right
}

void Chunk::addFaceRight(int x, int y, int z)
{
    // first triangle
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // bottom right
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom left

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top left
}

void Chunk::addFaceTop(int x, int y, int z)
{
    // first triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom right
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom right
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom left
}

void Chunk::addFaceBottom(int x, int y, int z)
{
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // top right

    // second triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // top left
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // top right
}