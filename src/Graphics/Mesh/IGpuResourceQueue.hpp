#pragma once

#include "Graphics/Mesh/MeshId.hpp"

class IGpuResourceQueue
{
public:
    virtual ~IGpuResourceQueue() = default;

    virtual void enqueueDeletion(MeshId id) = 0;
};
