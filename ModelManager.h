#pragma once
#include "pch.h"
#include "DirectXTK11/Inc/SimpleMath.h"
#include "DirectXTK12/Inc/SimpleMath.h"

namespace DX
{
	class StepTimer;
}

class ModelManager
{
public:

	static ModelManager* GetInstance();
	~ModelManager();
	static void CleanUp();
	
	ModelManager(const ModelManager& other) = delete;
	ModelManager(ModelManager&& other) noexcept = delete;
	ModelManager& operator=(const ModelManager& other) = delete;
	ModelManager& operator=(ModelManager&& other) noexcept = delete;

	void update(DX::StepTimer const& timer);

	void SetModelDX11(DirectX11::Model* pModel) { m_pModelDX11 = pModel; };
	void SetModelDX12(DirectX12::Model* pModel) { m_pModelDX12 = pModel; };
	 
	DirectX11::Model* GetModelDX11() const { return m_pModelDX11; };
	DirectX12::Model* GetModelDX12() const { return m_pModelDX12; };

	DirectX11::SimpleMath::Matrix GetWorldDX11() { return m_WorldDX11; };
	DirectX12::SimpleMath::Matrix GetWorldDX12() { return m_WorldDX12; };

	DirectX12::SimpleMath::Matrix GetViewDX12() { return m_ViewDX12; };
	DirectX11::SimpleMath::Matrix GetViewDX11() { return m_ViewDX11; };

	void SetDrag(int x, int y);

private:
	ModelManager();
	static ModelManager* m_pInstance;

	DirectX11::Model* m_pModelDX11{};
	DirectX12::Model* m_pModelDX12{};

	DirectX11::SimpleMath::Matrix m_WorldDX11{};
	DirectX12::SimpleMath::Matrix m_WorldDX12{};

	DirectX11::SimpleMath::Matrix m_ViewDX11{};
	DirectX12::SimpleMath::Matrix m_ViewDX12{};

	XMFLOAT3 m_CurrPos{};

	const float m_KeyboardMoveSensitivity{ 11.f };
	const float m_KeyboardMoveMultiplier{ 2.f };
	const float m_MouseRotationSensitivity{ .1f };
	const float m_MouseMoveSensitivity{ 10.f };

	POINTF dragPos{};
};

