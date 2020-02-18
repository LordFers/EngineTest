#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine {
	class Camera {
	public:
		Camera();
		virtual ~Camera();

	public:
		void Update();

	public:
		inline const glm::mat4& getView()
		{
			return m_View;
		}

		inline const glm::mat4& getProjection()
		{
			return m_Projection;
		}

	private:
		glm::mat4 m_Projection;
		glm::mat4 m_View;

	private:
		glm::vec3 m_Position;
		glm::quat m_Orientation;
		float m_Pitch, m_Yaw, m_Roll;
	};
}