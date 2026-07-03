#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Graphics/IRendererBackend.hpp"
#include "Graphics/Mesh.hpp"
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

    void uploadMesh(Mesh& mesh, const std::vector<Quad>& quads) override;
    void draw(const Mesh& mesh, const glm::mat4& model) override;

private:
    std::unique_ptr<Shader> shader;
    std::unique_ptr<TextureArray> texture_array;
    GLuint sampler;
    GLuint vao;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    WorldSettings world_settings;
};

#endif // RENDERER_HPP
