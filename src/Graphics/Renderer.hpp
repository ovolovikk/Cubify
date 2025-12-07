#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GL/glew.h>
#include <glm/mat4x4.hpp>

#include <string>
#include <memory>
#include <unordered_map>

#include "Graphics/Mesh.hpp"
#include "Graphics/Quad.hpp"
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
