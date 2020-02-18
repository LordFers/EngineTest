#include "ColladaParser.h"
#include "AnimatedModel.h"
#include "StaticModel.h"
#include "DisplayManager.h"
#include "Camera.h"
#include "Shader.h"
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine {
	class Game : public DisplayListener {
	private:
		Camera* m_Camera;
		Shader* m_Shader;
		GLuint vertexbuffer;
		GLuint VertexArrayID;
		glm::mat4 Projection;
		glm::mat4 View;
		glm::mat4 MVP;

		AnimatedModel m_Model;

		GLuint m_TextureID;

	public:
		virtual void OnCreate()
		{
			/*ColladaParser fileDae;
			fileDae.Init("Resources/personaje.xml");

			auto data = fileDae.getIndexData();
			for (auto value : data)
			{
				printf("%i\n", value);
			}*/

			/******************************************/

			glEnable(GL_CULL_FACE);
			
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //GL_FILL

			m_Camera = new Camera();
			m_Shader = new Shader("Resources/default.vs", "Resources/default.fs");
			m_Model.Init("Resources/model.dae");

			glGenTextures(1, &m_TextureID);
			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			int width, height;
			unsigned char* image = stbi_load("Resources/diffuse.png", &width, &height, nullptr, STBI_rgb_alpha);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

			stbi_image_free(image);

			//glGenVertexArrays(1, &VertexArrayID);
			//glBindVertexArray(VertexArrayID);

			glEnable(GL_ALPHA_TEST);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
			
			Projection = glm::perspectiveFovRH(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f);
			// Camera matrix
			View = glm::lookAt(
				glm::vec3(4, 5, -10), // Camera is at (4,3,-3), in World Space
				glm::vec3(0, 0, 0), // and looks at the origin
				glm::vec3(0, 1, 0));

			MVP = Projection * View * glm::mat4(1.0f);
			m_Shader->use();
		}

		glm::mat4 transform(float m[])
		{
			glm::mat4 mat;
			int k = 0;
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					mat[x][y] = m[k++];
				}
			}
			
			return mat;
		}

		virtual void OnRender(const float delta)
		{
			//glCullFace(GL_FRONT);
			m_Camera->Update();
			m_Model.Update(delta);

			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			auto mesh = m_Model.GetMesh();
			glBindBuffer(GL_ARRAY_BUFFER, mesh.GetVBO());

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), nullptr);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(11 * sizeof(GLfloat)));
			
			m_Shader->setMat4("View", m_Camera->getView());
			m_Shader->setMat4("Projection", m_Camera->getProjection());

			static float angle = -90.0f;//-90.0f;
			//angle += 0.1f;
			m_Shader->setMat4("Model", glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1, 0, 0)));

			auto value = m_Model.GetJointsUpdates();
			
			m_Shader->setMat4Array("Joints", value);
			m_Shader->setTexture0("Texture", m_TextureID);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.GetIBO());
			glDrawElements(GL_TRIANGLES, mesh.GetCountIndices(), GL_UNSIGNED_SHORT, nullptr);

			glDisableVertexAttribArray(4);
			glDisableVertexAttribArray(3);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);

			auto err = glGetError();

			if (err != GL_NO_ERROR)
				printf("OpenGL Error: [%u]\n", err);

		}

		virtual void OnResize(const int width, const int height) override
		{

		}

		virtual void OnMouseEvent() override
		{

		}

		virtual void OnDelete()
		{
			delete m_Shader;
		}
	};
}

using namespace Engine;

int main(int argc, char* argv[])
{
	new DisplayManager(new Game(), "Game Engine", 1280, 720, NULL);
	return (1);
}