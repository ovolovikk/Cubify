#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <pix3.h>
#include <wrl/client.h>

#include "Logging/Log.hpp"
#include "PrecompilerHeader.hpp"

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

    // Should be called only after user's DX12 objects are cleared
    inline void LogLiveObjects()
    {
#if defined(_DEBUG)
        using Microsoft::WRL::ComPtr;

        ComPtr<IDXGIDebug1> debug;
        ComPtr<IDXGIInfoQueue> infoQueue;
        if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))) ||
            FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&infoQueue))))
        {
            return;
        }

        infoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
        debug->ReportLiveObjects(DXGI_DEBUG_ALL,
            static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));

        const UINT64 messageCount = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
        for (UINT64 i = 0; i < messageCount; ++i)
        {
            SIZE_T size = 0;
            infoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &size);

            std::vector<char> bytes(size);
            auto* message = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.data());
            if (SUCCEEDED(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, message, &size)))
            {
                LOGW("[DX12] %s", message->pDescription);
            }
        }
#endif
    }
}
