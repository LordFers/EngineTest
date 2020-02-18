#include "DisplayManager.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

namespace Engine {
	DisplayManager::DisplayManager(DisplayListener* display, const char* title, int width, int height, char mode)
	{
		if (!glfwInit())
			exit(-1);

		glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // Usamos OpenGL 2.0 para tener retrocompatibilidad con drivers, si usamos la 3.3, vamos a tener que generar VAOs cuando no sean necesarios realmente.
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwSwapInterval(1);

		m_Window = glfwCreateWindow(width, height, title, NULL, NULL); //<- mode mask.

		if (!m_Window)
		{
			glfwTerminate();
			exit(-1);
		}

		//glfwSetWindowAspectRatio(m_Window, 16, 9);
		//glfwSetWindowUserPointer(m_Window, this);
		glfwMakeContextCurrent(m_Window);

		glewExperimental = true;//Usando OpenGL con compatibilidad hacia atrás de la 3.2, no es necesario el experimental.
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed glew.32.dll.\n");
			exit(-1);
		}

		display->OnCreate();

		double deltaTime = 0.0f;
		double lastFrame = 0.0f;
		double currentFrame = 0.0f;

		while (!glfwWindowShouldClose(m_Window))
		{
			currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			deltaTime = 0.1666666f;
			//printf("%f\n", deltaTime);
			display->OnRender((float)deltaTime);

			/* Swap front and back buffers */
			glfwSwapBuffers(m_Window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		display->OnDelete();
		glfwTerminate();
	}

	DisplayManager::~DisplayManager()
	{
		//TODO:
	}
}