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

void ModelManager::update(DX::StepTimer const& timer)
{
	//BYTE arr[256];
	//if(!GetKeyboardState(arr))
	//{
	//	std::cout << "Could not get keyboard state\n";
	//	return;
	//}
	const auto elapsedSec = static_cast<float>(timer.GetElapsedSeconds());
	//const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
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
	m_CurrPos.y += (down - up) * keyboardSpeed * elapsedSec;
	switch(g_RenderType) //global variable containing currently used graphics API
	{
	case RenderType::DirectX11:
		m_WorldDX11.Translation({ m_CurrPos.x, 0.f, m_CurrPos.y });
		break;
	case RenderType::DirectX12:
		m_WorldDX12.Translation({ m_CurrPos.x, 0.f, m_CurrPos.y });
		break;
	default:
		break;
	}

	//Mouse Input
	POINT mousePos;
	GetCursorPos(&mousePos);
	int x = mousePos.x;
	int y = mousePos.y;
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
	{
		m_CurrPos.y += y * m_MouseMoveSensitivity * elapsedSec;
		switch (g_RenderType)
		{
		case RenderType::DirectX11:
			m_WorldDX11.Translation({ 0, m_CurrPos.y, 0 });
			//m_WorldDX11.CreateRotationY(y * m_MouseRotationSensitivity);
			break;
		case RenderType::DirectX12:
			m_WorldDX12.Translation({ 0, m_CurrPos.y, 0 });
			//m_WorldDX12.CreateRotationY(y * m_MouseRotationSensitivity);
			break;
		default:
			break;
		}
	}
	else if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0)
	{
		switch (g_RenderType)
		{
		case RenderType::DirectX11:
			//m_WorldDX11.CreateRotationX(y * m_MouseRotationSensitivity * elapsedSec);
			//m_WorldDX11.CreateRotationY(x * m_MouseRotationSensitivity * elapsedSec);
			break;
		case RenderType::DirectX12:
			//m_WorldDX12.CreateRotationX(y * m_MouseRotationSensitivity * elapsedSec);
			//m_WorldDX12.CreateRotationY(x * m_MouseRotationSensitivity * elapsedSec);
			break;
		default:
			break;
		}
		
	}
	else if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 && (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0)
	{
		switch (g_RenderType)
		{
		case RenderType::DirectX11:
			m_WorldDX11.Translation({ 0, y * m_MouseMoveSensitivity * elapsedSec, 0 });
			break;
		case RenderType::DirectX12:
			m_WorldDX12.Translation({ 0, y * m_MouseMoveSensitivity * elapsedSec, 0 });
			break;
		default:
			break;
		}
	}
	
}
