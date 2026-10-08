#include "DX12Texture.hpp"

#include "d3dx12.h"
#include "Graphics/DirectX12Backend/DX12Common.hpp"

namespace Cubify::DX12
{
    DX12Texture::DX12Texture(DX12Device& device, const TextureDesc& desc)
    {
        if (desc.layers.empty() || desc.width <= 0 || desc.height <= 0)
        {
            LOGE("[DX12Texture] %s has no layers to upload", desc.debugName.c_str());
            return;
        }

        CreateSrvHeap(device.device());
        CreateTexture(device.device(), desc);
        if (!m_srvHeap || !m_texture)
        {
            return;
        }

        Upload(device, desc);
        CreateShaderResourceView(device.device(), desc);

        SetDebugName(m_texture.Get(), Widen(desc.debugName).c_str());
        LOGI("[DX12Texture] %s created: %zu layers of %dx%d",
            desc.debugName.c_str(), desc.layers.size(), desc.width, desc.height);
    }

    bool DX12Texture::isValid() const
    {
        return m_texture && m_srvHeap;
    }

    ID3D12DescriptorHeap* DX12Texture::srvHeap() const
    {
        return m_srvHeap.Get();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12Texture::srv() const
    {
        return m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    }

    void DX12Texture::CreateSrvHeap(ID3D12Device2* device)
    {
        // Shader visible, one slot for this texture
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
        };

        HR_CHECK(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)),
            "[DX12Texture] Failed to create SRV descriptor heap");
        SetDebugName(m_srvHeap.Get(), L"SRV Heap");
    }

    void DX12Texture::CreateTexture(ID3D12Device2* device, const TextureDesc& desc)
    {
        CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            static_cast<UINT64>(desc.width),
            static_cast<UINT>(desc.height),
            static_cast<UINT16>(desc.layers.size()),
            1);

        HR_CHECK(device->CreateCommittedResource(
            &defaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_texture)),
            "[DX12Texture] Failed to create texture");
    }

    void DX12Texture::Upload(DX12Device& device, const TextureDesc& desc)
    {
        const UINT layerCount = static_cast<UINT>(desc.layers.size());

        // Rows have to be padded to 256 bytes and each slice aligned to 512, so
        // the staging buffer is bigger than the raw pixels. UpdateSubresources
        // works all of that out from the footprints.
        const UINT64 uploadSize = GetRequiredIntermediateSize(m_texture.Get(), 0, layerCount);

        CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

        ComPtr<ID3D12Resource> uploadBuffer;
        HR_CHECK(device.device()->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer)),
            "[DX12Texture] Failed to create upload buffer");

        // Every array slice is its own subresource
        std::vector<D3D12_SUBRESOURCE_DATA> subresources(layerCount);
        for (UINT i = 0; i < layerCount; ++i)
        {
            subresources[i].pData = desc.layers[i];
            subresources[i].RowPitch = static_cast<LONG_PTR>(desc.width) * 4;
            subresources[i].SlicePitch = subresources[i].RowPitch * desc.height;
        }

        // Own command list, so the upload doesn't disturb the renderer's frame
        ComPtr<ID3D12CommandAllocator> allocator;
        HR_CHECK(device.device()->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)),
            "[DX12Texture] Failed to create upload command allocator");

        ComPtr<ID3D12GraphicsCommandList> commandList;
        HR_CHECK(device.device()->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)),
            "[DX12Texture] Failed to create upload command list");

        UpdateSubresources(
            commandList.Get(),
            m_texture.Get(),
            uploadBuffer.Get(),
            0, 0, layerCount,
            subresources.data());

        CD3DX12_RESOURCE_BARRIER toReadable = CD3DX12_RESOURCE_BARRIER::Transition(
            m_texture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &toReadable);

        commandList->Close();
        ID3D12CommandList* lists[] = { commandList.Get() };
        device.commandQueue()->ExecuteCommandLists(_countof(lists), lists);

        // Blocking wait, textures are only uploaded while starting up
        device.flush();
    }

    void DX12Texture::CreateShaderResourceView(ID3D12Device2* device, const TextureDesc& desc)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2DArray = {
                .MostDetailedMip = 0,
                .MipLevels = 1,
                .FirstArraySlice = 0,
                .ArraySize = static_cast<UINT>(desc.layers.size())
            }
        };

        device->CreateShaderResourceView(
            m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
    }
}
