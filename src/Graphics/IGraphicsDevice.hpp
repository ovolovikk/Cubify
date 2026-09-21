#pragma once

#include "Graphics/GraphicsApi.hpp"

// Adapter, logical device and the main queue. Everything else is built on top of it.
class IGraphicsDevice
{
public:
    virtual ~IGraphicsDevice() = default;

    virtual GraphicsApi api() const = 0;
};
