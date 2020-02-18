#include <iostream>
#include <unordered_set>
#include <glm/vec3.hpp>
#include <algorithm>
#include "ColladaParser.h"
#include "AnimatedModel.h"
#include "vboindexer.h"
#include "VertexSkinData.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

/*std::vector<std::string> split(std::string& phrase, const std::string& delimiter) {
	std::vector<std::string> list;

	std::size_t pos = 0;
	std::string token;
	while ((pos = phrase.find(delimiter)) != std::string::npos) {
		token = phrase.substr(0, pos);
		list.push_back(token);
		phrase.erase(0, pos + delimiter.length());
	}
	list.push_back(phrase);
	return list;
}*/

std::vector<std::string> Split(std::string const& original, char separator)
{
	std::vector<std::string> results;
	std::string::const_iterator start = original.begin();
	std::string::const_iterator end = original.end();
	std::string::const_iterator next = std::find(start, end, separator);
	while (next != end) {
		results.push_back(std::string(start, next));
		start = next + 1;
		next = std::find(start, end, separator);
	}
	results.push_back(std::string(start, next));
	return results;
}

inline std::string Mid(const std::string& s, int start, int end) {
	return s.substr(start, end - start + 1);
}

void DeleteBlank(std::string& str)
{
	int firstPos = 0;
	int lastPos = 0;

	for (size_t i = 0; i < str.size(); ++i)
	{
		if (!(str[i] == ' ' || str[i] == '\n')) {
			firstPos = i;
			break;
		}
	}

	for (int i = str.size() - 1; i > 0; --i)
	{
		if (!(str[i] == ' ' || str[i] == '\n')) {
			lastPos = i;
			break;
		}
	}

	str = Mid(str, firstPos, lastPos);
}

void deleteSharp(std::string& data)
{
	data = ReplaceAll(data, "#", " ");
	DeleteBlank(data);
}

std::vector<std::string> GetSeparatedData(std::string& data)
{
	data = ReplaceAll(data, "\n", " ");
	DeleteBlank(data);
	return Split(data, ' ');
}

std::string GetChannelWithoutSeparator(std::string& data)
{
	std::string s = "";
	for (size_t i = 0; i < data.size(); ++i)
	{
		if (data[i] == '/')
			return s;

		s += data[i];
	}

	return s;
}

namespace Engine {
	ColladaParser::ColladaParser()
	{
		m_Context = NULL;
		m_Document = NULL;
		m_InputSize = 0;
		m_MeshName = "";
	}

	ColladaParser::~ColladaParser()
	{
		xmlXPathFreeContext(m_Context);
		xmlFreeDoc(m_Document);
		xmlCleanupParser();
	}

	void ColladaParser::Init(const char* fileName)
	{
		/* Init XMLDoc and XPath Context */
		m_Document = xmlParseFile(fileName);
		m_Context = xmlXPathNewContext(m_Document);
		if (m_Context == NULL) {
			printf("Error in xmlXPathNewContext\n");
			//throw Exception
		}

		/* Init Collada Parser */
		m_MeshName = GetMeshName();
		m_InputSize = GetInputSize();
	}

	std::string ColladaParser::GetValueXPath(const std::string& xpath)
	{
		xmlXPathObjectPtr result;
		result = xmlXPathEvalExpression((xmlChar*)xpath.c_str(), m_Context);

		if (result == NULL) {
			printf("Error in Expression Path.\n");
			return "";
		}

		if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
			xmlXPathFreeObject(result);
			printf("No result: problem solving XML nodes.\n");
			return "";
		}

		xmlNodeSet* nodeset = result->nodesetval;
		xmlChar* keyword = xmlNodeListGetString(m_Document, nodeset->nodeTab[0]->xmlChildrenNode, 1);
		xmlXPathFreeObject(result);

		std::string ret = (const char*)keyword;
		xmlFree(keyword);
		return ret;
	}

	std::string ColladaParser::GetMeshName()
	{
		return GetValueXPath("/COLLADA/library_geometries/geometry/@id");
	}

	/* Cantidad de inputs que contendrán los índices del Mesh. (Sólo en lib_geometry) */
	unsigned short ColladaParser::GetInputSize()
	{
		xmlXPathObjectPtr result;
		result = xmlXPathEvalExpression((xmlChar*)"/COLLADA/library_geometries/geometry//p/..//input", m_Context);

		if (result == NULL) {
			printf("Error in Expression Path.\n");
			return 3;
		}

		if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
			xmlXPathFreeObject(result);
			printf("No result: problem solving XML nodes.\n");
			return 3;
		}

		xmlNodeSet* nodeset = result->nodesetval;
		int val = nodeset->nodeNr;
		xmlXPathFreeObject(result);
		return val;
	}

	std::vector<float> ColladaParser::GetVertexPositions()
	{
		std::string str = GetValueXPath("/COLLADA/library_geometries//float_array[@id='" + m_MeshName + "-positions-array']");
		auto vertexData = GetSeparatedData(str);
		auto indVert = GetIndexVertex();

		std::vector<float> vertices;
		vertices.resize(indVert.size() * 3);
		for (size_t i = 0; i < indVert.size(); ++i)
		{
			vertices[i * 3] = (float)atof(vertexData[indVert[i] * 3].c_str());
			vertices[i * 3 + 1] = (float)atof(vertexData[indVert[i] * 3 + 1].c_str());
			vertices[i * 3 + 2] = (float)atof(vertexData[indVert[i] * 3 + 2].c_str());
		}

		return vertices;
	}

	std::vector<float> ColladaParser::GetVertexNormals()
	{
		std::string str = GetValueXPath("/COLLADA/library_geometries//float_array[@id='" + m_MeshName + "-normals-array']");
		auto normalsData = GetSeparatedData(str);
		auto indNormals = GetIndexNormals();

		std::vector<float> normals;
		normals.resize(indNormals.size() * 3);
		for (size_t i = 0; i < indNormals.size(); ++i)
		{
			normals[i * 3] = (float)atof(normalsData[indNormals[i] * 3].c_str());
			normals[i * 3 + 1] = (float)atof(normalsData[indNormals[i] * 3 + 1].c_str());
			normals[i * 3 + 2] = (float)atof(normalsData[indNormals[i] * 3 + 2].c_str());
		}

		return normals;
	}

	std::vector<float> ColladaParser::GetVertexUVs()
	{
		std::string str = GetValueXPath("/COLLADA/library_geometries//float_array[@id='" + m_MeshName + "-map-0-array']");
		auto uvsData = GetSeparatedData(str);
		auto indUV = GetIndexUVs();

		std::vector<float> uvs;
		uvs.resize(indUV.size() * 2);
		for (size_t i = 0; i < indUV.size(); ++i)
		{
			uvs[i * 2] = (float)atof(uvsData[indUV[i] * 2].c_str());
			uvs[i * 2 + 1] = (float)atof(uvsData[indUV[i] * 2 + 1].c_str());
		}

		return uvs;
	}

	std::vector<unsigned short> ColladaParser::GetIndexVertex()
	{
		std::string str = GetValueXPath("/COLLADA/library_geometries//p");
		auto values = GetSeparatedData(str);

		size_t size = (values.size() / m_InputSize);
		std::vector<unsigned short> indices;
		indices.resize(size);
		size_t i = 0;
		for (i = 0; i < size; ++i)
		{
			indices[i] = (unsigned short)atoi(values[i * m_InputSize].c_str());
		}

		return indices;
	}

	std::vector<unsigned short> ColladaParser::GetIndexNormals()
	{
		std::string str = GetValueXPath("/COLLADA/library_geometries//p");
		auto values = GetSeparatedData(str);

		size_t size = (values.size() / m_InputSize);
		std::vector<unsigned short> indices;
		indices.resize(size);
		size_t i = 0;
		for (i = 0; i < size; ++i)
		{
			indices[i] = (unsigned short)atoi(values[i * m_InputSize + 1].c_str());
		}

		return indices;
	}

	std::vector<unsigned short> ColladaParser::GetIndexUVs()
	{
		std::string str = GetValueXPath("/COLLADA/library_geometries//p");
		auto values = GetSeparatedData(str);

		size_t size = (values.size() / m_InputSize);
		std::vector<unsigned short> indices;
		indices.resize(size);
		size_t i = 0;
		for (i = 0; i < size; ++i)
		{
			indices[i] = (unsigned short)atoi(values[i * m_InputSize + 2].c_str());
		}

		return indices;
	}

	std::string ColladaParser::GetJointSource()
	{
		std::string str = GetValueXPath("/COLLADA/library_controllers/controller/skin/vertex_weights/input[@semantic='JOINT']/@source");
		deleteSharp(str); //Para quitar los # molestos.
		return str;
	}

	std::string ColladaParser::GetWeightSource()
	{
		std::string str = GetValueXPath("/COLLADA/library_controllers/controller/skin/vertex_weights/input[@semantic='WEIGHT']/@source");
		deleteSharp(str); //Para quitar los # molestos.
		return str;
	}

	std::vector<unsigned short> ColladaParser::GetNewIndexPosition()
	{
		std::string str = GetValueXPath("/COLLADA/library_geometries//float_array[@id='" + m_MeshName + "-positions-array']"); //Cube-mesh-positions-array
		auto vertexData = GetSeparatedData(str);
		auto indVert = GetIndexVertex();

		std::vector<unsigned short> indices;
		unsigned short value = 5851;
		for (size_t i = 0; i < indVert.size(); ++i)
		{
			if (std::find(indices.begin(), indices.end(), indVert[i]) != indices.end())
			{
				indices.push_back(value++);
			}
			else
			{
				indices.push_back(indVert[i]);
			}
		}

		return indices;
	}

	void ColladaParser::GetVertexSkinning(std::vector<float>& vertexWeights, std::vector<unsigned short>& vertexJoints, SkeletonData& skeleton)
	{
		int INPUT_WEIGHTS_SIZE = 2; //Siempre vale 2.
		std::string str = GetValueXPath("/COLLADA/library_controllers/controller/skin/source[@id='" + GetJointSource() + "']/Name_array");
		auto s_jointList = GetSeparatedData(str);
		m_JointList = s_jointList;

		str = GetValueXPath("/COLLADA/library_controllers/controller/skin/vertex_weights/vcount");
		auto s_vcount = GetSeparatedData(str); //vertex_count weights for vertex.

		str = GetValueXPath("/COLLADA/library_controllers/controller/skin/vertex_weights/v");
		auto s_v = GetSeparatedData(str); //indices 740 model.dae
		auto indVert = GetIndexVertex(); //necesitamos usar los índices reales para completar los que no aparecen como siempre.
		//auto indNewVert = GetNewIndexPosition(); //Nuevos índices para que el Shader sepa cuál vértice tomar, y no afectar otros.

		str = GetValueXPath("/COLLADA/library_controllers/controller/skin/source[@id='" + GetWeightSource() + "']/float_array");
		auto s_weightList = GetSeparatedData(str); //lista del vertex weights.

		std::vector<unsigned short> indJoints;
		std::vector<unsigned short> indWeight;
		indJoints.resize(s_v.size() / INPUT_WEIGHTS_SIZE);
		indWeight.resize(s_v.size() / INPUT_WEIGHTS_SIZE);
		for (size_t i = 0; i < indWeight.size(); ++i)
		{
			indJoints[i] = (unsigned short)atoi(s_v[i * INPUT_WEIGHTS_SIZE].c_str());
			indWeight[i] = (unsigned short)atoi(s_v[i * INPUT_WEIGHTS_SIZE + 1].c_str());
		}

		std::vector<float> weights;
		std::vector<unsigned short> joints;
		std::vector<VertexSkinData> skinningData;

		//s_v bien, s_weightList bien, v_count está bien.
		unsigned int offset = 0;
		for (auto count : s_vcount)
		{
			int icount = atoi(count.c_str());
			
			if (icount == 1)
			{
				weights.push_back((float)atof(s_weightList[indWeight[offset]].c_str()));
				weights.push_back(0.0f);
				weights.push_back(0.0f);

				joints.push_back((unsigned short)indJoints[offset]);
				joints.push_back((unsigned short)0);
				joints.push_back((unsigned short)0);
			}
			else if (icount == 2)
			{
				weights.push_back((float)atof(s_weightList[indWeight[offset]].c_str()));
				weights.push_back((float)atof(s_weightList[indWeight[offset + 1]].c_str()));
				weights.push_back(0.0f);

				joints.push_back((unsigned short)indJoints[offset]);
				joints.push_back((unsigned short)(indJoints[offset + 1]));
				joints.push_back((unsigned short)0);
			}
			else if (icount >= 3)
			{
				if (icount > 3)
				{
					float total = 0.0f;
					for (size_t i = 0; i < 3; ++i) //Para recalcular las ponderaciones.
					{
						total += (float)atof(s_weightList[indWeight[offset + i]].c_str());
					}

					weights.push_back((float)std::fmin((float)atof(s_weightList[indWeight[offset + 0]].c_str()) / total, 1));
					weights.push_back((float)std::fmin((float)atof(s_weightList[indWeight[offset + 1]].c_str()) / total, 1));
					weights.push_back((float)std::fmin((float)atof(s_weightList[indWeight[offset + 2]].c_str()) / total, 1));
				}
				else
				{
					weights.push_back((float)atof(s_weightList[indWeight[offset + 0]].c_str()));
					weights.push_back((float)atof(s_weightList[indWeight[offset + 1]].c_str()));
					weights.push_back((float)atof(s_weightList[indWeight[offset + 2]].c_str()));
				}

				joints.push_back((unsigned short)indJoints[offset]);
				joints.push_back((unsigned short)(indJoints[offset + 1]));
				joints.push_back((unsigned short)(indJoints[offset + 2]));
			}

			offset += icount;
		}

		vertexWeights.resize(indVert.size() * 3);
		vertexJoints.resize(indVert.size() * 3);
		for (size_t i = 0; i < indVert.size(); ++i)
		{
			vertexWeights[i * 3] = weights[indVert[i] * 3];
			vertexWeights[i * 3 + 1] = weights[indVert[i] * 3 + 1];
			vertexWeights[i * 3 + 2] = weights[indVert[i] * 3 + 2];

			vertexJoints[i * 3] = joints[indVert[i] * 3];
			vertexJoints[i * 3 + 1] = joints[indVert[i] * 3 + 1];
			vertexJoints[i * 3 + 2] = joints[indVert[i] * 3 + 2];
		}

		xmlXPathObjectPtr result;
		result = xmlXPathEvalExpression((xmlChar*)"/COLLADA/library_visual_scenes/visual_scene/node/node[@type='JOINT']", m_Context);

		if (result == NULL) {
			printf("Error in Expression Path.\n");
			return;
		}

		if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
			xmlXPathFreeObject(result);
			printf("No result: problem solving XML nodes.\n");
			return;
		}

		xmlNodePtr node = result->nodesetval->nodeTab[0];
		skeleton = GetSkeletonData(node, 0); //copio los datos del puntero nuevo al viejo.
		xmlXPathFreeObject(result);
	}

	//Esto está perfecto.
	glm::mat4 GetMatInstanceContent(const char* text)
	{
		std::string str = std::string(text);
		auto data = GetSeparatedData(str);

		size_t k = 0;
		glm::mat4 mat = glm::mat4(0.0f);
		for (auto value : data)
		{
			int x = (int)(k % 4);
			int y = (int)(k / 4);
			mat[x][y] = (float)atof(value.c_str());
			++k;
		}

		return mat;
	}

	glm::mat4 GetMatInstanceContentOffset(const char* text, unsigned int offset)
	{
		std::string str = std::string(text);
		auto data = GetSeparatedData(str);

		size_t k = 0;
		glm::mat4 mat = glm::mat4(0.0f);

		for (size_t i = 0; i < 16; ++i)
		{
			int x = (int)(i % 4);
			int y = (int)(i / 4);

			mat[x][y] = (float)atof(data[i + offset * 16].c_str());
		}

		return mat;
	}

	//Esto hasta donde se vio estaba perfecto.
	static int id = 0;
	SkeletonData ColladaParser::GetSkeletonData(xmlNode* node, unsigned short parent)
	{
		//Temporalmente hago esta mierda porqué diseñé todo como el culo, por leer mal la especificación de COLLADA en Khronos Groups.
		/*xmlXPathObjectPtr result;
		result = xmlXPathEvalExpression((xmlChar*)"/COLLADA/library_controllers/controller/skin/source[@id='Armature_Cube-skin-bind_poses']/float_array", m_Context); //xmlXPathEvalExpression((xmlChar*)"/COLLADA/library_controllers/controller/skin/source[@id='MutantMeshController-Matrices']/float_array", m_Context);//xmlXPathEvalExpression((xmlChar*)"/COLLADA/library_controllers/controller/skin/source[@id='Armature_Cube-skin-bind_poses']/float_array", m_Context);
		xmlNodePtr n = result->nodesetval->nodeTab[0];

		std::string matchar = (const char*)n->children->content;

		xmlXPathFreeObject(result);*/
		//////////////
		SkeletonData skl;
		skl.m_ID = id++;
		skl.m_IDName = (const char*)node->properties->children->content;
		skl.m_ParentID = parent;
		skl.m_BindPose = GetMatInstanceContent((const char*)node->children->next->children->content);
		
		skl.m_InvBindPose = glm::inverse(skl.m_BindPose);
		if (skl.m_ID == 0)
			skl.m_InvBindPose = glm::rotate(skl.m_InvBindPose, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		
		//skl.m_InvBindPose = GetMatInstanceContentOffset(matchar.c_str(), skl.m_ID);
		//skl.m_BindPose = glm::inverse(skl.m_InvBindPose);
		//skl.m_AnimatedTransform = glm::mat4(1.0f);
		//std::cout << node->properties->children->content << std::endl;

		xmlNode* cur_node = node->children;
		while (cur_node != nullptr) //necesario para que recorra el árbol.
		{
			if (cur_node->type == XML_ELEMENT_NODE)
			{
				if (std::string((const char*)cur_node->name) == "node")
				{
					skl.m_Childrens.push_back(GetSkeletonData(cur_node, skl.m_ID));
				}
			}

			cur_node = cur_node->next;
		}

		return skl;
	}

	void ColladaParser::GetMeshData(std::vector<float>& vertices, std::vector<unsigned short>& indices, SkeletonData& skeleton)
	{
		std::vector<unsigned short> jointList;//IDs
		std::vector<float> weightList;
		GetVertexSkinning(weightList, jointList, skeleton);

		//auto anim = GetAnimation(skeleton); lo cargo afuera.
		auto verticesCorrect = GetVertexPositions();
		auto normals = GetVertexNormals();
		auto uvs = GetVertexUVs();

		std::vector<glm::vec3> glmVertices;
		std::vector<glm::vec3> glmNormals;
		std::vector<glm::vec2> glmUVs;
		std::vector<glm::vec3> glmWeights;
		std::vector<glm::ivec3> glmJoints;

		glmVertices.resize(verticesCorrect.size() / 3);
		glmNormals.resize(normals.size() / 3);
		glmUVs.resize(uvs.size() / 2);
		glmWeights.resize(weightList.size() / 3);
		glmJoints.resize(jointList.size() / 3);

		size_t i = 0;
		for (i = 0; i < glmVertices.size(); ++i)
		{
			float x = verticesCorrect[i * 3];
			float y = verticesCorrect[i * 3 + 1];
			float z = verticesCorrect[i * 3 + 2];

			glmVertices[i] = glm::vec3(x, y, z);
		}

		for (i = 0; i < glmNormals.size(); ++i)
		{
			float nx = normals[i * 3];
			float ny = normals[i * 3 + 1];
			float nz = normals[i * 3 + 2];

			glmNormals[i] = glm::vec3(nx, ny, nz);
		}
		
		for (i = 0; i < glmUVs.size(); ++i)
		{
			float u = uvs[i * 2];
			float v = uvs[i * 2 + 1];

			glmUVs[i] = glm::vec2(u, v);
		}

		for (i = 0; i < glmWeights.size(); ++i)
		{
			float w1 = weightList[i * 3];
			float w2 = weightList[i * 3 + 1];
			float w3 = weightList[i * 3 + 2];

			glmWeights[i] = glm::vec3(w1, w2, w3);
		}

		for (i = 0; i < glmJoints.size(); ++i)
		{
			float j1 = jointList[i * 3];
			float j2 = jointList[i * 3 + 1];
			float j3 = jointList[i * 3 + 2];

			glmJoints[i] = glm::ivec3(j1, j2, j3);
		}

		std::vector<unsigned short> newIndices;
		std::vector<glm::vec3> newVertices;
		std::vector<glm::vec2> newUVs;
		std::vector<glm::vec3> newNormals;
		std::vector<glm::vec3> newWeights;
		std::vector<glm::ivec3> newJoints;

		indexVBO_Skeleton(glmVertices, glmUVs, glmNormals, glmWeights, glmJoints, newIndices, newVertices, newUVs, newNormals, newWeights, newJoints);

		vertices.resize(newVertices.size() * 3 + newNormals.size() * 3  + newUVs.size() * 2 + newWeights.size() * 3 + newJoints.size() * 3);
		for (i = 0; i < vertices.size() / 14; ++i)
		{
			vertices[i * 14] = newVertices[i].x;// *0.1f;
			vertices[i * 14 + 1] = newVertices[i].y;// *0.1f; cambiamos el eje y por el z, así nos queda el y up, en lugar del z up,.
			vertices[i * 14 + 2] = newVertices[i].z;// *0.1f;
			
			vertices[i * 14 + 3] = newNormals[i].x;
			vertices[i * 14 + 4] = newNormals[i].y;
			vertices[i * 14 + 5] = newNormals[i].z;
			
			vertices[i * 14 + 6] = newUVs[i].x;
			vertices[i * 14 + 7] = 1.0f - newUVs[i].y;

			vertices[i * 14 + 8] = newWeights[i].x;
			vertices[i * 14 + 9] = newWeights[i].y;
			vertices[i * 14 + 10] = newWeights[i].z;

			vertices[i * 14 + 11] = (float)newJoints[i].x;
			vertices[i * 14 + 12] = (float)newJoints[i].y;
			vertices[i * 14 + 13] = (float)newJoints[i].z;
		}

		for (i = 0; i < newIndices.size() / 3; ++i)
		{
			indices.push_back(newIndices[i * 3 + 0]);
			indices.push_back(newIndices[i * 3 + 1]);
			indices.push_back(newIndices[i * 3 + 2]); //lo cargamos al revés, para que se lea en sentido anti horario, sino, hay que utilizar el estándar LH.
		}
	}

	const SkeletonData& GetSkeletonFromID(const SkeletonData& skeleton, unsigned short ID)
	{
		if (skeleton.m_ID == ID)
			return skeleton;

		for (size_t i = 0; i < skeleton.m_Childrens.size(); ++i)
		{
			const SkeletonData& tempSkeleton = GetSkeletonFromID(skeleton.m_Childrens[i], ID);
			if (tempSkeleton.m_ID == ID)
				return tempSkeleton;
		}

		return skeleton;
	}

	glm::mat4* GetListMatrices(const char* text, size_t count, const SkeletonData& skeleton)
	{
		size_t i, j;
		int x, y;
		std::string s_m = text;
		std::vector<std::string> m = GetSeparatedData(s_m);
		glm::mat4* mats = new glm::mat4[count];

		for (i = 0; i < count; ++i)
		{
			mats[i] = glm::mat4(1.0f);
			for (j = 0; j < 16; ++j)
			{
				x = j % 4;
				y = j / 4;
				mats[i][x][y] = (float)atof(m[i * 16 + j].c_str());
			}
		}

		return mats;
	}

	unsigned short GetIDFromSkeleton(const SkeletonData& skeleton, const char* text)
	{
		if (!strcmp(text, skeleton.m_IDName))
			return skeleton.m_ID;

		for (size_t i = 0; i < skeleton.m_Childrens.size(); ++i)
		{
			unsigned short value = GetIDFromSkeleton(skeleton.m_Childrens[i], text);
			if (value != 0xFFFF) //65535
				return value;
		}

		return 0xFFFF;
	}

	Animation ColladaParser::GetAnimation(const SkeletonData& skeleton)
	{
		Animation animation;
		xmlXPathObjectPtr result;
		xmlNode* node;
		size_t i;

		result = xmlXPathEvalExpression((xmlChar*)"/COLLADA/library_animations//animation", m_Context);
		animation.m_BonesAnimated = 0;

		if (result == NULL) {
			printf("Error in Expression Path.\n");
			return animation;
		}

		if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
			xmlXPathFreeObject(result);
			printf("No result: problem solving XML nodes.\n");
			return animation;
		}

		animation.m_BonesAnimated = result->nodesetval->nodeNr;
		animation.m_AnimationBone = new AnimationBone[animation.m_BonesAnimated];

		//std::vector<Animation&> listAnimations;
		//listAnimations.push_back(animation);

		node = result->nodesetval->nodeTab[0];
		for (i = 0; i < animation.m_BonesAnimated; ++i)
		{
			AnimationBone& anim = animation.m_AnimationBone[i];
			std::string s = (const char*)node->children->next->next->next->next->next->next->next->next->next->properties->next->children->content;//(const char*)node->children->next->next->next->next->properties->next->children->content;
			s = GetChannelWithoutSeparator(s);
			anim.m_ID = GetIDFromSkeleton(skeleton, s.c_str());
			anim.m_TransformsCount = atoi((const char*)node->children->next->children->next->properties->next->children->content);//atoi((const char*)node->children->next->children->properties->next->children->content) / 16;
			anim.m_Orient = new glm::quat[anim.m_TransformsCount];
			anim.m_Translation = new glm::vec3[anim.m_TransformsCount];
			
			glm::mat4* transforms = GetListMatrices((const char*)node->children->next->next->next->children->next->children->content, anim.m_TransformsCount, skeleton);//GetListMatrices((const char*)node->children->next->children->children->content, anim.m_TransformsCount, skeleton);

			for (size_t i = 0; i < anim.m_TransformsCount; ++i)
			{
				//const Skeleton& temp = GetSkeletonFromID(skeleton, anim.m_ID);
				anim.m_Orient[i] = glm::quat_cast(transforms[i]);// );// *(temp.m_InvBindPose);
				anim.m_Translation[i] = glm::vec3(transforms[i][3].x, transforms[i][3].y, transforms[i][3].z);
				//if (!strcmp(temp.m_IDName, "Torso"))//(!strcmp(temp.m_IDName, "Upper_Arm_L") || !strcmp(temp.m_IDName, "Upper_Arm_R"))
				//{
				//	anim.m_Transforms[i] = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 0.0f)) ;
				//}
			}

			delete [] transforms;
			node = node->next->next; //siguiente animation.
		}

		xmlXPathFreeObject(result);
		return animation;
	}
}