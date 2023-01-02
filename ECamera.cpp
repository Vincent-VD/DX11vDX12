#include "pch.h"
#include "ECamera.h"
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

namespace Elite
{

	Camera* Camera::m_Instance = nullptr;

	Camera* Camera::GetInstance()
	{
		if (m_Instance == nullptr)
		{
			m_Instance = new Camera();
		}
		return m_Instance;
	}

	void Camera::CleanUp()
	{
		delete m_Instance;
	}

	Camera::~Camera()
	{
		m_Instance = nullptr;
	}

	Camera::Camera(const XMFLOAT3& position, const float width, const float height) :
		m_Fov(XM_PIDIV4),
		m_Position{ position },
		m_WorldPosition{ 0, 0, 0 },
		m_WorldScale{ 1, 1, 1 },
		m_WorldRotation{ 0, 0, 0, 1 },
		m_Forward{ 0, 0, 1 },
		m_Right{ 1, 0, 0 },
		m_Up{ 0, 1, 0 },
		m_FarPlane{ 2500.0f },
		m_NearPlane{ 0.1f },
		m_AspectRatio{ width / height }
	{
		//Calculate initial matrices based on given parameters (position & target)
		//CalculateLookAt();
	}

	void Camera::Update(float elapsedSec)
	{

		//Capture Input (absolute) Rotation & (relative) Movement
		//*************
		//Keyboard Input
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(0);
		float keyboardSpeed = pKeyboardState[SDL_SCANCODE_LSHIFT] ? m_KeyboardMoveSensitivity * m_KeyboardMoveMultiplier : m_KeyboardMoveSensitivity;
		m_RelativeTranslation.x = (pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) * keyboardSpeed * elapsedSec;
		m_RelativeTranslation.y = 0;
		m_RelativeTranslation.z = (pKeyboardState[SDL_SCANCODE_S] - pKeyboardState[SDL_SCANCODE_W]) * keyboardSpeed * elapsedSec;

		//Mouse Input
		int x, y = 0;
		uint32_t mouseState = SDL_GetRelativeMouseState(&x, &y);
		if (mouseState == SDL_BUTTON_LMASK)
		{
			m_RelativeTranslation.z += y * m_MouseMoveSensitivity * elapsedSec;
			m_AbsoluteRotation.y += x * m_MouseRotationSensitivity;
		}
		else if (mouseState == SDL_BUTTON_RMASK)
		{
			m_AbsoluteRotation.x -= y * m_MouseRotationSensitivity;
			m_AbsoluteRotation.y -= x * m_MouseRotationSensitivity;
		}
		else if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
		{
			m_RelativeTranslation.y += y * m_MouseMoveSensitivity * elapsedSec;
		}

		//Update LookAt (view2world & world2view matrices)
		//*************
		CalculateLookAt();
	}

	void Camera::CalculateLookAt()
	{

		//Calculate World Matrix
	//**********************
		auto rot = XMLoadFloat4(&m_AbsoluteRotation);
		auto world = XMMatrixScaling(1.f, 1.f, 1.f) *
			XMMatrixRotationQuaternion(rot) *
			XMMatrixTranslation(m_RelativeTranslation.x, m_RelativeTranslation.y, m_RelativeTranslation.z);

		XMStoreFloat4x4(&m_World, world);

		//Get World Transform
		XMVECTOR pos, scale;
		if (XMMatrixDecompose(&scale, &rot, &pos, world))
		{
			XMStoreFloat3(&m_WorldPosition, pos);
			XMStoreFloat3(&m_WorldScale, scale);
			XMStoreFloat4(&m_WorldRotation, rot);
		};

		const auto rotMat = XMMatrixRotationQuaternion(rot);
		const auto forward = XMVector3TransformCoord(XMVectorSet(0, 0, 1, 0), rotMat);
		const auto right = XMVector3TransformCoord(XMVectorSet(1, 0, 0, 0), rotMat);
		const auto up = XMVector3Cross(forward, right);

		XMStoreFloat3(&m_Forward, forward);
		XMStoreFloat3(&m_Right, right);
		XMStoreFloat3(&m_Up, up);

		XMMATRIX projection = XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);

		const XMVECTOR worldPosition = XMLoadFloat3(&m_WorldPosition);
		const XMVECTOR lookAt = XMLoadFloat3(&m_Forward);
		const XMVECTOR upVec = XMLoadFloat3(&m_Up);

		const XMMATRIX view = XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
		const XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
		const XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

		XMStoreFloat4x4(&m_Projection, projection);
		XMStoreFloat4x4(&m_View, view);
		XMStoreFloat4x4(&m_ViewInverse, viewInv);
		XMStoreFloat4x4(&m_ViewProjection, view * projection);
		XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
	}

	void Camera::SetAspectRatio(uint32_t width, uint32_t height)
	{
		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
	}

}