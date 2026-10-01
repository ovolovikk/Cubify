#pragma once

#include <d3d12.h>

namespace Cubify::DX12
{
    // Test mode only. Copies the back buffer into a PNG and blocks until the GPU is done.
    // Leaves commandList closed, the caller resets it.
    bool SaveBackBufferToPng(ID3D12Device2* device, ID3D12CommandQueue* queue,
        ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer, const char* filePath);
}
