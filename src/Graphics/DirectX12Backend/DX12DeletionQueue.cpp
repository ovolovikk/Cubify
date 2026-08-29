#include "DX12DeletionQueue.hpp"

#include "Graphics/MeshId.hpp"

namespace Cubify::DX12
{
	void DX12DeletionQueue::enqueueDeletion(MeshId mesh)
	{
		m_toDelete.push_back(mesh);
	}

	std::vector<MeshId> DX12DeletionQueue::takeMeshes()
	{
		std::vector<MeshId> taken = std::move(m_toDelete);
		return taken;
	}
}