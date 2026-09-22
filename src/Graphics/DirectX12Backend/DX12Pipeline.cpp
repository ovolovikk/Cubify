#include "DX12Pipeline.hpp"

#include "d3dx12.h"
#include <dxcapi.h>
#include "Graphics/DirectX12Backend/DX12Common.hpp"

namespace Cubify::DX12
{
    // Shader and debug names are plain ASCII
    static std::wstring Widen(const std::string& text)
    {
        return std::wstring(text.begin(), text.end());
    }

    static ComPtr<IDxcBlob> CompileShader(const ShaderDesc& shader, const wchar_t* target)
    {
        static ComPtr<IDxcUtils> utils;
        static ComPtr<IDxcCompiler3> compiler;
        static ComPtr<IDxcIncludeHandler> includeHandler;

        if(!utils)
        {
            DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
            DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
            utils->CreateDefaultIncludeHandler(&includeHandler);
        }

        const std::wstring path = L"shaders/dx12/" + Widen(shader.name) + L".hlsl";
        const std::wstring entry = Widen(shader.entry);

        ComPtr<IDxcBlobEncoding> sourceBlob;
        HR_FALLBACK(utils->LoadFile(path.c_str(), nullptr, &sourceBlob), nullptr,
            "[DX12Pipeline] Failed to load shader file: %ls", path.c_str());

        std::vector<LPCWSTR> arguments = {
            path.c_str(),
            L"-E", entry.c_str(),
            L"-T", target,
            // Lets the stage files pull in common.hlsli by bare name
            L"-I", L"shaders/dx12",
        };

#if defined (_DEBUG)
        arguments.push_back(L"-Zi");
        arguments.push_back(L"-Qembed_debug");
        arguments.push_back(L"-Od");
#else
        arguments.push_back(L"-Qstrip_reflect");
        arguments.push_back(L"-O3");
#endif
        DxcBuffer sourceBuffer{
            .Ptr = sourceBlob->GetBufferPointer(),
            .Size = sourceBlob->GetBufferSize(),
            .Encoding = DXC_CP_ACP
        };

        ComPtr<IDxcResult> result;
        HR_FALLBACK(compiler->Compile(
            &sourceBuffer,
            arguments.data(),
            static_cast<UINT32>(arguments.size()),
            includeHandler.Get(),
            IID_PPV_ARGS(&result)
        ), nullptr, "[DX12Pipeline] Internal DXC compiler error.");

        ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

        if (errors != nullptr && errors->GetStringLength() > 0)
        {
            LOGE("Shader compile log (%ls):\n%s\n", entry.c_str(), errors->GetStringPointer());
        }

        HRESULT status;
        result->GetStatus(&status);
        if (FAILED(status))
        {
            return nullptr;
        }

        ComPtr<IDxcBlob> compiled;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiled), nullptr);

        return compiled;
    }

    ComPtr<ID3D12RootSignature> DX12Pipeline::CreateRootSignature(ID3D12Device2* device)
    {
        // 35/64 DWORD space used, a root SRV costs 2 and a table costs 1
        CD3DX12_ROOT_PARAMETER1 params[4]{};
        params[ROOT_PARAM_VIEW_PROJ].InitAsConstants(MATRIX_CONSTANT_COUNT, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        params[ROOT_PARAM_MODEL].InitAsConstants(MATRIX_CONSTANT_COUNT, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        // Root descriptor takes a GPU address directly, so no descriptor heap is
        // needed for the geometry buffer
        params[ROOT_PARAM_QUADS].InitAsShaderResourceView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

        CD3DX12_DESCRIPTOR_RANGE1 textureRange;
        textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
        params[ROOT_PARAM_TEXTURES].InitAsDescriptorTable(
            1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_STATIC_SAMPLER_DESC sampler(
            0,
            D3D12_FILTER_MIN_MAG_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
        desc.Init_1_1(_countof(params), params, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

        ComPtr<ID3DBlob> serialized;
        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DX12SerializeVersionedRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_1, &serialized, &errors);
        if (FAILED(hr) && errors)
        {
            LOGE("[DX12Pipeline] Root signature error: %s", static_cast<const char*>(errors->GetBufferPointer()));
        }
        HR_FALLBACK(hr, nullptr, "[DX12Pipeline] Failed to initialize Root Signature");

        ComPtr<ID3D12RootSignature> rootSignature;
        HR_FALLBACK(device->CreateRootSignature(
            0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)),
            nullptr, "[DX12Pipeline] Failed to create root signature");
        SetDebugName(rootSignature.Get(), L"Main Root Signature");
        LOGI("[DX12Pipeline] Root signature created successfully");
        return rootSignature;
    }

    DX12Pipeline::DX12Pipeline(ID3D12Device2* device, ID3D12RootSignature* rootSignature, const PipelineDesc& desc)
    {
        ComPtr<IDxcBlob> vs = CompileShader(desc.vertexShader, L"vs_6_0");
        ComPtr<IDxcBlob> ps = CompileShader(desc.fragmentShader, L"ps_6_0");
        if (!vs || !ps)
        {
            LOGE("[DX12Pipeline] Skipping PSO creation for %s: shader compilation failed", desc.debugName.c_str());
            return;
        }

        CD3DX12_RASTERIZER_DESC rasterizer(D3D12_DEFAULT);
        rasterizer.FrontCounterClockwise = TRUE;
        rasterizer.CullMode = desc.cull == CullMode::Back ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;

        CD3DX12_DEPTH_STENCIL_DESC1 depthStencil(D3D12_DEFAULT);
        depthStencil.DepthEnable = TRUE;
        depthStencil.DepthWriteMask = desc.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

        CD3DX12_BLEND_DESC blend(D3D12_DEFAULT);
        if (desc.blend == BlendMode::AlphaBlend)
        {
            // Luna chapter 9, 10 to experiment
            blend.RenderTarget[0].BlendEnable = TRUE;
            blend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            blend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        }

        D3D12_RT_FORMAT_ARRAY rtvFormats{
            .RTFormats = { BACK_BUFFER_FORMAT },
            .NumRenderTargets = 1
        };

        CD3DX12_PIPELINE_STATE_STREAM1 stream;
        stream.pRootSignature = rootSignature;
        stream.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
        stream.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
        stream.BlendState = blend;
        stream.RasterizerState = rasterizer;
        stream.DepthStencilState = depthStencil;
        stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        stream.RTVFormats = rtvFormats;
        stream.DSVFormat = DEPTH_FORMAT;
        stream.SampleDesc = DXGI_SAMPLE_DESC{ .Count = 1, .Quality = 0 };

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{
            .SizeInBytes = sizeof(stream),
            .pPipelineStateSubobjectStream = &stream
        };

        HR_CHECK(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)),
            "[DX12Pipeline] Failed to create pipeline state: %s", desc.debugName.c_str());
        SetDebugName(m_pipelineState.Get(), Widen(desc.debugName).c_str());
        LOGI("[DX12Pipeline] Pipeline state created: %s", desc.debugName.c_str());
    }

    ID3D12PipelineState* DX12Pipeline::pipelineState() const
    {
        return m_pipelineState.Get();
    }
}
