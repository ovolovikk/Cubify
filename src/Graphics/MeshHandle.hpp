#pragma once

#include <memory>
#include <utility>

#include "Graphics/IGpuResourceQueue.hpp"
#include "Graphics/MeshId.hpp"

class MeshHandle
{
public:
    MeshHandle() = default;

    MeshHandle(MeshId id, std::weak_ptr<IGpuResourceQueue> queue)
        : m_id(id), m_queue(std::move(queue))
    {
    }

    ~MeshHandle() { reset(); }

    MeshHandle(const MeshHandle&) = delete;
    MeshHandle& operator=(const MeshHandle&) = delete;

    MeshHandle(MeshHandle&& other) noexcept
        : m_id(other.m_id), m_queue(std::move(other.m_queue))
    {
        other.m_id = InvalidMeshId;
    }

    MeshHandle& operator=(MeshHandle&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            m_id = other.m_id;
            m_queue = std::move(other.m_queue);
            other.m_id = InvalidMeshId;
        }
        return *this;
    }

    void reset()
    {
        if (m_id != InvalidMeshId)
        {
            if (auto queue = m_queue.lock())
            {
                queue->enqueueDeletion(m_id);
            }
            m_id = InvalidMeshId;
        }
        m_queue.reset();
    }

private:
    MeshId m_id = InvalidMeshId;
    std::weak_ptr<IGpuResourceQueue> m_queue;

public:
    MeshId id() const { return m_id; }
    bool isValid() const { return m_id != InvalidMeshId; }
};
