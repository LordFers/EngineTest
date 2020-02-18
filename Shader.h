#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

typedef unsigned int GLuint;

namespace Engine {
	class Shader {
	public:
		Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
		virtual ~Shader();

	public:
		// ------------------------------------------------------------------------
		void use();
		void setBool(const std::string& name, bool value) const;
		void setInt(const std::string& name, int value) const;
		void setFloat(const std::string& name, float value) const;
		// ------------------------------------------------------------------------
		void setVec2(const std::string& name, const glm::vec2& value) const;
		void setVec2(const std::string& name, float x, float y) const;
		void setVec3(const std::string& name, const glm::vec3& value) const;
		void setVec3(const std::string& name, float x, float y, float z) const;
		void setVec4(const std::string& name, const glm::vec4& value) const;
		void setVec4(const std::string& name, float x, float y, float z, float w);
		// ------------------------------------------------------------------------
		void setMat2(const std::string& name, const glm::mat2& mat) const;
		void setMat3(const std::string& name, const glm::mat3& mat) const;
		void setMat4(const std::string& name, const glm::mat4& mat) const;
		void setMat4Array(const std::string& name, std::vector<glm::mat4>& mat);
		// ------------------------------------------------------------------------
		void setTexture0(const std::string& name, const GLuint texID) const;
	private:
		void checkCompileErrors(unsigned int shader, std::string type);

	protected:
		unsigned int m_ID;
	};
}