// #include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "UnitSphere.h"
#include "CelestialBody.h"

#define DAYS_PER_SECOND 2.0f

class Callbacks : public CallbackInterface
{

public:
	Callbacks()
		:	camera(glm::radians(0.0f)
		,	glm::radians(0.0f), 10.0)
		,	aspect(1.0f)
		,	rightMouseDown(false)
		,	mouseOldX(0.0)
		,	mouseOldY(0.0)
		,	pause(false)
		,	restart(false)
		,	playbackSpeed(1.0f)
		,	focusedBody(0)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			pause = !pause;
		}

		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			restart = true;
		}

		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			playbackSpeed *= 2.0f;
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			playbackSpeed /= 2.0f;
		}

		if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		{
			focusedBody = 1;
		}
		else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		{
			focusedBody = 2;
		}
		else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		{
			focusedBody = 3;
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if (action == GLFW_PRESS)
				rightMouseDown = true;
			else if (action == GLFW_RELEASE)
				rightMouseDown = false;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos)
	{
		if (rightMouseDown)
		{
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}

	virtual void scrollCallback(double xoffset, double yoffset)
	{
		camera.incrementR(yoffset);
	}

	virtual void windowSizeCallback(int width, int height)
	{
		CallbackInterface::windowSizeCallback(width, height);
		aspect = float(width) / float(height);
	}

	bool shouldPause()
	{
		return pause;
	}

	bool shouldRestart()
	{
		bool result = restart;
		restart = false;
		return result;
	}

	float getPlaybackSpeed()
	{
		return playbackSpeed;
	}

	int getFocusedBody()
	{
		return focusedBody;
	}

	void viewPipeline(ShaderProgram &sp, glm::vec3 lightPosition)
	{
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint location = glGetUniformLocation(sp, "lightPos");
		glm::vec3 light = glm::vec4(lightPosition, 1.0f);
		glUniform3fv(location, 1, glm::value_ptr(light));

		location = glGetUniformLocation(sp, "viewPos");
		glm::vec3 eye = glm::vec4(camera.getPos(), 1.0f);
		glUniform3fv(location, 1, glm::value_ptr(eye));

		GLint uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;

private:
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;

	bool pause;
	bool restart;
	float playbackSpeed;
	int focusedBody;
};

int main()
{
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "Solar System Simulation");

	GLDebug::enable();

	// CALLBACKS
	auto cb = std::make_shared<Callbacks>();
	window.setCallbacks(cb);

	ShaderProgram noLightingShader("shaders/no_lighting.vert", "shaders/no_lighting.frag");
	ShaderProgram lightingShader("shaders/lighting.vert", "shaders/lighting.frag");

	UnitSphere sphere(20);
	sphere.m_gpu_geom.bind();

	CelestialBody skyBox("textures/8k_stars.jpg", GL_NEAREST, nullptr, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	CelestialBody sun("textures/8k_sun.jpg", GL_NEAREST, &skyBox, 1.0f, 0.0f, 0.0f, 6.0f, DAYS_PER_SECOND * 360.0f / 27.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	CelestialBody earth("textures/8k_earth.jpg", GL_NEAREST, &sun, 0.3f, 3.0f, 0.0f, 23.4f, DAYS_PER_SECOND * 360.0f / 1.0f, DAYS_PER_SECOND * 360.0f / 365.0f, 0.85f, 0.7f, 16.0f);
	CelestialBody moon("textures/8k_moon.jpg", GL_NEAREST, &earth, 0.15f, 1.0f, 20.0f, 6.7f, DAYS_PER_SECOND * 360.0f / 27.0f, DAYS_PER_SECOND * 360.0f / 27.0f, 0.7f, 0.5f, 32.0f);

	cb->camera.setFocusBody(&sun);
	int currFocusedBody = 1;

	float prevTime = glfwGetTime();

	// RENDER LOOP
	while (!window.shouldClose())
	{
		glfwPollEvents();

		if (cb->shouldRestart())
		{
			skyBox.reset();
			sun.reset();
			earth.reset();
			moon.reset();
		}

		int newFocusedBody = cb->getFocusedBody();
		if (newFocusedBody != currFocusedBody)
		{
			if (newFocusedBody == 1)
				cb->camera.setFocusBody(&sun);
			else if (newFocusedBody == 2)
				cb->camera.setFocusBody(&earth);
			else if (newFocusedBody == 3)
				cb->camera.setFocusBody(&moon);
			currFocusedBody = newFocusedBody;
		}

		float currTime = glfwGetTime();
		float deltaTime = currTime - prevTime;
		prevTime = currTime;

		deltaTime *= cb->getPlaybackSpeed();
		if (cb->shouldPause())
			deltaTime = 0;

		sun.spin(deltaTime);
		sun.orbit(deltaTime);

		earth.spin(deltaTime);
		earth.orbit(deltaTime);

		moon.spin(deltaTime);
		moon.orbit(deltaTime);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		noLightingShader.use();
		cb->viewPipeline(noLightingShader, sun.getAbsolutePosition());
		skyBox.draw(noLightingShader, sphere.m_size);
		sun.draw(noLightingShader, sphere.m_size);

		lightingShader.use();
		cb->viewPipeline(lightingShader, sun.getAbsolutePosition());
		earth.draw(lightingShader, sphere.m_size);
		moon.draw(lightingShader, sphere.m_size);

		glDisable(GL_FRAMEBUFFER_SRGB);
		window.swapBuffers();
	}
	glfwTerminate();
	return 0;
}
