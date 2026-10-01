#pragma once

#include <memory>

#include "Graphics/GraphicsApi.hpp"
#include "Graphics/IPipeline.hpp"
#include "Graphics/ISwapChain.hpp"
#include "Graphics/ITexture.hpp"

// Adapter, logical device and the main queue
class IGraphicsDevice
{
public:
    virtual ~IGraphicsDevice() = default;

    virtual GraphicsApi api() const = 0;

    // Return nullptr if creation fails, e.g. a shader that doesn't compile
    virtual std::unique_ptr<IPipeline> createPipeline(const PipelineDesc& desc) = 0;
    virtual std::unique_ptr<ISwapChain> createSwapChain(const SwapChainDesc& desc) = 0;
    virtual std::unique_ptr<ITexture> createTexture(const TextureDesc& desc) = 0;
};
