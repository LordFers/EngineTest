#pragma once
#include "DisplayListener.h"

typedef struct GLFWwindow GLFWwindow;
namespace Engine {
	class DisplayManager {
	public:
		DisplayManager(DisplayListener* display, const char* title, int width, int height, char mode);
		virtual ~DisplayManager();

	protected:
		GLFWwindow* m_Window;
	};
}