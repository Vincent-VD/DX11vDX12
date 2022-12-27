//
// GameDX11.cpp
//

#include "pch.h"
#include "GameDX11.h"

#include "DirectXTK11/Inc/SimpleMath.h"

extern void ExitGame() noexcept;

using namespace DirectX11;
using namespace DirectX11::SimpleMath;

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
    const float elapsedTime = static_cast<float>(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;
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

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullNone());

    m_effect->Apply(context);

    context->IASetInputLayout(m_inputLayout.Get());

    m_batch->Begin();

    VertexPositionColor v1(Vector3(400.f, 150.f, 0.f), Colors::Red);
    VertexPositionColor v2(Vector3(600.f, 450.f, 0.f), Colors::Green);
    VertexPositionColor v3(Vector3(200.f, 450.f, 0.f), Colors::Blue);

    m_batch->DrawTriangle(v1, v2, v3);

    m_batch->End();

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void GameDX11::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, DirectX::Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void GameDX11::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    m_states = std::make_unique<DirectX11::CommonStates>(device);

    m_effect = std::make_unique<DirectX11::BasicEffect>(device);
    m_effect->SetVertexColorEnabled(true);

    DX::ThrowIfFailed(
        DirectX11::CreateInputLayoutFromEffect<VertexType>(device, m_effect.get(),
            m_inputLayout.ReleaseAndGetAddressOf())
    );

    auto context = m_deviceResources->GetD3DDeviceContext();
    m_batch = std::make_unique<DirectX11::PrimitiveBatch<VertexType>>(context);
}

void GameDX11::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    const auto size = m_deviceResources->GetOutputSize();

    const Matrix proj = Matrix::CreateScale(2.f / static_cast<float>(size.right),
        -2.f / static_cast<float>(size.bottom), 1.f)
        * Matrix::CreateTranslation(-1.f, 1.f, 0.f);
    m_effect->SetProjection(proj);
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
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
}

void GameDX11::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
