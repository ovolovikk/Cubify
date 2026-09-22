#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "Graphics/IPipeline.hpp"

namespace Cubify::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12Pipeline : public IPipeline
    {
    public:
        static constexpr UINT ROOT_PARAM_VIEW_PROJ = 0; // b0
        static constexpr UINT ROOT_PARAM_MODEL = 1;     // b1
        static constexpr UINT ROOT_PARAM_QUADS = 2;     // t0
        static constexpr UINT ROOT_PARAM_TEXTURES = 3;  // t1
        static constexpr UINT MATRIX_CONSTANT_COUNT = 16;

        static ComPtr<ID3D12RootSignature> CreateRootSignature(ID3D12Device2* device);

        DX12Pipeline(ID3D12Device2* device, ID3D12RootSignature* rootSignature, const PipelineDesc& desc);

        ID3D12PipelineState* pipelineState() const;

    private:
        ComPtr<ID3D12PipelineState> m_pipelineState;
    };
}
