#pragma once

#include <d3d12.h>

namespace Cubify::DX12
{
    class DX12Device;

    // Test mode only. Copies the back buffer into a PNG and blocks until the GPU is done.
    bool SaveBackBufferToPng(DX12Device& device, ID3D12GraphicsCommandList* commandList,
        ID3D12Resource* backBuffer, const char* filePath);
}
