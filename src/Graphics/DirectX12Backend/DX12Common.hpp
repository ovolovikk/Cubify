#pragma once

#include <d3d12.h>
#include <string>

#include "Logging/Log.hpp"

#define HR_CHECK(expr, ...)                                 \
    {                                                       \
        HRESULT hrResult = (expr);                          \
        if (FAILED(hrResult))                               \
        {                                                   \
            LOGE(__VA_ARGS__);                              \
            LOGE("[DX12] hr = 0x%08X", hrResult);           \
            return;                                         \
        }                                                   \
    }

#define HR_FALLBACK(expr, returnVal, ...)                   \
    {                                                       \
        HRESULT hrResult = (expr);                          \
        if (FAILED(hrResult))                               \
        {                                                   \
            LOGE(__VA_ARGS__);                              \
            LOGE("[DX12] hr = 0x%08X", hrResult);           \
            return returnVal;                               \
        }                                                   \
    }

namespace Cubify::DX12
{
    inline constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
    inline constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;

    // ASCII only, for debug names and shader file names
    inline std::wstring Widen(const std::string& text)
    {
        return std::wstring(text.begin(), text.end());
    }

    inline void SetDebugName(ID3D12Object* object, const wchar_t* name)
    {
        if (object)
        {
            object->SetName(name);
        }
    }
}
