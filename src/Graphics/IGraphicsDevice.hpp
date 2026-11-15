#pragma once

#include <memory>

#include "Graphics/IRendererBackend.hpp"
#include "Graphics/Resources/ISwapChain.hpp"

class IGraphicsDevice
{
public:
    virtual ~IGraphicsDevice() = default;

    virtual std::unique_ptr<IRendererBackend> createRenderer(const SwapChainDesc& desc) = 0;
};
