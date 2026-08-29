#include "Graphics/OpenGLBackend/GLRenderer.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/OpenGLBackend/Shader.hpp"
#include "Graphics/TextureArray.hpp"
#include "Core/Logging/Log.hpp"
#include "stb_image_write.h"

GLRenderer::GLRenderer(int width, int height, bool is_void_mode)
    : resources(std::make_shared<GpuResourceManager>()),
      sampler(0), vao(0), view_matrix(1.0f), projection_matrix(1.0f),
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
        "assets/textures/sectorr_grass_top.png",
        "assets/textures/sectorr_grass_side.png",
        "assets/textures/sectorr_dirt.png",
        "assets/textures/sectorr_stone.png",
        "assets/textures/sectorr_sand.png",
        "assets/textures/sectorr_water.png",
        "assets/textures/utopia_sand.png",
        "assets/textures/utopia_silt.png",
        "assets/textures/utopia_water.png"
    };
    if (is_void_mode) layers.push_back("assets/textures/void.png");
    texture_array = std::make_unique<TextureArray>(layers);

    // required dummy vao for core profile
    glGenVertexArrays(1, &vao);
}

GLRenderer::~GLRenderer()
{
    glDeleteVertexArrays(1, &vao);
}

void GLRenderer::resize(int width, int height)
{
    glViewport(0, 0, width, height);
}

void GLRenderer::onResize(int width, int height)
{
    resize(width, height);
}

void GLRenderer::setWorldSettings(const WorldSettings& settings)
{
    world_settings = settings;
    glClearColor(world_settings.skyColor.r, world_settings.skyColor.g, world_settings.skyColor.b, 1.0f);
}

void GLRenderer::beginFrame()
{
    resources->processPendingDeletions();

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

void GLRenderer::endFrame()
{
    // TODO: Figure out what should be there
}

void GLRenderer::beginTransparentPass()
{
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
}

void GLRenderer::endTransparentPass()
{
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

void GLRenderer::setViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
    view_matrix = view;
    projection_matrix = projection;
}

void GLRenderer::uploadMesh(MeshHandle& mesh, const std::vector<Quad>& quads)
{
    if (!mesh.isValid()) {
        if (quads.empty()) {
            return;
        }
        mesh = MeshHandle(resources->create(), resources);
    }

    resources->upload(mesh.id(), quads);
}

void GLRenderer::draw(MeshId mesh, const glm::mat4& model)
{
    const GpuResourceManager::GpuMesh* gpu = resources->find(mesh);
    if (gpu == nullptr || gpu->quadCount == 0 || gpu->handle == 0) {
        return;
    }

    if (shader) {
        shader->setMat4("model", model);
    }

    glBindVertexArray(vao);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpu->handle);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLuint>(gpu->quadCount));
}

bool GLRenderer::captureBackbuffer(const char* filePath)
{
    // WILL REMOVE THAT
    return false;
}
