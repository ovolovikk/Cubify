#pragma once

#include <memory>

#include "Graphics/IRendererBackend.hpp"

class IGraphicsDevice
{
public:
    virtual ~IGraphicsDevice() = default;

    virtual std::unique_ptr<IRendererBackend> createRenderer(void* windowHandle, int width, int height) = 0;
};
