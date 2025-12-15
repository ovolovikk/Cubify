#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include <glm/vec3.hpp>

#include "World/Chunk.hpp"
#include "World/TerrainGenerator.hpp"

class ChunkManager
{
public:
    ChunkManager();
    ~ChunkManager();

    ChunkManager(const ChunkManager&) = delete;
    ChunkManager& operator=(const ChunkManager&) = delete;

    void update(glm::vec3 player_pos);

    Chunk* getChunk(int x, int z);
    const Chunk* getChunk(int x, int z) const;
    const auto& getChunks() const { return chunks; }

    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);

    int getRenderDistance() const { return RENDER_DISTANCE; }
private:
    std::unordered_map<long long, std::unique_ptr<Chunk>> chunks;
    TerrainGenerator terrain_generator;
    
    static constexpr auto RENDER_DISTANCE = 24;

    void addChunk(int x, int z);
    void removeChunk(int x, int z);
    long long getChunkId(int x, int z) const;

    void saveChunk(Chunk* chunk);
    std::string getChunkFileName(int x, int z) const;
};

#endif // CHUNK_MANAGER_HPP
