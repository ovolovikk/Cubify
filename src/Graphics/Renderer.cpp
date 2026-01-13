#include "Graphics/Renderer.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/Shader.hpp"
#include "Graphics/TextureArray.hpp"
#include "Core/Logging/Log.hpp"

Renderer::Renderer(int width, int height)
    : sampler(0), vao(0), view_matrix(1.0f), projection_matrix(1.0f)
{
    glViewport(0, 0, width, height);

    glClearColor(0.28f, 0.66f, 1.f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE); // MSAA if avaivable
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // subsystem initialization
    shader = std::make_unique<Shader>();
    if (!shader->load("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl")) {
        LOGE("Failed to load shaders");
        return;
    }
    
    LOGI("Renderer Initialized");

    std::vector<string> layers = {
        "textures/grass_top.png",
        "textures/grass_side.png",
        "textures/dirt.png",
        "textures/stone.png",
        "textures/sand.png",
        "textures/wooden_plank.png",
        "textures/water.png"
    };
    texture_array = std::make_unique<TextureArray>(layers);

    // required dummy vao for core profile
    glGenVertexArrays(1, &vao);
}

Renderer::~Renderer()
{
    glDeleteVertexArrays(1, &vao);
}

void Renderer::resize(int width, int height)
{
    glViewport(0, 0, width, height);
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
    
    glBindVertexArray(vao);
}

void Renderer::setViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
    view_matrix = view;
    projection_matrix = projection;
}

void Renderer::uploadMesh(Mesh& mesh, const std::vector<Quad>& quads)
{
    if (quads.empty()) {
        mesh.quadCount = 0;
        return;
    }

    if (mesh.SSBO == 0) {
        glGenBuffers(1, &mesh.SSBO);
    }

    mesh.quadCount = quads.size();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh.SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, quads.size() * sizeof(Quad), quads.data(), GL_STATIC_DRAW);
}

void Renderer::draw(const Mesh& mesh, const glm::mat4& model)
{
    if (mesh.quadCount == 0 || mesh.SSBO == 0) {
        return;
    }
    
    if (shader) {
        shader->setMat4("model", model);
    }
    
    glBindVertexArray(vao);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh.SSBO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLuint>(mesh.quadCount));
}