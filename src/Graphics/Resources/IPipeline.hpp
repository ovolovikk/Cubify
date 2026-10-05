#pragma once

#include <string>

enum class BlendMode
{
    Opaque,
    AlphaBlend
};

enum class CullMode
{
    None,
    Back
};

struct ShaderDesc
{
    std::string name; // file name without folder or extension, e.g. "vertex_shader"
    std::string entry;
};

struct PipelineDesc
{
    std::string debugName;
    ShaderDesc vertexShader;
    ShaderDesc fragmentShader;
    BlendMode blend = BlendMode::Opaque;
    CullMode cull = CullMode::Back;
    bool depthWrite = true;
};

class IPipeline
{
public:
    virtual ~IPipeline() = default;
};
