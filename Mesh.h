#pragma once

#include <vector>

namespace Engine {
	class Mesh {
	public:
		Mesh();
		~Mesh();

	public:
		void Init(std::vector<float> vertices, std::vector<unsigned short> indices);

	public:
		inline unsigned int GetVBO() const { return m_VBO_ID; }
		inline unsigned int GetIBO() const { return m_IBO_ID; }
		inline unsigned int GetCountVertices() const { return m_Vertices.size(); }
		inline unsigned int GetCountIndices() const { return m_Indices.size(); }

	private:
		std::vector<float> m_Vertices;
		std::vector<unsigned short> m_Indices;

	private:
		unsigned int m_VBO_ID;
		unsigned int m_IBO_ID;
		//Material m_Material; -> puede ser la configuración del Shader.
	};
}