#pragma once

class Chunk;

class ITerrainGenerator
{
public:
    virtual ~ITerrainGenerator() = default;

    virtual void generateChunkTerrain(Chunk* chunk) = 0;

protected:
    ITerrainGenerator() = default;
};
