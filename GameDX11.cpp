//
// GameDX11.cpp
//

#include "pch.h"
#include "GameDX11.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

GameDX11::GameDX11() noexcept :
	BaseGame()
{
    m_deviceResources = std::make_unique<DX::DeviceResourcesDX11>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

// InitializeDX11 the Direct3D resources required to run.
void GameDX11::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

// Executes the basic game loop.
void GameDX11::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}



// Updates the world.
void GameDX11::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
   
    //std::cout << timer.GetFramesPerSecond() << std::endl;
}

// Draws the scene.
void GameDX11::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // TODO: Add your rendering code here.


}

// Helper method to clear the back buffers.
void GameDX11::Clear()
{
    //// Clear the views.
    //m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    //m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    //// Set the viewport.
    //D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight), 0.f, 1.f };
    //m_d3dContext->RSSetViewports(1, &viewport);
}

void GameDX11::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

void GameDX11::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void GameDX11::OnActivated()
{
    // TODO: Game is becoming active window.
}

void GameDX11::OnDeactivated()
{
	// TODO: Game is becoming background window.
}

void GameDX11::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void GameDX11::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void GameDX11::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void GameDX11::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void GameDX11::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

void GameDX11::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void GameDX11::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
