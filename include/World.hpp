#ifndef WORLD_HPP
#define WORLD_HPP

#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

#include "Chunk.hpp"
#include "Shader.hpp"

class World
{
public:
    World();
    ~World();

    void addChunk(int x, int z);
    Chunk* getChunk(int x, int z);

    void render(Shader& shader);

private:
    std::unordered_map<long long, std::unique_ptr<Chunk>> chunks;

    long long getChunkId(int x, int z) const;
};

#endif // WORLD_HPP