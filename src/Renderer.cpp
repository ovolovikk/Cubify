#include "Renderer.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"
#include "Texture.hpp"
#include "World.hpp"
#include "Chunk.hpp"

Renderer::Renderer(): sampler(0), view_matrix(1.0f), projection_matrix(1.0f)
{
}

Renderer::~Renderer()
{
    for (auto& [chunk, meshData] : chunkMeshes) {
        glDeleteVertexArrays(1, &meshData.VAO);
        glDeleteBuffers(1, &meshData.VBO);
        glDeleteBuffers(1, &meshData.uvVBO);
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

    texture_atlas = std::make_unique<Texture>("textures/grass_atlas.png");
}

void Renderer::shutdown()
{
    glfwTerminate();
}

void Renderer::beginFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (shader) {
        shader->use();
        shader->setMat4("projection", projection_matrix);
        shader->setMat4("view", view_matrix);
        shader->setInt("u_Atlas", 0);
    }
    
    if (texture_atlas) {
        texture_atlas->bind(0);
    }
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
    
    glGenVertexArrays(1, &meshData.VAO);
    glGenBuffers(1, &meshData.VBO);
    glGenBuffers(1, &meshData.uvVBO);
    
    glBindVertexArray(meshData.VAO);
    
    // upload position data
    const auto& vertices = chunk->getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, meshData.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // upload UV data
    const auto& texCoords = chunk->getTexCoords();
    glBindBuffer(GL_ARRAY_BUFFER, meshData.uvVBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
    
    meshData.vertexCount = chunk->getVertexCount();
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
    glBindVertexArray(meshData.VAO);
    glDrawArrays(GL_TRIANGLES, 0, meshData.vertexCount);
}

void Renderer::drawWorld(const World& world)
{
    for (auto& [id, chunk] : world.getChunks())
    {
        // Upload mesh if not already cached
        uploadChunkMesh(chunk.get());
        
        int x = static_cast<int>(id >> 32);
        int z = static_cast<int>(id & 0xFFFFFFFF);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x * CHUNK_SIZE, 0, z * CHUNK_SIZE));
        drawChunk(*chunk, model);
    }
}