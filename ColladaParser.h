#pragma once

#include <string>
#include <vector>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "AnimatedModel.h"

namespace Engine {
	class ColladaParser {
	public:
		ColladaParser();
		virtual ~ColladaParser();

	public:
		void Init(const char* fileName);

	private:
		std::vector<unsigned short> GetIndexVertex();
		std::vector<unsigned short> GetIndexNormals();
		std::vector<unsigned short> GetIndexUVs();

		std::vector<float> GetVertexPositions();
		std::vector<float> GetVertexNormals();
		std::vector<float> GetVertexUVs();
		void GetVertexSkinning(std::vector<float>& weights, std::vector<unsigned short>& joints, SkeletonData& skeleton);
		SkeletonData GetSkeletonData(xmlNode* node, unsigned short parent);
		//std::vector<unsigned short> GetVertexJoints();

	public:
		void GetMeshData(std::vector<float>& vertices, std::vector<unsigned short>& indices, SkeletonData& skeleton);
		Animation GetAnimation(const SkeletonData& skeleton);

	private:
		std::string GetJointSource();
		std::string GetWeightSource();

	private:
		unsigned short GetInputSize();
		std::string GetMeshName();
		std::string GetValueXPath(const std::string& xpath);

	private: //borrar:
		std::vector<unsigned short> GetNewIndexPosition();

	private:
		xmlDoc* m_Document;
		xmlXPathContext* m_Context;
		std::string m_MeshName;
		unsigned short m_InputSize;
		std::vector<std::string> m_JointList;
	};
}