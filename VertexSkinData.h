#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <math.h>

namespace Engine {
	class VertexSkinData {
	public:
		VertexSkinData()
		{

		}

		void addJointEffect(int jointId, float weight) {
			for (size_t i = 0; i < weights.size(); i++) {
				if (weight > weights.at(i)) {
					std::vector<float>::iterator itA;
					std::vector<int>::iterator itB;
					itA = weights.begin() + i;
					itB = jointIds.begin() + i;
					weights.insert(itA, weight);
					jointIds.insert(itB, jointId);
					//jointIds.at(i) = jointId;
					//weights.at(i) = weight;
					return;
				}
			}
			jointIds.push_back(jointId);
			weights.push_back(weight);
		}

		void limitJointNumber(int max)
		{
			float* topWeights = new float[max];
			for (size_t i = 0; i < (size_t)max; ++i)
			{
				topWeights[i] = 0.0f;
			}

			if (jointIds.size() > (size_t)max)
			{	
				float total = saveTopWeights(topWeights);
				refillWeightList(topWeights, total);
				removeExcessJointIds(max);
			}
			else if (jointIds.size() < (size_t)max) {
				fillEmptyWeights(max);
			}

			delete [] topWeights;
		}

		void fillEmptyWeights(int max) {
			while (jointIds.size() < (size_t)max) {
				jointIds.push_back(0);
				weights.push_back(0.0f);
			}
		}

		float saveTopWeights(float* topWeightsArray) {
			float total = 0;
			for (int i = 0; i < 3; i++) {
				topWeightsArray[i] = weights.at(i);
				total += topWeightsArray[i];
			}
			return total;
		}

		void refillWeightList(float* topWeights, float total) {
			weights.clear();
			for (int i = 0; i < 3; i++) {
				weights.push_back((float)std::fmin(topWeights[i] / total, 1));
			}
		}

		void removeExcessJointIds(int max) {
			while (jointIds.size() > (size_t)max) {
				jointIds.pop_back();
			}
		}

	public:
		std::vector<int> jointIds;
		std::vector<float> weights;
	};
}