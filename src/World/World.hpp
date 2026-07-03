#ifndef WORLD_HPP
#define WORLD_HPP

#include <glm/glm.hpp>

#include "Graphics/IRendererBackend.hpp"
#include "World/ChunkManager.hpp"
#include "World/WorldType.hpp"

struct Frustum;

struct RayCastResult
{
    bool success;
    glm::ivec3 block_position;
    glm::ivec3 previous_position;
};

class World
{
public:
    explicit World(WorldType worldType = WorldType::MINECRAFT);
    ~World();

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);
    RayCastResult rayCast(glm::vec3 origin, glm::vec3 direction, float max_distance) const;
    void rayCastBreakBlock(glm::vec3 origin, glm::vec3 direction, float max_distance);
    void rayCastPlaceBlock(glm::vec3 origin, glm::vec3 direction, float max_distance, BlockType type);

    void update(glm::vec3 player_pos);
    void draw(IRendererBackend& renderer, const Frustum& frustum);
    void prepareAllChunks(IRendererBackend& renderer);

    glm::vec3 getSpawnPoint();

    int GetRenderDistance() const { return chunk_manager->getRenderDistance(); }

private:
    std::unique_ptr<ChunkManager> chunk_manager;

    static constexpr auto RAYCAST_STEP = 0.05f;
};

#endif // WORLD_HPP
