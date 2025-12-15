#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>
#include <cstdint>

#include "Helpers/BlockType.hpp"
#include "Helpers/Quad.hpp"
#include "Helpers/Mesh.hpp"

constexpr auto CHUNK_SIZE = 16;
constexpr auto CHUNK_HEIGHT = 128;

class Chunk
{
public:
    Chunk(int x, int z);
    ~Chunk();

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    void setBlock(int x, int y, int z, BlockType type);
    BlockType getBlock(int x, int y, int z) const;
    bool isBlockAir(int x, int y, int z) const;

    const std::vector<Quad>& getQuads() const { return quads; }
    void addQuad(const Quad& quad);
    void clearQuads();

    bool isDirty() const { return dirty; }
    void setDirty(bool d) { dirty = d; }
    bool hasUnsavedChanges() const { return unsaved_changes; }
    void setUnsavedChanges(bool d) { unsaved_changes = d; }
    int getChunkX() const { return chunkX; }
    int getChunkZ() const { return chunkZ; }
    
    int getSpawnY() const;
    bool isValidCoordinates(int x, int y, int z) const;

    Mesh& getMesh() { return mesh; }
    
    friend class ChunkManager;
private:
    Mesh mesh;
    BlockType blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    std::vector<Quad> quads;

    bool dirty = false;
    bool unsaved_changes = false;
    int chunkX, chunkZ;
};

#endif // CHUNK_HPP
