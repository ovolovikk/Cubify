#pragma once

#include <memory>

#include "Graphics/GraphicsApi.hpp"
#include "Graphics/IPipeline.hpp"

// Adapter, logical device and the main queue
class IGraphicsDevice
{
public:
    virtual ~IGraphicsDevice() = default;

    virtual GraphicsApi api() const = 0;

    // nullptr if the pipeline can't be built, e.g. a shader doesn't compile
    virtual std::unique_ptr<IPipeline> createPipeline(const PipelineDesc& desc) = 0;
};
