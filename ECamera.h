/*=============================================================================*/
// Copyright 2021 Elite Engine 2.0
// Authors: Thomas Goussaert
/*=============================================================================*/
// ECamera.h: Base Camera Implementation with movement
/*=============================================================================*/

#pragma once
#include "pch.h"

namespace Elite
{
	class Camera
	{
	public:

		static Camera* GetInstance();
		~Camera();
		static void CleanUp();

		//~Camera() = default;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;

		void Update(float elapsedSec);

		void SetAspectRatio(uint32_t width, uint32_t height);

		const XMFLOAT4X4& GetView() const { return m_View; }
		const XMFLOAT4X4& GetProjection() const { return m_Projection; }
		const XMFLOAT4X4& GetViewProjection() const { return m_ViewProjection; }
		const XMFLOAT4X4& GetViewInverse() const { return m_ViewInverse; }
		const XMFLOAT4X4& GetViewProjectionInverse() const { return m_ViewProjectionInverse; }


		XMFLOAT3 GetPosition()
		{
			return m_WorldPosition;
		}

		const float GetFov() const { return m_Fov; }

	private:
		Camera(const XMFLOAT3& position = { 0.f, 0.f, -10.f }, const float width = 800.f, const float height = 600.f);
		static Camera* m_Instance;

		void CalculateLookAt();

		float m_AspectRatio{};
		float m_Fov{};

		const float m_KeyboardMoveSensitivity{ 10.f };
		const float m_KeyboardMoveMultiplier{ 10.f };
		const float m_MouseRotationSensitivity{ .1f };
		const float m_MouseMoveSensitivity{ 2.f };

		XMFLOAT4 m_AbsoluteRotation{}; //Pitch(x) & Yaw(y) only
		XMFLOAT3 m_RelativeTranslation{};

		XMFLOAT3 m_Position{};
		const XMFLOAT3 m_ViewForward{};

		XMFLOAT4X4 m_World{};
		XMFLOAT3 m_WorldPosition{};
		XMFLOAT3 m_WorldScale{};
		XMFLOAT4 m_WorldRotation{};

		XMFLOAT3 m_Forward{};
		XMFLOAT3 m_Right{};
		XMFLOAT3 m_Up{};

		XMFLOAT4X4 m_View{};
		XMFLOAT4X4 m_Projection{};
		XMFLOAT4X4 m_ViewInverse{};
		XMFLOAT4X4 m_ViewProjection{};
		XMFLOAT4X4 m_ViewProjectionInverse{};

		float m_FarPlane{}, m_NearPlane{}, m_Size{};
		bool m_IsActive{}, m_PerspectiveProjection{};
	};
}
