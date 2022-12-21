#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"
#include <string>
#include <vector>
#include <cmath>

#define MAX_ITERATIONS 8

struct FractalParams {
	int scene;
	int iteration;

	bool isDifferent(FractalParams other) {
		return scene != other.scene || iteration != other.iteration;
	}
};

class FractalCallbacks : public CallbackInterface {

public:
	FractalCallbacks(FractalParams& fp) : fp(fp) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (key == GLFW_KEY_UP) {
				fp.iteration += 1;
			}
			else if (key == GLFW_KEY_DOWN) {
				fp.iteration -= 1;
			}
			else if (key == GLFW_KEY_RIGHT) {
				fp.iteration += 1;
			}
			else if (key == GLFW_KEY_LEFT) {
				fp.iteration -= 1;
			}

			fp.iteration = std::clamp(fp.iteration, 0, MAX_ITERATIONS);

			if (key == GLFW_KEY_1) {
				fp.scene = 1;
				fp.iteration = 0;
			}
			else if (key == GLFW_KEY_2) {
				fp.scene = 2;
				fp.iteration = 0;
			}
			else if (key == GLFW_KEY_3) {
				fp.scene = 3;
				fp.iteration = 0;
			}
			else if (key == GLFW_KEY_4) {
				fp.scene = 4;
				fp.iteration = 0;
			}
		}
	}

	FractalParams getParams() {
		return fp;
	}

private:
	FractalParams fp;
};

void sierpinskiTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, int m, CPU_Geometry& cpuGeom, glm::vec3 assignedColor) {
	if (m > 0) {
		glm::vec3 d = 0.5f * a + 0.5f * b;
		glm::vec3 e = 0.5f * b + 0.5f * c;
		glm::vec3 f = 0.5f * c + 0.5f * a;

		sierpinskiTriangle(a, d, f, m - 1, cpuGeom, glm::vec3(1.f, 0.f, 0.f));
		sierpinskiTriangle(d, b, e, m - 1, cpuGeom, glm::vec3(0.f, 1.f, 0.f));
		sierpinskiTriangle(f, e, c, m - 1, cpuGeom, glm::vec3(0.f, 0.f, 1.f));
	}
	else {
		cpuGeom.verts.push_back(a);
		cpuGeom.verts.push_back(b);
		cpuGeom.verts.push_back(c);

		cpuGeom.cols.push_back(assignedColor);
		cpuGeom.cols.push_back(assignedColor);
		cpuGeom.cols.push_back(assignedColor);
	}
}

void uniformMassTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, int m, CPU_Geometry& cpuGeom) {
	if (m > 0) {
		glm::vec3 d = (a + b + c) / 3.f;

		uniformMassTriangle(a, b, d, m - 1, cpuGeom);
		uniformMassTriangle(b, c, d, m - 1, cpuGeom);
		uniformMassTriangle(c, a, d, m - 1, cpuGeom);
	}
	else {
		cpuGeom.verts.push_back(a);
		cpuGeom.verts.push_back(b);

		cpuGeom.verts.push_back(b);
		cpuGeom.verts.push_back(c);

		cpuGeom.verts.push_back(c);
		cpuGeom.verts.push_back(a);

		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
	}
}

void kochSnowflake(glm::vec3 a, glm::vec3 b, int m, CPU_Geometry& cpuGeom) {
	if (m > 0) {
		glm::vec3 c = 2 / 3.f * a + 1 / 3.f * b;
		glm::vec3 e = 1 / 3.f * a + 2 / 3.f * b;

		glm::mat3 rotate60(glm::vec3(0.5f, sin(glm::radians(60.f)), 0), glm::vec3(-sin(glm::radians(60.f)), 0.5f, 0), glm::vec3(0, 0, 1));
		glm::vec3 temp = c - a;
		glm::vec3 d = a + temp + temp * rotate60;

		kochSnowflake(a, c, m - 1, cpuGeom);
		kochSnowflake(c, d, m - 1, cpuGeom);
		kochSnowflake(d, e, m - 1, cpuGeom); 
		kochSnowflake(e, b, m - 1, cpuGeom);
	}
	else {
		cpuGeom.verts.push_back(a);
		cpuGeom.verts.push_back(b);

		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
		cpuGeom.cols.push_back(glm::vec3(1.f, 1.f, 1.f));
	}
}

glm::vec3 getNextColor(int iteration) {
	// cycling between 4 colors (red, green, blue, yellow) based on iteration
	if (iteration % 4 == 0)
		return glm::vec3(1, 0, 0); // Red
	else if (iteration % 4 == 1)
		return glm::vec3(0, 1, 0); // Green
	else if (iteration % 4 == 2)
		return glm::vec3(0, 0, 1); // Blue
	else
		return glm::vec3(1, 1, 0); // Yellow
}

void dragonCurve(glm::vec3 a, glm::vec3 b, int m, CPU_Geometry& cpuGeom, glm::vec3 assignedColor, bool keepColor) {
	if (m > 0) {
		glm::vec3 mid = 1 / 2.f * a + 1 / 2.f * b;
		glm::mat3 rightRotate(glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 0));
		glm::vec3 c = mid + (mid - a) * rightRotate;

		if (keepColor) {
			dragonCurve(a, c, m - 1, cpuGeom, assignedColor, true);
			dragonCurve(b, c, m - 1, cpuGeom, assignedColor, true);
		}
		else {
			dragonCurve(a, c, m - 1, cpuGeom, getNextColor(m-1), false);
			dragonCurve(b, c, m - 1, cpuGeom, assignedColor, true);
		}
	}
	else {
		cpuGeom.verts.push_back(a);
		cpuGeom.verts.push_back(b);

		cpuGeom.cols.push_back(assignedColor);
		cpuGeom.cols.push_back(assignedColor);
	}
}

CPU_Geometry generateFractalVerts(FractalParams fp) {
	glm::vec3 x, y, z;
	CPU_Geometry cpuGeom;

	switch (fp.scene) {
	case 1:
		x = glm::vec3(-0.866f, -0.5f, 0.f); // bottom left
		y = glm::vec3(0.866f, -0.5f, 0.f); // bottom right
		z = glm::vec3(0.f, 1.f, 0.f); // top
		sierpinskiTriangle(x, y, z, fp.iteration, cpuGeom, glm::vec3(1.f, 0.f, 0.f));
		break;
	case 2:
		x = glm::vec3(-0.866f, -0.5f, 0.f); // bottom left
		y = glm::vec3(0.866f, -0.5f, 0.f); // bottom right
		z = glm::vec3(0.f, 1.f, 0.f); // top
		uniformMassTriangle(x, y, z, fp.iteration, cpuGeom);
		break;
	case 3:
		x = glm::vec3(-0.866f, -0.5f, 0.f); // bottom left
		y = glm::vec3(0.866f, -0.5f, 0.f); // bottom right
		z = glm::vec3(0.f, 1.f, 0.f); // top
		kochSnowflake(x, y, fp.iteration, cpuGeom);
		kochSnowflake(y, z, fp.iteration, cpuGeom);
		kochSnowflake(z, x, fp.iteration, cpuGeom);
		break;
	case 4:
		x = glm::vec3(-0.5f, 0.f, 0.f); // left
		y = glm::vec3(0.5f, 0.f, 0.5f); // right
		dragonCurve(x, y, fp.iteration, cpuGeom, getNextColor(fp.iteration), false);
		break;
	}

	return cpuGeom;
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "Fractal Generator");

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	FractalParams fp{ 1, 0 };

	// CALLBACKS
	auto callbacks = std::make_shared<FractalCallbacks>(fp);
	window.setCallbacks(callbacks);

	// GEOMETRY
	CPU_Geometry cpuGeom = generateFractalVerts(fp);
	GPU_Geometry gpuGeom;

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		FractalParams newParams = callbacks->getParams();
		if (newParams.isDifferent(fp)) {
			fp = newParams;
			cpuGeom = generateFractalVerts(fp);

			gpuGeom.setVerts(cpuGeom.verts);
			gpuGeom.setCols(cpuGeom.cols);
		}

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		switch (fp.scene) {
		case 1:
			glDrawArrays(GL_TRIANGLES, 0, cpuGeom.verts.size());
			break;
		case 2:
			glDrawArrays(GL_LINES, 0, cpuGeom.verts.size());
			break;
		case 3:
			glDrawArrays(GL_LINES, 0, cpuGeom.verts.size());
			break;
		case 4:
			glDrawArrays(GL_LINES, 0, cpuGeom.verts.size());
			break;
		}
		glDisable(GL_FRAMEBUFFER_SRGB);

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
