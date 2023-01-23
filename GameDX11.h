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

    uint32_t GetCurrentInstanceCount() const { return m_UsedInstanceCount; };

private:
    // Device resources.
    std::unique_ptr<DX::DeviceResourcesDX11>      m_DeviceResources;

    // DirectXTK objects.
    std::unique_ptr<DirectX11::SpriteBatch>       m_SpriteBatch;
    std::unique_ptr<DirectX11::SpriteFont>        m_SmallFont;

    //--------------------------------------------------------------------------------------
    // Sample Objects.
    //--------------------------------------------------------------------------------------

    Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_InputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_IndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_InstanceData;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_BoxColors;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_VertexConstants;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_PixelConstants;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_PixelShader;

    struct aligned_deleter { void operator()(void* p) { _aligned_free(p); } };

    std::unique_ptr<Instance[]>                             m_CPUInstanceData;
    std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_RotationQuaternions;
    std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_Velocities;
    uint32_t                                                m_UsedInstanceCount;

    DirectX::XMFLOAT4X4                         m_Proj;
    Lights                                      m_Lights;
    float                                       m_Pitch;
    float                                       m_Yaw;

    std::default_random_engine                  m_RandomEngine;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

	virtual void Clear() override;

	virtual void CreateDeviceDependentResources() override;
	virtual void CreateWindowSizeDependentResources() override;

    void ReplaceBufferContents(ID3D11Buffer* buffer, size_t bufferSize, const void* data);
    void ResetSimulation();

    float FloatRand(float lowerBound = -1.0f, float upperBound = 1.0f);
};
