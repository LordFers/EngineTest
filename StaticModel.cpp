#include "StaticModel.h"
#include "ColladaParser.h"

namespace Engine {
	StaticModel::StaticModel()
	{

	}

	StaticModel::~StaticModel()
	{
		//TODO:
	}

	void StaticModel::Init(const char* fileName)
	{
		ColladaParser parser;
		parser.Init(fileName);
		std::vector<float> vertices;
		std::vector<unsigned short> indices;
		//parser.GetMeshData(vertices, indices);
		m_Mesh.Init(vertices, indices);
	}
}