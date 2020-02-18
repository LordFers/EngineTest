#include "Camera.h"

namespace Engine {
	Camera::Camera()
	{
		m_Position = glm::vec3(0.0f, 5.0f, 20.0f);
		m_Pitch = m_Yaw = m_Roll = 0.0f;
		m_Projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
		m_View = glm::mat4(1.0f);
	}

	Camera::~Camera()
	{
		//TODO:
	}

	void Camera::Update()
	{
		m_Position.y += 0.001f;
		//m_Position.z += 0.001f;

		//m_Pitch += 0.01f;
		//m_Yaw += 0.01f;
		m_Yaw = glm::radians(0.0f);
		m_View = glm::mat4(1.0f); //setIdentity.
		m_View = glm::rotate(m_View, m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		m_View = glm::rotate(m_View, m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		m_View = glm::translate(m_View, -m_Position);
		//m_View = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));
	}
}