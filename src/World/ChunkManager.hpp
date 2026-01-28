#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include <glm/vec3.hpp>

#include "World/Chunk.hpp"
#include "World/Generators/ITerrainGenerator.hpp"
#include "World/WorldType.hpp"
#include "Utils/Config.hpp"

class ChunkManager
{
public:
    explicit ChunkManager(WorldType worldType = WorldType::MINECRAFT);
    ~ChunkManager();

    ChunkManager(const ChunkManager&) = delete;
    ChunkManager& operator=(const ChunkManager&) = delete;

    void update(glm::vec3 player_pos);

    Chunk* getChunk(int x, int z);
    const Chunk* getChunk(int x, int z) const;
    const auto& getChunks() const { return chunks; }

    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);

    int getRenderDistance() const { return Config::Get().gConfig.renderDistance; }
    WorldType getWorldType() const { return m_worldType; }
private:
    std::unordered_map<long long, std::unique_ptr<Chunk>> chunks;
    std::unique_ptr<ITerrainGenerator> terrain_generator;
    WorldType m_worldType;
    
    // Optimization: track last player chunk to skip unnecessary updates
    int m_lastPlayerChunkX = INT_MIN;
    int m_lastPlayerChunkZ = INT_MIN;
    bool m_forceUpdate = true;
    
    // Ленивая загрузка: максимум чанков за кадр (при обычном движении)
    static constexpr int MAX_CHUNKS_PER_FRAME = 8;

    bool addChunk(int x, int z);  // true если чанк создан
    void removeChunk(int x, int z);
    long long getChunkId(int x, int z) const;

    void saveChunk(Chunk* chunk);
    std::string getChunkFileName(int x, int z) const;
    std::string getWorldFolderName() const;
    
    std::string m_saveFolder;  // путь к папке сохранений этого мира
};

#endif // CHUNK_MANAGER_HPP
