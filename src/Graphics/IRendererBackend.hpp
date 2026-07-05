#pragma once

#include <glm/mat4x4.hpp>

#include "Graphics/Mesh.hpp"
#include "Graphics/Quad.hpp"
#include "World/WorldSettings.hpp"

class IRendererBackend
{
public:
    virtual ~IRendererBackend() = default;

    virtual void resize(int width, int height) = 0;
    virtual void onResize(int width, int height) = 0;
    
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void beginTransparentPass() = 0;
    virtual void endTransparentPass() = 0;

    virtual void setViewProjection(const glm::mat4& view, const glm::mat4& projection) = 0;
    virtual void setWorldSettings(const WorldSettings& settings) = 0;

    virtual void uploadMesh(Mesh& mesh, const std::vector<Quad>& quads) = 0;
    virtual void draw(const Mesh& mesh, const glm::mat4& model) = 0;

private:

};
