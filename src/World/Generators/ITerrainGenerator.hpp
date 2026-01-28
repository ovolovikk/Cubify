#ifndef ITERRAIN_GENERATOR_HPP
#define ITERRAIN_GENERATOR_HPP

class Chunk;

class ITerrainGenerator
{
public:
    virtual ~ITerrainGenerator() = default;

    virtual void generateChunkTerrain(Chunk* chunk) = 0;

protected:
    ITerrainGenerator() = default;
};

#endif // ITERRAIN_GENERATOR_HPP
