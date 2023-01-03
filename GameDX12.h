#pragma once
#include "pch.h"

#include <random>

#include "BaseGame.h"
#include "StepTimer.h"
#include "DeviceResourcesDX12.h"

class GameDX12 : public BaseGame
{
public:
	GameDX12() noexcept;
	virtual ~GameDX12() override = default;

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
    // Device resources.
    std::unique_ptr<DX::DeviceResourcesDX12>        m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                               m_timer;

    // Input devices.
    std::unique_ptr<DirectX12::GamePad>           m_gamePad;
    std::unique_ptr<DirectX12::Keyboard>          m_keyboard;
    std::unique_ptr<DirectX12::Mouse>             m_mouse;

    DirectX12::GamePad::ButtonStateTracker        m_gamePadButtons;
    DirectX12::Keyboard::KeyboardStateTracker     m_keyboardButtons;
    bool                                        m_gamepadPresent;

    // DirectXTK objects.
    std::unique_ptr<DirectX12::GraphicsMemory>    m_graphicsMemory;
    std::unique_ptr<DirectX12::DescriptorHeap>    m_resourceDescriptors;
    std::unique_ptr<DirectX12::SpriteBatch>       m_batch;
    std::unique_ptr<DirectX12::SpriteFont>        m_smallFont;
    std::unique_ptr<DirectX12::SpriteFont>        m_ctrlFont;

    enum Descriptors
    {
        TextFont,
        ControllerFont,
        Count
    };

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

    // Direct3D 12 pipeline objects.
    Microsoft::WRL::ComPtr<ID3D12RootSignature>  m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>  m_pipelineState;

    // Direct3D 12 resources.
    Microsoft::WRL::ComPtr<ID3D12Resource>       m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                     m_vertexBufferView[3];
    Microsoft::WRL::ComPtr<ID3D12Resource>       m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW                      m_indexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource>       m_boxColors;

    Microsoft::WRL::ComPtr<ID3D12Resource>       m_instanceData;
    uint8_t* m_mappedInstanceData;
    D3D12_GPU_VIRTUAL_ADDRESS                    m_instanceDataGpuAddr;

    // A synchronization fence and an event. These members will be used
    // to synchronize the CPU with the GPU so that there will be no
    // contention for the instance data. 
    Microsoft::WRL::ComPtr<ID3D12Fence>          m_fence;
    Microsoft::WRL::Wrappers::Event              m_fenceEvent;

    struct aligned_deleter { void operator()(void* p) { _aligned_free(p); } };

    std::unique_ptr<Instance[]>                             m_CPUInstanceData;
    std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_rotationQuaternions;
    std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_velocities;
    uint32_t                                                m_usedInstanceCount;

    DirectX::XMFLOAT4X4                         m_proj;
    DirectX::XMFLOAT4X4                         m_clip;
    Lights                                      m_lights;
    float                                       m_pitch;
    float                                       m_yaw;

    std::default_random_engine                  m_randomEngine;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

	virtual void Clear() override;

	virtual void CreateDeviceDependentResources() override;
	virtual void CreateWindowSizeDependentResources() override;

    void ResetSimulation();
    float FloatRand(float lowerBound = -1.0f, float upperBound = 1.0f);
};
