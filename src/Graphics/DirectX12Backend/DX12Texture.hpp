#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "Graphics/DirectX12Backend/DX12Device.hpp"
#include "Graphics/Resources/ITexture.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12Texture : public ITexture
    {
    public:
        DX12Texture(DX12Device& device, const TextureDesc& desc);

        bool isValid() const;
        ID3D12DescriptorHeap* srvHeap() const;
        D3D12_GPU_DESCRIPTOR_HANDLE srv() const;

    private:
        void CreateSrvHeap(ID3D12Device2* device);
        void CreateTexture(ID3D12Device2* device, const TextureDesc& desc);
        void Upload(DX12Device& device, const TextureDesc& desc);
        void CreateShaderResourceView(ID3D12Device2* device, const TextureDesc& desc);

        ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        ComPtr<ID3D12Resource> m_texture;
    };
}
