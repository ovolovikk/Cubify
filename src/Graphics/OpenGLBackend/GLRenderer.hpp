#pragma once

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Graphics/IRendererBackend.hpp"
#include "Graphics/MeshHandle.hpp"
#include "Graphics/MeshId.hpp"
#include "Graphics/OpenGLBackend/GpuResourceManager.hpp"
#include "Graphics/Quad.hpp"
#include "World/WorldSettings.hpp"

class Shader;
class TextureArray;

class GLRenderer : public IRendererBackend
{
public:
    GLRenderer(int width, int height, bool is_void_mode);
    ~GLRenderer();
    
    GLRenderer(const GLRenderer&) = delete;
    GLRenderer& operator=(const GLRenderer&) = delete;

    void resize(int width, int height) override;
    void onResize(int width, int height) override;

    void beginFrame() override;
    void endFrame() override;
    void beginTransparentPass() override;
    void endTransparentPass() override;

    void setViewProjection(const glm::mat4& view, const glm::mat4& projection) override;
    void setWorldSettings(const WorldSettings& settings) override;

    void uploadMesh(MeshHandle& mesh, const std::vector<Quad>& quads) override;
    void draw(MeshId mesh, const glm::mat4& model) override;

    bool captureBackbuffer(const char* filePath) override;

private:
    std::shared_ptr<GpuResourceManager> resources;

    std::unique_ptr<Shader> shader;
    std::unique_ptr<TextureArray> texture_array;
    GLuint sampler;
    GLuint vao;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    WorldSettings world_settings;
};
