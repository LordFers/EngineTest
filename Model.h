#pragma once

#include "Mesh.h"

namespace Engine {
	class Model {
	public:
		virtual ~Model() = default;
		virtual void Init(const char* fileName) = 0;
		virtual inline const Mesh& GetMesh() const = 0;
	};
}