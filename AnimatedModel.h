#pragma once

#include "Model.h"
#include <string>
#include <glm/glm.hpp>

namespace Engine {
	typedef struct _JointData {
		unsigned short m_ID;
		unsigned short m_ParentID;
		const char* m_IDName;
		std::vector<_JointData> m_Childrens;
		glm::mat4 m_BindPose;
		glm::mat4 m_InvBindPose;
		//glm::mat4 m_AnimatedTransform; //delete
	} SkeletonData;

	typedef struct {
		unsigned short m_ParentID;
		glm::mat4 m_InvBindPose;
		glm::mat4 m_Transform;
	} Joint;

	typedef std::vector<Joint> Skeleton;

	typedef struct {
		unsigned short m_ID;
		size_t m_TransformsCount; //frames.
		//glm::mat4* m_Transforms; //frame transform.
		glm::quat* m_Orient;
		glm::vec3* m_Translation;
	} AnimationBone;

	typedef struct {
		size_t m_BonesAnimated;
		AnimationBone* m_AnimationBone;
	} Animation;

	class AnimatedModel : public Model {
	public:
		AnimatedModel();
		~AnimatedModel();

	public:
		virtual void Init(const char* fileName);
		virtual inline const Mesh& GetMesh() const override { return m_Mesh; }

	public:
		inline const Skeleton& GetSkeleton() const { return m_Skeleton; }
		std::vector<glm::mat4> GetJointsUpdates();
		void Update(const float delta);
		void UpdateSkeleton(SkeletonData& skeletonData, size_t j, const float progressionTime);

	protected:
		Mesh m_Mesh;
		Skeleton m_Skeleton;
		SkeletonData m_SkeletonData;
		Animation m_Animation;
	};
}