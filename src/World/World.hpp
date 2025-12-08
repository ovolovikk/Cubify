#ifndef WORLD_HPP
#define WORLD_HPP

#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

#include "World/Chunk.hpp"
#include "Graphics/Shader.hpp"
#include "World/TerrainGenerator.hpp"
#include "Graphics/Renderer.hpp"

struct Frustum;

class World
{
public:
    World();
    ~World();

    void addChunk(int x, int z);
    void removeChunk(int x, int z);
    Chunk* getChunk(int x, int z);
    
    void update(glm::vec3 player_pos);
    void draw(Renderer& renderer, const Frustum& frustum);

    int getRenderDistance() const { return render_distance; }

    const std::unordered_map<long long, std::unique_ptr<Chunk>>& getChunks() const { return chunks; }

private:
    std::unordered_map<long long, std::unique_ptr<Chunk>> chunks;

    TerrainGenerator terrain_generator;
    uint8_t render_distance = 8;

    long long getChunkId(int x, int z) const;
};

#endif // WORLD_HPP