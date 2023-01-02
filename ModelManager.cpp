#include "pch.h"
#include "ModelManager.h"

#include "StepTimer.h"

ModelManager* ModelManager::m_pInstance = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new ModelManager();
	}
	return m_pInstance;
}

void ModelManager::CleanUp()
{
	delete m_pInstance;
}

ModelManager::~ModelManager()
{
	delete m_pModelDX11;
	delete m_pModelDX12;
	m_pInstance = nullptr;
}

ModelManager::ModelManager()
{
	m_ViewDX11 = DirectX11::SimpleMath::Matrix::CreateLookAt(DirectX11::SimpleMath::Vector3(2.f, 2.f, 2.f),
		DirectX11::SimpleMath::Vector3::Zero, DirectX11::SimpleMath::Vector3::UnitY);
	m_ViewDX12 = DirectX12::SimpleMath::Matrix::CreateLookAt(DirectX12::SimpleMath::Vector3(2.f, 2.f, 2.f),
		DirectX12::SimpleMath::Vector3::Zero, DirectX12::SimpleMath::Vector3::UnitY);
}


void ModelManager::update(DX::StepTimer const& timer)
{
	const auto elapsedSec = static_cast<float>(timer.GetElapsedSeconds());
	const float keyboardSpeed = GetAsyncKeyState(VK_LSHIFT) ? m_KeyboardMoveSensitivity * m_KeyboardMoveMultiplier : m_KeyboardMoveSensitivity;
	auto left{ static_cast<float>(std::abs(GetAsyncKeyState('A'))) };
	auto right{ static_cast<float>(std::abs(GetAsyncKeyState('D'))) };
	auto up{ static_cast<float>(std::abs(GetAsyncKeyState('W'))) };
	auto down{ static_cast<float>(std::abs(GetAsyncKeyState('S'))) };
	left = std::max(0.f, std::min(left, 1.f));
	right = std::max(0.f, std::min(right, 1.f));
	up = std::max(0.f, std::min(up, 1.f));
	down = std::max(0.f, std::min(down, 1.f));
	m_CurrPos.x += (right - left) * keyboardSpeed * elapsedSec;
	m_CurrPos.z += (down - up) * keyboardSpeed * elapsedSec;
	switch(g_RenderType) //global variable containing currently used graphics API
	{
	case RenderType::DirectX11:
		m_WorldDX11.Translation({ m_CurrPos.x, m_CurrPos.y, m_CurrPos.z });
		break;
	case RenderType::DirectX12:
		m_WorldDX12.Translation({ m_CurrPos.x, m_CurrPos.y, m_CurrPos.z });
		break;
	default:
		break;
	}

	//Mouse Input
	if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0)
	{
		m_CurrPos.y -= dragPos.y * m_MouseMoveSensitivity * elapsedSec;
		switch (g_RenderType)
		{
		case RenderType::DirectX11:
			m_WorldDX11.Translation({ m_CurrPos.x, m_CurrPos.y, m_CurrPos.z });
			m_ViewDX11 = DirectX11::SimpleMath::Matrix::CreateRotationY(dragPos.y * m_MouseRotationSensitivity);
			break;
		case RenderType::DirectX12:
			m_WorldDX12.Translation({ m_CurrPos.x, m_CurrPos.y, m_CurrPos.z });
			m_ViewDX12 = DirectX11::SimpleMath::Matrix::CreateRotationY(dragPos.y * m_MouseRotationSensitivity);
			break;
		default:
			break;
		}
	}
	else if ((GetKeyState(VK_RBUTTON) & 0x8000) != 0)
	{
		switch (g_RenderType)
		{
		case RenderType::DirectX11:
			m_ViewDX11 = DirectX11::SimpleMath::Matrix::CreateRotationX(dragPos.y * m_MouseRotationSensitivity * elapsedSec);
			m_ViewDX11 = DirectX11::SimpleMath::Matrix::CreateRotationY(dragPos.x * m_MouseRotationSensitivity * elapsedSec);
			break;
		case RenderType::DirectX12:
			m_ViewDX12 = DirectX11::SimpleMath::Matrix::CreateRotationX(dragPos.y * m_MouseRotationSensitivity * elapsedSec);
			m_ViewDX12 = DirectX11::SimpleMath::Matrix::CreateRotationY(dragPos.x * m_MouseRotationSensitivity * elapsedSec);
			break;
		default:
			break;
		}
		
	}
	else if ((GetKeyState(VK_RBUTTON) & 0x8000) != 0 && (GetKeyState(VK_LBUTTON) & 0x8000) != 0)
	{
		switch (g_RenderType)
		{
		case RenderType::DirectX11:
			m_WorldDX11.Translation({ 0, dragPos.y * m_MouseMoveSensitivity * elapsedSec, 0 });
			break;
		case RenderType::DirectX12:
			m_WorldDX12.Translation({ 0, dragPos.y * m_MouseMoveSensitivity * elapsedSec, 0 });
			break;
		default:
			break;
		}
	}
	dragPos = { 0,0 };
}

void ModelManager::SetDrag(int x, int y)
{
	int width{}, height{};
	g_game->GetDefaultSize(width, height);
	dragPos.x = static_cast<float>(x) / static_cast<float>(width) - 0.5f;
	dragPos.y = static_cast<float>(y) / static_cast<float>(height) - 0.5f;
}