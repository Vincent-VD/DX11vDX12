//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef __MINGW32__
#include <unknwn.h>
#endif

#include <wrl/client.h>
#include <wingdi.h>

//DirectX11 headers
#include <d3d11_3.h>
#include <dxgi1_6.h>

#ifdef USING_DIRECTX_HEADERS
#include <directx/dxgiformat.h>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>
#else
#include <d3d12.h>

#include "d3dx12.h"
#endif

#include <dxgi1_4.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include "dxgidebug.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <string>
#include <wrl.h>
#include <shellapi.h>

#include "DirectXTK11/Inc/SimpleMath.h"

#include "DirectXTK12/Inc/SimpleMath.h"

#include "Shared.h"
#include "BaseGame.h"
//#include "BufferHelpers.h"
//#include "CommonStates.h"
//#include "DDSTextureLoader.h"
#include "DescriptorHeap.h"
//#include "DirectXHelpers.h"
#include "EffectPipelineStateDescription.h"
//#include "Effects.h"
//#include "GamePad.h"
//#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
//#include "Keyboard.h"
//#include "Model.h"
//#include "Mouse.h"
//#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "RenderTargetState.h"
#include "ResourceUploadBatch.h"
//#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
//#include "WICTextureLoader.h"


enum class RenderType
{
	DirectX11 = 0,
    DirectX12 = 1,
};

namespace
{
    //--------------------------------------------------------------------------------------
    // Constants
    //--------------------------------------------------------------------------------------
    const uint32_t  c_maxInstances = 200000;
    const uint32_t  c_startInstanceCount = 200;
    const uint32_t  c_minInstanceCount = 1000;
    const float     c_boxBounds = 60.0f;
    size_t          c_cubeIndexCount = 36;
    const float     c_velocityMultiplier = 500.0f;
    const float     c_rotationGain = 0.004f;

    //--------------------------------------------------------------------------------------
    // Cube vertex definition
    //--------------------------------------------------------------------------------------
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 norm;
    };
}


namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }
}

#ifdef __MINGW32__
namespace Microsoft
{
    namespace WRL
    {
        namespace Wrappers
        {
            class Event
            {
            public:
                Event() noexcept : m_handle{} {}
                explicit Event(HANDLE h) noexcept : m_handle{ h } {}
                ~Event() { if (m_handle) { ::CloseHandle(m_handle); m_handle = nullptr; } }

                void Attach(HANDLE h) noexcept
                {
                    if (h != m_handle)
                    {
                        if (m_handle) ::CloseHandle(m_handle);
                        m_handle = h;
                    }
                }

                bool IsValid() const { return m_handle != nullptr; }
                HANDLE Get() const { return m_handle; }

            private:
                HANDLE m_handle;
            };
        }
    }
}
#else
#include <wrl/event.h>
#endif

#ifdef __MINGW32__
constexpr UINT PIX_COLOR_DEFAULT = 0;

inline void PIXBeginEvent(UINT64, PCWSTR) {}

template<typename T>
inline void PIXBeginEvent(T*, UINT64, PCWSTR) {}

inline void PIXEndEvent() {}

template<typename T>
inline void PIXEndEvent(T*) {}
#else
// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
// then add the NuGet package WinPixEventRuntime to the project.
#include <pix3.h>
#endif

