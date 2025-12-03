#include "Renderer.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_set>

#include "Shader.hpp"
#include "TextureArray.hpp"
#include "World.hpp"
#include "Chunk.hpp"

Renderer::Renderer(): sampler(0), view_matrix(1.0f), projection_matrix(1.0f)
{
}

Renderer::~Renderer()
{
    for (auto& [chunk, meshData] : chunkMeshes) {
        glDeleteBuffers(1, &meshData.SSBO);
    }
    chunkMeshes.clear();
}

void Renderer::init(int width, int height)
{
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE); // MSAA if avaivable

    // subsystem initialization
    shader = std::make_unique<Shader>();
    if (!shader->load("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl")) {
        printf("Failed to load shaders\n");
        return;
    }

    std::vector<string> layers = {
        "textures/grass_top.png",
        "textures/grass_side.png",
        "textures/dirt.png"
    };
    texture_array = std::make_unique<TextureArray>(layers);
}

void Renderer::beginFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (shader) {
        shader->use();
        shader->setMat4("projection", projection_matrix);
        shader->setMat4("view", view_matrix);
        shader->setInt("u_Textures", 0);
    }
    
    if(texture_array) {
        texture_array->bind(0);
    }
    
    // required dummy vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

void Renderer::shutdown()
{
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
}

void Renderer::endFrame()
{
    //TODO
}

void Renderer::setViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
    view_matrix = view;
    projection_matrix = projection;
}

void Renderer::uploadChunkMesh(const Chunk* chunk)
{
    if (!chunk || chunkMeshes.find(chunk) != chunkMeshes.end()) {
        return;
    }
    
    ChunkMeshData meshData;
    
    glGenBuffers(1, &meshData.SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshData.SSBO);
    
    const auto& quads = chunk->getQuads();
    glBufferData(GL_SHADER_STORAGE_BUFFER, quads.size() * sizeof(Quad), quads.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    meshData.quadCount = chunk->getQuadCount();
    chunkMeshes[chunk] = meshData;
}

void Renderer::drawChunk(const Chunk& chunk, const glm::mat4& model)
{
    auto it = chunkMeshes.find(&chunk);
    if (it == chunkMeshes.end()) {
        return;
    }
    
    if (shader) {
        shader->setMat4("model", model);
    }
    
    const ChunkMeshData& meshData = it->second;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshData.SSBO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, meshData.quadCount);
}

void Renderer::drawWorld(const World& world)
{
    std::unordered_set<const Chunk*> activeChunks;

    for (auto& [id, chunk] : world.getChunks())
    {
        activeChunks.insert(chunk.get());

        // Upload mesh if not already cached
        uploadChunkMesh(chunk.get());
        
        int x = static_cast<int>(id >> 32);
        int z = static_cast<int>(id & 0xFFFFFFFF);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x * CHUNK_SIZE, 0, z * CHUNK_SIZE));
        drawChunk(*chunk, model);
    }

    // clean meshes
    for (auto it = chunkMeshes.begin(); it != chunkMeshes.end();)
    {
        if(activeChunks.find(it->first) == activeChunks.end()) {
            glDeleteBuffers(1, &it->second.SSBO);
            it = chunkMeshes.erase(it);
        } else ++it;
    }
}