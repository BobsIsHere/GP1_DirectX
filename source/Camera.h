#pragma once

namespace dae
{
	class Camera final
	{
	public:
		// CONSTRUCTOR AND DESTRUCTOR
		Camera(const Vector3& origin, float fovAngle, float nearPlane, float farPlane, float aspectRatio);
		~Camera();

		// RULE OF FIVE
		Camera(const Camera& other) = delete;
		Camera& operator=(const Camera& other) = delete;
		Camera(Camera&& other) noexcept = delete;
		Camera& operator=(Camera&& other) noexcept = delete;

		// MEMBER FUNCTIONS
		void Update(const Timer* pTimer);
		Matrix GetViewMatrix() const;
		Matrix GetProjectionMatrix() const;
		Matrix GetInverseViewMatrix() const;
		Vector3 GetCameraOrigin() const;

	private:
		// MEMBER VARIABLES
		const float m_ZN;
		const float m_ZF;
		const float m_AspectRatio;
		const float m_FOV;
		float m_TotalPitch;
		float m_TotalYaw;

		Vector3 m_Forward;
		Vector3 m_Right;
		Vector3 m_Up;
		Vector3 m_Origin;

		Matrix m_InvViewMatrix{};
		Matrix m_ViewMatrix{};
		Matrix m_ProjectionMatrix{};

		// MEMBER FUNCTIONS
		void CalculateViewMatrix();
		void CalculateProjectionMatrix();
	};

}