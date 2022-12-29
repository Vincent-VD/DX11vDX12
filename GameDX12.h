#pragma once
#include "pch.h"
#include "BaseGame.h"
#include "StepTimer.h"
#include "DeviceResourcesDX12.h"

class GameDX12 : public BaseGame, DX::IDeviceNotify
{
public:
	GameDX12() noexcept;
	virtual ~GameDX12() override;

	GameDX12(GameDX12&&) = delete;
	GameDX12& operator= (GameDX12&&) = delete;
	GameDX12(GameDX12 const&) = delete;
	GameDX12& operator= (GameDX12 const&) = delete;

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

	float m_aspectRatio;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	std::unique_ptr<DX::DeviceResourcesDX12> m_deviceResources;

	std::unique_ptr<DirectX12::GraphicsMemory> m_graphicsMemory;

	using VertexType = DirectX12::VertexPositionColor;

	std::unique_ptr<DirectX12::CommonStates> m_states;
	std::unique_ptr<DirectX12::EffectFactory> m_fxFactory;
	std::unique_ptr<DirectX12::EffectTextureFactory> m_modelResources;
	std::unique_ptr<DirectX12::Model> m_model;
	DirectX12::Model::EffectCollection m_modelNormal;

	DirectX12::SimpleMath::Matrix m_world;
	DirectX12::SimpleMath::Matrix m_view;
	DirectX12::SimpleMath::Matrix m_proj;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

	virtual void Clear() override;

	virtual void CreateDeviceDependentResources() override;
	virtual void CreateWindowSizeDependentResources() override;
};
