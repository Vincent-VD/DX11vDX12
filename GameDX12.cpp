#include "pch.h"
#include "GameDX12.h"

//
// GameDX12.cpp
//

extern void ExitGameDX12() noexcept;

using namespace DirectX12;
using namespace DirectX12::SimpleMath;

using Microsoft::WRL::ComPtr;

GameDX12::GameDX12() noexcept :
	BaseGame(),
	m_aspectRatio()
{
    m_deviceResources = std::make_unique<DX::DeviceResourcesDX12>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

GameDX12::~GameDX12()
{
    // Ensure that the GPU is no longer referencing resources that are about to be destroyed.
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

// Initialize the Direct3D resources required to run.
void GameDX12::Initialize(HWND window, int width, int height)
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

// Executes the basic GameDX12 loop.
void GameDX12::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// Updates the world.
void GameDX12::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    PIXEndEvent();
}

// Draws the scene.
void GameDX12::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // TODO: Add your rendering code here.
    m_effect->Apply(commandList);

    m_batch->Begin(commandList);

    VertexPositionColor v1(Vector3(400.f, 150.f, 0.f), Colors::Red);
    VertexPositionColor v2(Vector3(600.f, 450.f, 0.f), Colors::Green);
    VertexPositionColor v3(Vector3(200.f, 450.f, 0.f), Colors::Blue);

    m_batch->DrawTriangle(v1, v2, v3);

    m_batch->End();

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

    PIXEndEvent();
}

//void GameDX12::PopulateCommandList()
//{
//
//    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_swapBufferCount, m_rtvDescriptorSize);
//    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
//
//    // Record commands.
//    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
//    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
//    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
//    m_commandList->DrawInstanced(3, 1, 0, 0);
//
//    DX::ThrowIfFailed(m_commandList->Close());
//
//}


// Helper method to prepare the command list for rendering and clear the back buffers.
void GameDX12::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto const rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto const dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto const viewport = m_deviceResources->GetScreenViewport();
    auto const scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}

void GameDX12::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory = std::make_unique<DirectX12::GraphicsMemory>(device);

    // TODO: Initialize device dependent objects here (independent of window size).
    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(device);

    RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
        m_deviceResources->GetDepthBufferFormat());

    EffectPipelineStateDescription pd(
        &VertexType::InputLayout,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        CommonStates::CullNone,
        rtState);

    m_effect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, pd);

}

void GameDX12::CreateWindowSizeDependentResources()
{const auto size = m_deviceResources->GetOutputSize();

   const Matrix proj = Matrix::CreateScale(2.f / static_cast<float>(size.right),
        -2.f / static_cast<float>(size.bottom), 1.f)
        * Matrix::CreateTranslation(-1.f, 1.f, 0.f);
    m_effect->SetProjection(proj);
}

void GameDX12::OnActivated()
{
    // TODO: Game is becoming active window.
}

void GameDX12::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void GameDX12::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void GameDX12::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void GameDX12::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void GameDX12::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void GameDX12::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

void GameDX12::OnDeviceLost()
{
    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory.reset();
    m_effect.reset();
    m_batch.reset();
}

void GameDX12::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}