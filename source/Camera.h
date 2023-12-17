#pragma once

namespace dae
{
	class Camera final
	{
	public:
		Camera(float fovAngle, float aspect, const Vector3& origin);
		~Camera();

		//Rule of Five
		Camera(const Camera& other) = delete;
		Camera& operator=(const Camera& other) = delete;
		Camera(Camera&& other) noexcept = delete;
		Camera& operator=(Camera&& other) noexcept = delete;

		//Member Functions
		void Update(const Timer* pTimer);
		Matrix GetViewMatrix() const;
		Matrix GetProjectionMatrix() const;
		Matrix CreateWorldViewProjectionMatrix() const;

	private:
		//Member Variables
		float m_FOV;
		float m_AspectRatio;
		float m_TotalPitch;
		float m_TotalYaw;

		Vector3 m_Forward;
		Vector3 m_Right;
		Vector3 m_Up;
		Vector3 m_Origin;

		Matrix m_InvViewMatrix{};
		Matrix m_ViewMatrix{};
		Matrix m_ProjectionMatrix{};

		//Member Functions
		void CalculateViewMatrix();
		void CalculateProjectionMatrix();
	};

}