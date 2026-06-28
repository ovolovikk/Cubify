#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Graphics/Mesh.hpp"
#include "Graphics/Quad.hpp"
#include "World/WorldSettings.hpp"

class Shader;
class TextureArray;

class Renderer
{
public:
    Renderer(int width, int height, bool is_void_mode);
    ~Renderer();
    
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void resize(int width, int height);
    void onResize(int width, int height);

    void beginFrame();
    void beginTransparentPass();
    void endTransparentPass();

    void setViewProjection(const glm::mat4& view, const glm::mat4& projection);
    void setWorldSettings(const WorldSettings& settings);

    void uploadMesh(Mesh& mesh, const std::vector<Quad>& quads);
    void draw(const Mesh& mesh, const glm::mat4& model);

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
