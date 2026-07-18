#pragma once

#include <string>

enum class GraphicsApi
{
    OpenGL,
    DirectX12
};

inline GraphicsApi graphicsApiFromString(const std::string& name)
{
    return name == "directx12" ? GraphicsApi::DirectX12 : GraphicsApi::OpenGL;
}
