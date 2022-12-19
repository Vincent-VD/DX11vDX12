#pragma once
#include "StepTimer.h"
//Header taken from minigin
#include "DXSampleHelper.h"
#include "DeviceResources.h"

class BaseGame : public DX::IDeviceNotify
{
public:
	BaseGame() noexcept;
	virtual ~BaseGame() = 0 {};

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

protected:

	// Application state
	HWND                                                m_window;
	int                                                 m_outputWidth;
	int                                                 m_outputHeight;

	// Game state
	DX::StepTimer                                       m_timer;

	std::wstring GetAssetFullPath(LPCWSTR assetName);

private:

	virtual void Update(DX::StepTimer const& timer) = 0;
	virtual void Render() = 0;

	virtual void Clear() = 0;

	virtual void CreateDeviceDependentResources() = 0;
	virtual void CreateWindowSizeDependentResources() = 0;

	// Root assets path.
	std::wstring m_assetsPath;
};

