#pragma once

namespace Engine {
	class DisplayListener {
	public:
		virtual ~DisplayListener() = default;

	public:
		virtual void OnCreate() = 0;
		virtual void OnDelete() = 0;
		virtual void OnRender(const float delta) = 0;
		virtual void OnResize(const int width, const int height) = 0;
		virtual void OnMouseEvent() = 0;
	};
}