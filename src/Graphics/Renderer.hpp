#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GL/glew.h>
#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

#include "Graphics/Mesh.hpp"
#include "Graphics/Quad.hpp"

class Shader;
class TextureArray;

class Renderer
{
public:
    Renderer(int width, int height);
    ~Renderer();
    
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void resize(int width, int height);
    void onResize(int width, int height);

    void beginFrame();

    void setViewProjection(const glm::mat4& view, const glm::mat4& projection);

    void uploadMesh(Mesh& mesh, const std::vector<Quad>& quads);
    void draw(const Mesh& mesh, const glm::mat4& model);

private:
    std::unique_ptr<Shader> shader;
    std::unique_ptr<TextureArray> texture_array;
    GLuint sampler;
    GLuint vao;
    

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
};

#endif // RENDERER_HPP
