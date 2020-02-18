#include "AnimatedModel.h"
#include "ColladaParser.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Windows.h>

namespace Engine {
	AnimatedModel::AnimatedModel()
	{
		//m_SkeletonRoot = new Skeleton;
	}

	AnimatedModel::~AnimatedModel()
	{
		//TODO:
	}

	void FillSkeleton(Skeleton& skeleton, SkeletonData& skeletonData, glm::mat4 parentBindPose)
	{
		Joint tSkel;
		tSkel.m_ParentID = skeletonData.m_ParentID;
		tSkel.m_Transform = glm::mat4(1.0f);

		glm::mat4 tempBind = parentBindPose * skeletonData.m_BindPose;
		tSkel.m_InvBindPose = glm::inverse(tempBind);
		skeleton.push_back(tSkel);

		for (size_t i = 0; i < skeletonData.m_Childrens.size(); ++i)
		{
			FillSkeleton(skeleton, skeletonData.m_Childrens[i], tempBind);
		}
	}

	void AnimatedModel::Init(const char* fileName)
	{
		ColladaParser parser;
		//SkeletonData m_SkeletonData;
		parser.Init(fileName);
		std::vector<float> vertices;
		std::vector<unsigned short> indices;
		parser.GetMeshData(vertices, indices, m_SkeletonData);
		m_Animation = parser.GetAnimation(m_SkeletonData);
		m_Mesh.Init(vertices, indices);
		FillSkeleton(m_Skeleton, m_SkeletonData, glm::mat4(1.0f));
	}

	glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t) {
		return (1.0f - t) * a + t * b;
	}

	//static int iidd = 0;
	void AnimatedModel::UpdateSkeleton(SkeletonData& skeletonData, size_t j, const float progressionTime)
	{   //Acá el delta es el factor de interpolación, no es el deltaTime.
		glm::quat interpolationQuat = glm::slerp(m_Animation.m_AnimationBone[0].m_Orient[j], m_Animation.m_AnimationBone[0].m_Orient[j + 1], progressionTime);
		glm::vec3 interpolationPos = glm::mix(glm::vec3(m_Animation.m_AnimationBone[0].m_Translation[j].x, m_Animation.m_AnimationBone[0].m_Translation[j].y, m_Animation.m_AnimationBone[0].m_Translation[j].z),
			glm::vec3(m_Animation.m_AnimationBone[0].m_Translation[j + 1].x, m_Animation.m_AnimationBone[0].m_Translation[j + 1].y, m_Animation.m_AnimationBone[0].m_Translation[j + 1].z),
			progressionTime);

		//interpolationQuat = glm::normalize(interpolationQuat);
		glm::mat4 finalFrame = glm::toMat4(interpolationQuat);
		finalFrame[3].x = interpolationPos.x;
		finalFrame[3].y = interpolationPos.y;
		finalFrame[3].z = interpolationPos.z;

		m_Skeleton[0].m_Transform = finalFrame;

		for (size_t i = 1; i < m_Animation.m_BonesAnimated; ++i)
		{
			unsigned short ID = m_Animation.m_AnimationBone[i].m_ID;
			interpolationQuat = glm::slerp(m_Animation.m_AnimationBone[i].m_Orient[j], m_Animation.m_AnimationBone[i].m_Orient[j + 1], progressionTime);
			interpolationPos = glm::mix(glm::vec3(m_Animation.m_AnimationBone[i].m_Translation[j].x, m_Animation.m_AnimationBone[i].m_Translation[j].y, m_Animation.m_AnimationBone[i].m_Translation[j].z),
												   glm::vec3(m_Animation.m_AnimationBone[i].m_Translation[j + 1].x, m_Animation.m_AnimationBone[i].m_Translation[j + 1].y, m_Animation.m_AnimationBone[i].m_Translation[j + 1].z),
													progressionTime);

			//interpolationQuat = glm::normalize(interpolationQuat);
			finalFrame = glm::toMat4(interpolationQuat);
			finalFrame[3].x = interpolationPos.x;
			finalFrame[3].y = interpolationPos.y;
			finalFrame[3].z = interpolationPos.z;
			
			m_Skeleton[ID].m_Transform = m_Skeleton[m_Skeleton[ID].m_ParentID].m_Transform * finalFrame;
		}
	}

	static float m_fAnimTime = 0.0f;
	static int frame0 = 0;
	void AnimatedModel::Update(const float delta)
	{
		if ((m_fAnimTime - (float)frame0) > 1.0f) {
			frame0++;
			if (frame0 == m_Animation.m_AnimationBone[0].m_TransformsCount - 1)
			{
				frame0 = 0;
				m_fAnimTime = 0.0f;
			}
		}
		
		m_fAnimTime += delta / (float)m_Animation.m_AnimationBone[0].m_TransformsCount;
		float progressionTime = m_fAnimTime - (float)frame0;
		UpdateSkeleton(m_SkeletonData, frame0, progressionTime);
	}

	void GetJoints(Skeleton& skeleton, SkeletonData& skeletonData, std::vector<glm::mat4>& matrix)
	{
		for (size_t i = 0; i < skeleton.size(); ++i)
		{
			matrix[i] = skeleton[i].m_Transform * skeleton[i].m_InvBindPose;
		}
	}

	std::vector<glm::mat4> AnimatedModel::GetJointsUpdates()
	{
		std::vector<glm::mat4> mat;
		mat.resize(50);
		GetJoints(m_Skeleton, m_SkeletonData, mat);
		return mat;
	}
}