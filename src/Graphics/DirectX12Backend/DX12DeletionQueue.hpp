#pragma once

#include "Graphics/IGPUResourceQueue.hpp"

#include "PrecompilerHeader.hpp"

// Renderer clears the deletion queue at the end of each frame
namespace Cubify::DX12
{
	class DX12DeletionQueue : public IGpuResourceQueue
	{
	public:
		void enqueueDeletion(MeshId mesh) override;
		std::vector<MeshId> takeMeshes();

	private:
		std::vector<MeshId> m_toDelete;
	};
}
