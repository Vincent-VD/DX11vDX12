//
// GameDX11.h
//

#pragma once

#include "BaseGame.h"
#include "StepTimer.h"
#include "DeviceResources.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class GameDX11 : public BaseGame, DX::IDeviceNotify
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

private:

	std::unique_ptr<DX::DeviceResourcesDX11> m_deviceResources;

	using VertexType = DirectX::VertexPositionColor;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

	virtual void Clear() override;

	virtual void CreateDeviceDependentResources() override;
	virtual void CreateWindowSizeDependentResources() override;
};
