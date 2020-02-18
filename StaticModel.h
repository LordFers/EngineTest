#pragma once

#include "Model.h"

namespace Engine {
	class StaticModel : public Model {
	public:
		StaticModel();
		~StaticModel();

	public:
		virtual void Init(const char* fileName);
		virtual inline const Mesh& GetMesh() const override { return m_Mesh; }

	protected:
		Mesh m_Mesh;
	};
}