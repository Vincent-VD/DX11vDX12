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

	uint32_t GetCurrentInstanceCount() const { return m_UsedInstanceCount; };

private:
	// Device resources.
	std::unique_ptr<DX::DeviceResourcesDX12>        m_DeviceResources;

	// DirectXTK objects.
	std::unique_ptr<DirectX12::GraphicsMemory>    m_GraphicsMemory;
	std::unique_ptr<DirectX12::DescriptorHeap>    m_ResourceDescriptors;
	std::unique_ptr<DirectX12::SpriteBatch>       m_SpriteBatch;
	std::unique_ptr<DirectX12::SpriteFont>        m_SmallFont;

	enum Descriptors
	{
		TextFont,
		ControllerFont,
		Count
	};

	//--------------------------------------------------------------------------------------
	// Sample Objects.
	//--------------------------------------------------------------------------------------

	// Direct3D 12 pipeline objects.
	Microsoft::WRL::ComPtr<ID3D12RootSignature>  m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>  m_PipelineState;

	// Direct3D 12 resources.
	Microsoft::WRL::ComPtr<ID3D12Resource>       m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>       m_VertexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW                     m_VertexBufferView[3];
	Microsoft::WRL::ComPtr<ID3D12Resource>       m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>       m_IndexBufferUpload;
	D3D12_INDEX_BUFFER_VIEW                      m_IndexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource>       m_BoxColors;
	Microsoft::WRL::ComPtr<ID3D12Resource>       m_BoxColorsUpload;

	Microsoft::WRL::ComPtr<ID3D12Resource>       m_InstanceData;
	uint8_t*                                     m_MappedInstanceData;
	D3D12_GPU_VIRTUAL_ADDRESS                    m_InstanceDataGpuAddr;

	// A synchronization fence and an event. These members will be used
	// to synchronize the CPU with the GPU so that there will be no
	// contention for the instance data. 
	Microsoft::WRL::ComPtr<ID3D12Fence>          m_Fence;
	Microsoft::WRL::Wrappers::Event              m_FenceEvent;

	struct aligned_deleter { void operator()(void* p) { _aligned_free(p); } };

	std::unique_ptr<Instance[]>                             m_CPUInstanceData;
	std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_RotationQuaternions;
	std::unique_ptr<DirectX::XMVECTOR[], aligned_deleter>   m_Velocities;
	uint32_t                                                m_UsedInstanceCount;

	DirectX::XMFLOAT4X4                         m_Proj;
	DirectX::XMFLOAT4X4                         m_Clip;
	Lights                                      m_Lights;
	float                                       m_Pitch;
	float                                       m_Yaw;

	std::default_random_engine                  m_RandomEngine;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

	virtual void Clear() override;

	virtual void CreateDeviceDependentResources() override;
	virtual void CreateWindowSizeDependentResources() override;

	void ResetSimulation();
	float FloatRand(float lowerBound = -1.0f, float upperBound = 1.0f);
};
