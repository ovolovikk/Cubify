#ifndef WORLD_HPP
#define WORLD_HPP

#include <memory>
#include <glm/glm.hpp>

#include "World/ChunkManager.hpp"
#include "Graphics/Renderer.hpp"

struct Frustum;

class World
{
public:
    World();
    ~World();

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    
    void setBlock(int x, int y, int z, BlockType type);
    void rayCastBreakBlock(glm::vec3 origin, glm::vec3 direction, float max_distance);

    void update(glm::vec3 player_pos);
    void draw(Renderer& renderer, const Frustum& frustum);

    glm::vec3 getSpawnPoint();

    int GetRenderDistance() const { return chunk_manager->getRenderDistance(); }

    const ChunkManager& getChunkManager() const { return *chunk_manager; }
    ChunkManager& getChunkManager() { return *chunk_manager; }

private:
    std::unique_ptr<ChunkManager> chunk_manager;
};

#endif // WORLD_HPP
