#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>

#include "Core/BlockType.hpp"
#include "Graphics/Quad.hpp"
#include "Graphics/Mesh.hpp"
#include "Math/AABB.hpp"

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
    const std::vector<Quad>& getTransparentQuads() const { return transparentQuads; }
    void addQuad(const Quad& quad);
    void addTransparentQuad(const Quad& quad);
    void clearQuads();

    bool isDirty() const { return dirty; }
    void setDirty(bool d) { dirty = d; }
    bool hasUnsavedChanges() const { return unsaved_changes; }
    void setUnsavedChanges(bool d) { unsaved_changes = d; }
    int getChunkX() const { return chunkX; }
    int getChunkZ() const { return chunkZ; }
    

    Mesh& getMesh() { return mesh; }
    Mesh& getTransparentMesh() { return transparentMesh; }
    
    AABB getAABB() const;

    friend class ChunkManager;
private:
    bool isValidCoordinates(int x, int y, int z) const;

    Mesh mesh;
    Mesh transparentMesh;
    BlockType blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    std::vector<Quad> quads;
    std::vector<Quad> transparentQuads;

    bool dirty = false;
    bool unsaved_changes = false;
    int chunkX, chunkZ;
};

#endif // CHUNK_HPP
