#include "DX12Screenshot.hpp"

#include <wrl/client.h>

#include "d3dx12.h"
#include "Graphics/DirectX12Backend/DX12Common.hpp"
#include "stb_image_write.h"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    bool SaveBackBufferToPng(ID3D12Device2* device, ID3D12CommandQueue* queue,
        ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer, const char* filePath)
    {
        D3D12_RESOURCE_DESC backBufferDesc = backBuffer->GetDesc();

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        UINT rowCount = 0;
        UINT64 rowSizeInBytes = 0;
        UINT64 totalBytes = 0;
        device->GetCopyableFootprints(&backBufferDesc, 0, 1, 0,
            &footprint, &rowCount, &rowSizeInBytes, &totalBytes);

        // READBACK is the mirror image of UPLOAD: GPU writes, CPU reads
        CD3DX12_HEAP_PROPERTIES readbackHeap(D3D12_HEAP_TYPE_READBACK);
        CD3DX12_RESOURCE_DESC readbackDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);

        ComPtr<ID3D12Resource> readback;
        HR_FALLBACK(device->CreateCommittedResource(
            &readbackHeap,
            D3D12_HEAP_FLAG_NONE,
            &readbackDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&readback)),
            false, "[DX12Screenshot] Failed to create readback buffer");

        CD3DX12_RESOURCE_BARRIER toCopySource = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
        commandList->ResourceBarrier(1, &toCopySource);

        CD3DX12_TEXTURE_COPY_LOCATION destination(readback.Get(), footprint);
        CD3DX12_TEXTURE_COPY_LOCATION source(backBuffer, 0);
        commandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);

        CD3DX12_RESOURCE_BARRIER backToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->ResourceBarrier(1, &backToRenderTarget);

        commandList->Close();
        ID3D12CommandList* commandLists[] = { commandList };
        queue->ExecuteCommandLists(_countof(commandLists), commandLists);

        // Own fence, so the renderer's frame pacing stays untouched
        ComPtr<ID3D12Fence> fence;
        HR_FALLBACK(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), false,
            "[DX12Screenshot] Failed to create fence");

        HANDLE copied = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        queue->Signal(fence.Get(), 1);
        fence->SetEventOnCompletion(1, copied);
        WaitForSingleObject(copied, INFINITE);
        CloseHandle(copied);

        void* mapped = nullptr;
        CD3DX12_RANGE readRange(0, static_cast<SIZE_T>(totalBytes));
        HR_FALLBACK(readback->Map(0, &readRange, &mapped), false,
            "[DX12Screenshot] Failed to map readback buffer");

        const size_t rowBytes = static_cast<size_t>(rowSizeInBytes);
        std::vector<uint8_t> pixels(rowBytes * rowCount);
        for (UINT row = 0; row < rowCount; ++row)
        {
            memcpy(pixels.data() + rowBytes * row,
                static_cast<const uint8_t*>(mapped) + static_cast<size_t>(footprint.Footprint.RowPitch) * row,
                rowBytes);
        }

        CD3DX12_RANGE writtenRange(0, 0);
        readback->Unmap(0, &writtenRange);

        // D3D and PNG both start at the top left, so unlike the OpenGL path
        // there is no flip here.
        stbi_flip_vertically_on_write(0);
        return stbi_write_png(filePath,
            static_cast<int>(backBufferDesc.Width),
            static_cast<int>(backBufferDesc.Height),
            4, pixels.data(), static_cast<int>(rowBytes)) != 0;
    }
}
