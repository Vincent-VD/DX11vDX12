//
// GameDX11.h
//

#pragma once
#include "pch.h"
#include "BaseGame.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "DirectXTK11/Inc/SimpleMath.h"

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

	using VertexType = DirectX11::VertexPositionColor;

	DirectX11::SimpleMath::Matrix m_world;
	DirectX11::SimpleMath::Matrix m_view;
	DirectX11::SimpleMath::Matrix m_proj;

	std::unique_ptr<DirectX11::CommonStates> m_states;
	std::unique_ptr<DirectX11::IEffectFactory> m_fxFactory;
	std::unique_ptr<DirectX11::Model> m_model;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

	virtual void Clear() override;

	virtual void CreateDeviceDependentResources() override;
	virtual void CreateWindowSizeDependentResources() override;
};
