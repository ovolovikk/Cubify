#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>

#include "World/Chunk.hpp"
#include "World/World.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/TextureArray.hpp"

class Renderer
{
public:
    Renderer();
    ~Renderer();
    
    void init(int width, int height);
    void resize(int width, int height);
    void shutdown();

    void beginFrame();
    void endFrame();

    void setViewProjection(const glm::mat4& view, const glm::mat4& projection);
    void uploadChunkMesh(Chunk* chunk);
    void drawChunk(const Chunk& chunk, const glm::mat4& model);
    void drawWorld(const World& world);

private:
    struct ChunkMeshData {
        GLuint SSBO;
        size_t quadCount;
    };
    
    std::unique_ptr<Shader> shader;
    std::unique_ptr<TextureArray> texture_array;
    GLuint sampler;
    GLuint vao;
    
    std::unordered_map<const Chunk*, ChunkMeshData> chunkMeshes;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
};

#endif // RENDERER_HPP
