#pragma once

#include <d3d12.h>

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
    inline void SetDebugName(ID3D12Object* object, const wchar_t* name)
    {
        if (object)
        {
            object->SetName(name);
        }
    }
}
