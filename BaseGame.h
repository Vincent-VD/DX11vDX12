#pragma once
#include "StepTimer.h"
//Header taken from minigin
#include "DeviceResources.h"

class BaseGame : public DX::IDeviceNotify
{
public:
	BaseGame() noexcept;
	virtual ~BaseGame() = default;

	BaseGame(const BaseGame& other) = delete;
	BaseGame(BaseGame&& other) noexcept = delete;
	BaseGame& operator=(const BaseGame& other) = delete;
	BaseGame& operator=(BaseGame&& other) noexcept = delete;

	// Initialization and management
	virtual void Initialize(HWND window, int width, int height) = 0;

	// Basic game loop
	virtual void Tick() = 0;

	// Messages
	virtual void OnActivated();
	virtual void OnDeactivated();
	virtual void OnSuspending();
	virtual void OnResuming();
	virtual void OnWindowMoved();
	virtual void OnDisplayChange();
	virtual void OnWindowSizeChanged(int width, int height);

	// Properties
	void GetDefaultSize(int& width, int& height) const noexcept;
	void GetCurrentWindowSize(int& width, int& height) const noexcept;

protected:

	// Application state
	HWND                                                m_Window;
	int                                                 m_OutputWidth;
	int                                                 m_OutputHeight;

	// Game state
	DX::StepTimer                                       m_Timer;

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


private:

	virtual void Update(DX::StepTimer const& timer) = 0;
	virtual void Render() = 0;

	virtual void Clear() = 0;

	virtual void CreateDeviceDependentResources() = 0;
	virtual void CreateWindowSizeDependentResources() = 0;
};

