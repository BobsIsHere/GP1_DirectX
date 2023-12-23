#include "pch.h"
#include "Camera.h"

dae::Camera::Camera(float fovAngle, float aspect, const Vector3& origin) :
	m_AspectRatio{ aspect },
	m_Origin{ origin },
	m_TotalPitch{},
	m_TotalYaw{}
{
	m_FOV = tanf((fovAngle * TO_RADIANS) / 2.f);

	m_Forward = Vector3::UnitZ; 
	m_Right = Vector3::UnitX; 
	m_Up = Vector3::UnitY; 
}

dae::Camera::~Camera() 
{

}

void dae::Camera::Update(const Timer* pTimer)
{
	const float deltaTime = pTimer->GetElapsed();

	//keyboard input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
	const float speed{ 20.f };

	if (pKeyboardState[SDL_SCANCODE_W])
	{
		m_Origin += m_Forward * speed * deltaTime;
	}
	else if (pKeyboardState[SDL_SCANCODE_S])
	{
		m_Origin -= m_Forward * speed * deltaTime;
	}
	else if (pKeyboardState[SDL_SCANCODE_D])
	{
		m_Origin += m_Right * speed * deltaTime;
	}
	else if (pKeyboardState[SDL_SCANCODE_A])
	{
		m_Origin -= m_Right * speed * deltaTime;
	}

	//mouse input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	if ((mouseState & SDL_BUTTON_RMASK) != 0 && !(mouseState & SDL_BUTTON_LMASK) != 0)
	{
		// Right mouse button is pressed
		m_TotalYaw += mouseX;
		m_TotalPitch += mouseY;
	}

	if ((mouseState & SDL_BUTTON_LMASK) != 0 && (mouseState & SDL_BUTTON_RMASK) != 0)
	{
		m_Origin += m_Up * speed * float(mouseY) * deltaTime;
	}
	else if ((mouseState & SDL_BUTTON_LMASK) != 0)
	{
		if (mouseY < 0)
		{
			m_Origin += m_Forward * speed * deltaTime;
		}
		else if (mouseY > 0)
		{
			m_Origin -= m_Forward * speed * deltaTime;
		}

		// Right mouse button is pressed
		m_TotalYaw += mouseX;
	}
	CalculateViewMatrix();
	CalculateProjectionMatrix();
}

void dae::Camera::CalculateViewMatrix()
{
	//ONB => invViewMatrix
	//Inverse(ONB) => ViewMatrix
	Matrix rotation{ Matrix::CreateRotationX(-m_TotalPitch * TO_RADIANS) * Matrix::CreateRotationY(m_TotalYaw * TO_RADIANS) };

	m_Forward = rotation.TransformVector(Vector3::UnitZ);
	m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
	m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();

	m_ViewMatrix = Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Up);
	m_InvViewMatrix = m_ViewMatrix.Inverse();
}

void dae::Camera::CalculateProjectionMatrix()
{
	const float zn{ 0.1f };
	const float zf{ 100.f };

	m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_FOV, m_AspectRatio, zn, zf);
}

dae::Matrix dae::Camera::GetViewMatrix() const
{
	return m_ViewMatrix;
}

dae::Matrix dae::Camera::GetProjectionMatrix() const
{

	return m_ProjectionMatrix;
}

dae::Matrix dae::Camera::GetInverseViewMatrix() const
{
	return m_InvViewMatrix;
}

dae::Vector3 dae::Camera::GetCameraOrigin() const
{
	return m_Origin;
}