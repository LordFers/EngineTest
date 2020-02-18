#include <GL/glew.h>
#include "Mesh.h"

namespace Engine {
	Mesh::Mesh()
	{

	}

	Mesh::~Mesh()
	{
		//TODO:
	}

	void Mesh::Init(std::vector<float> vertices, std::vector<unsigned short> indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		glGenBuffers(1, &m_VBO_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_IBO_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
	}
}