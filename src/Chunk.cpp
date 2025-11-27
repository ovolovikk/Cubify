#include "Chunk.hpp"

Chunk::Chunk()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &uvVBO);

    for(int x = 0;x < CHUNK_SIZE;++x)
    {
        for(int y = 0;y < CHUNK_HEIGHT;++y)
        {
            for(int z = 0;z < CHUNK_SIZE;++z)
            {
                if (y == CHUNK_HEIGHT - 1)
                    blocks[x][y][z] = BlockType::GRASS;
                else
                    blocks[x][y][z] = BlockType::DIRT;
            }
        }
    }
}

Chunk::~Chunk()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &uvVBO);
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

                float u = 0.0f, v = 0.0f;

                // check all 6 sides if neighbor is air => draw the face pointing to it
                // Atlas layout (2x2):
                // Column 0: top = side grass (u=0.0,v=0.5), bottom = dirt (u=0.0,v=0.0)
                // Column 1: top = grass top (u=0.5,v=0.5)
                if(isBlockAir(x, y, z + 1)) { 
                    if (blocks[x][y][z] == BlockType::GRASS) { u = 0.0f; v = 0.5f; }
                    else { u = 0.0f; v = 0.0f; } // Dirt
                    addFaceFront(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), u, v);
                }
                if(isBlockAir(x, y, z - 1)) { 
                    if (blocks[x][y][z] == BlockType::GRASS) { u = 0.0f; v = 0.5f; }
                    else { u = 0.0f; v = 0.0f; } // Dirt
                    addFaceBack(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), u, v);
                }
                if(isBlockAir(x - 1, y, z)) { 
                    if (blocks[x][y][z] == BlockType::GRASS) { u = 0.0f; v = 0.5f; }
                    else { u = 0.0f; v = 0.0f; } // Dirt
                    addFaceLeft(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), u, v);
                }
                if(isBlockAir(x + 1, y, z)) { 
                    if (blocks[x][y][z] == BlockType::GRASS) { u = 0.0f; v = 0.5f; }
                    else { u = 0.0f; v = 0.0f; } // Dirt
                    addFaceRight(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), u, v);
                }
                if(isBlockAir(x, y + 1, z)) { 
                    if (blocks[x][y][z] == BlockType::GRASS) { u = 0.5f; v = 0.5f; } // Grass top (right column, top row)
                    else { u = 0.0f; v = 0.0f; } // Dirt for non-grass blocks
                    addFaceTop(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), u, v);
                }
                if(isBlockAir(x, y - 1, z)) { 
                    // Bottom is dirt for both
                    u = 0.0f; v = 0.0f;
                    addFaceBottom(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), u, v);
                }
            }
    
    vertex_count = vertices.size() / 3;

    glBindVertexArray(VAO);

    // position VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // uvVBO
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void Chunk::render()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
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
void Chunk::addFaceFront(float x, float y, float z, float u, float v)
{
    float u0 = u + inset;
    float v0 = v + inset;
    float u1 = u + tileSize - inset;
    float v1 = v + tileSize - inset;
    
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v0);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(u1);    texCoords.push_back(v0);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right 
    texCoords.push_back(u1);    texCoords.push_back(v1);

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right
    texCoords.push_back(u1);    texCoords.push_back(v1);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // top left
    texCoords.push_back(u0);    texCoords.push_back(v1);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v0);
}

void Chunk::addFaceBack(float x, float y, float z, float u, float v)
{
    
    float u0 = u + inset;
    float v0 = v + inset;
    float u1 = u + tileSize - inset;
    float v1 = v + tileSize - inset;
    
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    texCoords.push_back(u1);    texCoords.push_back(v0);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // bottom right
    texCoords.push_back(u0);    texCoords.push_back(v0);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(u0);    texCoords.push_back(v1);

    // second triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    texCoords.push_back(u1);    texCoords.push_back(v0);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(u1);    texCoords.push_back(v1);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(u0);    texCoords.push_back(v1);
}

void Chunk::addFaceLeft(float x, float y, float z, float u, float v)
{
    
    float u0 = u + inset;
    float v0 = v + inset;
    float u1 = u + tileSize - inset;
    float v1 = v + tileSize - inset;
    
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v0);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(u0);    texCoords.push_back(v1);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(u1);    texCoords.push_back(v0);
    
    // second triangle
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(u0);    texCoords.push_back(v1);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(u1);    texCoords.push_back(v0);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // top right
    texCoords.push_back(u1);    texCoords.push_back(v1);
}

void Chunk::addFaceRight(float x, float y, float z, float u, float v)
{

    float u0 = u + inset;
    float v0 = v + inset;
    float u1 = u + tileSize - inset;
    float v1 = v + tileSize - inset;
    
    // first triangle
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // bottom right
    texCoords.push_back(u1);    texCoords.push_back(v0);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(u1);    texCoords.push_back(v1);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v0);

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(u1);    texCoords.push_back(v1);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v0);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // top left
    texCoords.push_back(u0);    texCoords.push_back(v1);
}

void Chunk::addFaceTop(float x, float y, float z, float u, float v)
{

    float u0 = u + inset;
    float v0 = v + inset;
    float u1 = u + tileSize - inset;
    float v1 = v + tileSize - inset;
    
    // first triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z); // top right
    texCoords.push_back(u1);    texCoords.push_back(v0);
    
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom right
    texCoords.push_back(u1);    texCoords.push_back(v1);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(u0);    texCoords.push_back(v0);

    // second triangle
    vertices.push_back(x + 1);  vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom right
    texCoords.push_back(u1);    texCoords.push_back(v1);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z); // top left
    texCoords.push_back(u0);    texCoords.push_back(v0);
    
    vertices.push_back(x);      vertices.push_back(y + 1);  vertices.push_back(z + 1); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v1);
}

void Chunk::addFaceBottom(float x, float y, float z, float u, float v)
{
    
    float u0 = u + inset;
    float v0 = v + inset;
    float u1 = u + tileSize - inset;
    float v1 = v + tileSize - inset;
    
    // first triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v1);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z + 1); // bottom right
    texCoords.push_back(u1);    texCoords.push_back(v1);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // top right
    texCoords.push_back(u1);    texCoords.push_back(v0);

    // second triangle
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z); // top left
    texCoords.push_back(u0);    texCoords.push_back(v0);
    
    vertices.push_back(x);      vertices.push_back(y);      vertices.push_back(z + 1); // bottom left
    texCoords.push_back(u0);    texCoords.push_back(v1);
    
    vertices.push_back(x + 1);  vertices.push_back(y);      vertices.push_back(z); // top right
    texCoords.push_back(u1);    texCoords.push_back(v0);
}

