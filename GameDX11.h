//
// GameDX11.h
//

#pragma once
#include "pch.h"

#include <random>

#include "BaseGame.h"
#include "StepTimer.h"
#include "DeviceResources.h"


class GameDX11 : public BaseGame
{
public:

	GameDX11() noexcept;
	virtual ~GameDX11() override = default;

	GameDX11(GameDX11&&) = delete;
	GameDX11& operator= (GameDX11&&) = delete;
	GameDX11(GameDX11 const&) = delete;
	GameDX11& operator= (GameDX11 const&) = delete;

	// Initialization and management
	virtual void Initialize(HWND window, int width, int height) override;

	// Basic game loop
	virtual void Tick() override;

	// IDeviceNotify
	void OnDeviceLost() override;
	void OnDeviceRestored() override;

	// Messages
	virtual void OnActivated() override;
	virtual void OnDeactivated() override;
	virtual void OnSuspending() override;
	virtual void OnResuming() override;
	virtual void OnWindowMoved() override;
	virtual void OnDisplayChange() override;
	virtual void OnWindowSizeChanged(int width, int height) override;

    uint32_t GetCurrentInstanceCount() const { return m_usedInstanceCount; };

private:
    // Device resources.
    std::unique_ptr<DX::DeviceResourcesDX11>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX11::GamePad>       m_gamePad;
    //std::unique_ptr<DirectX11::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX11::Mouse>         m_mouse;

    DirectX11::GamePad::ButtonStateTracker    m_gamePadButtons;
    DirectX11::Keyboard::KeyboardStateTracker m_keyboardButtons;
    bool                                    m_gamepadPresent;

    // DirectXTK objects.
    std::unique_ptr<DirectX11::SpriteBatch>       m_batch;
    std::unique_ptr<DirectX11::SpriteFont>        m_smallFont;
    std::unique_ptr<DirectX11::SpriteFont>        m_ctrlFont;

    //--------------------------------------------------------------------------------------
    // Sample Objects.
    //--------------------------------------------------------------------------------------

    // Instance vertex definition
    struct Instance
    {
        DirectX::XMFLOAT4 quaternion;
        DirectX::XMFLOAT4 positionAndScale;
    };

    // Light data structure (maps to constant buffer in pixel shader)
    struct Lights
    {
        DirectX::XMFLOAT4 directional;
        DirectX::XMFLOAT4 pointPositions[c_pointLightCount];
        DirectX::XMFLOAT4 pointColors[c_pointLightCount];
    };

    Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_instanceData;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_boxColors;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_vertexConstants;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_pixelConstants;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_pixelShader;

    struct aligned_deleter { void operator()(void* p) { _aligned_free(p); } };

    std::unique_ptr<Instance[]>                             m_CPUInstanceData;
    std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_rotationQuaternions;
    std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_velocities;
    uint32_t                                                m_usedInstanceCount;

    DirectX::XMFLOAT4X4                         m_proj;
    Lights                                      m_lights;
    float                                       m_pitch;
    float                                       m_yaw;

    std::default_random_engine                  m_randomEngine;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

	virtual void Clear() override;

	virtual void CreateDeviceDependentResources() override;
	virtual void CreateWindowSizeDependentResources() override;

    void ReplaceBufferContents(ID3D11Buffer* buffer, size_t bufferSize, const void* data);
    void ResetSimulation();

    float FloatRand(float lowerBound = -1.0f, float upperBound = 1.0f);
};
