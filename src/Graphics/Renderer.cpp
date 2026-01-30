#include "Graphics/Renderer.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/Shader.hpp"
#include "Graphics/TextureArray.hpp"
#include "Core/Logging/Log.hpp"

Renderer::Renderer(int width, int height)
    : sampler(0), vao(0), view_matrix(1.0f), projection_matrix(1.0f),
      world_settings(WorldSettings::getForWorldType(WorldType::MINECRAFT))
{
    glViewport(0, 0, width, height);

    glClearColor(world_settings.skyColor.r, world_settings.skyColor.g, world_settings.skyColor.b, 1.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE); // MSAA if avaivable
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // subsystem initialization
    LOGI("[Renderer] Initializing Renderer");
    shader = std::make_unique<Shader>();
    if (!shader->load("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl")) {
        LOGE("[Renderer] Failed to load shaders");
        return;
    }

    std::vector<string> layers = {
        "assets/textures/grass_top.png",
        "assets/textures/grass_side.png",
        "assets/textures/dirt.png",
        "assets/textures/stone.png",
        "assets/textures/sand.png",
        "assets/textures/wooden_plank.png",
        "assets/textures/water.png",
        "assets/textures/bedrock.png",
        "assets/textures/ice.png",
        "assets/textures/edmunds_grass_top.png",
        "assets/textures/edmunds_grass_side.png",
        "assets/textures/edmunds_dirt.png",
        "assets/textures/edmunds_stone.png",
        "assets/textures/edmunds_sand.png",
        "assets/textures/edmunds_water.png"
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

void Renderer::onResize(int width, int height)
{
    resize(width, height);
}

void Renderer::setWorldSettings(const WorldSettings& settings)
{
    world_settings = settings;
    glClearColor(world_settings.skyColor.r, world_settings.skyColor.g, world_settings.skyColor.b, 1.0f);
}

void Renderer::beginFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (shader) {
        shader->use();
        shader->setMat4("projection", projection_matrix);
        shader->setMat4("view", view_matrix);
        shader->setInt("u_Textures", 0);
        shader->setFloat("fogDensity", world_settings.fogDensity);
        shader->setFloat("fogPower", world_settings.fogPower);
        shader->setVec3("u_SkyColor", world_settings.skyColor);
        shader->setFloat("u_Time", static_cast<float>(glfwGetTime()));
    }
    
    if(texture_array) {
        texture_array->bind(0);
    }
    
    glBindVertexArray(vao);
}

void Renderer::beginTransparentPass()
{
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
}

void Renderer::endTransparentPass()
{
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
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