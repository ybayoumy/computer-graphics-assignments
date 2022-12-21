//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
#include <glad/glad.h>

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

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#define PI 3.14159f
#define MOUSE_SENSITIVITY 10.0f
#define MOVE_SPEED 0.05f

void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

enum MouseAction { noAction, leftPressed, leftReleased, rightPressed, rightReleased };
enum CurveType { bezier, bSpline };

struct Camera {
	glm::vec3 position;
	glm::vec3 lookDirection;
	glm::vec3 upDirection;

	float yaw;
	float pitch;

	glm::mat4 viewMatrix;

	Camera (glm::vec3 pos, glm::vec3 lookP){
		position = pos;
		lookDirection = glm::normalize(lookP - position);
		upDirection = glm::vec3{ 0.0f, 1.0f, 0.0f };
		yaw = -90;
		pitch = 0;
		updateViewMat();
	}

	void setPosition(glm::vec3 newPos, glm::vec3 newLookP) {
		position = newPos;
		lookDirection = glm::normalize(newLookP - position);
		upDirection = glm::vec3{ 0.0f, 1.0f, 0.0f };
		yaw = -90;
		pitch = 0;
		updateViewMat();
	}

	void moveHorizontal(float speed) {
		glm::vec3 rightDirection = glm::normalize(glm::cross(lookDirection, upDirection));
		position += rightDirection * speed;
	}

	void moveForward(float speed) {
		position += lookDirection * speed;
	}

	void rotateCamera(float xDiff, float yDiff) {
		float sensitivity = MOUSE_SENSITIVITY;

		yaw += xDiff * sensitivity;
		pitch += yDiff * sensitivity;
		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		lookDirection = glm::normalize(glm::vec3(
			glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
			glm::sin(glm::radians(pitch)),
			glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
		));
	}

	void updateViewMat() {
		viewMatrix = glm::lookAt(position, position + lookDirection, upDirection);
	}
};

// EXAMPLE CALLBACKS
class A3Callbacks : public CallbackInterface {

public:
	A3Callbacks(int screenWidth, int screenHeight) :
		clearPoints(false),
		bPressed(false),
		nPressed(false),
		tPressed(false),
		onePressed(false),
		twoPressed(false),
		threePressed(false),
		fourPressed(false),
		wPressed(false),
		aPressed(false),
		sPressed(false),
		dPressed(false),
		lastMouseAction(noAction),
		mousePosition(),
		screenDim(screenWidth, screenHeight)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			clearPoints = true;
		} else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
			bPressed = true;
		} else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
			nPressed = true;
		} else if (key == GLFW_KEY_T && action == GLFW_PRESS) {
			tPressed = true;
		} else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			onePressed = true;
		} else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			twoPressed = true;
		} else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
			threePressed = true;
		} else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
			fourPressed = true;
		}

		// Handling movement keys
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			wPressed = true;
		} else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			aPressed = true;
		} else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			sPressed = true;
		} else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			dPressed = true;
		} else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			wPressed = false;
		} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			aPressed = false;
		} else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			sPressed = false;
		} else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			dPressed = false;
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			lastMouseAction = leftPressed;
		} else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			lastMouseAction = leftReleased;
		} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			lastMouseAction = rightPressed;
		} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			lastMouseAction = rightReleased;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		mousePosition.x = (float) xpos;
		mousePosition.y = (float) ypos;
	}

	virtual void windowSizeCallback(int width, int height) {
		CallbackInterface::windowSizeCallback(width,  height);
	}

	glm::vec2 getMouseCoords() {
		glm::vec2 scaledVec = (mousePosition + glm::vec2(0.5f, 0.5f)) / screenDim;
		glm::vec2 flippedY = glm::vec2(scaledVec.x, 1.0 - scaledVec.y);

		return flippedY * 2.0f - glm::vec2(1.0f, 1.0f);
	}

	MouseAction getMouseAction() {
		return lastMouseAction;
	}

	void setMouseAction(MouseAction newAction) {
		lastMouseAction = newAction;
	}

	bool shouldClear() {
		if (clearPoints) {
			clearPoints = false;
			return true;
		}
		return false;
	}

	bool isKeyPressed(int button) {
		bool res;
		if (button == GLFW_KEY_B) {
			res = bPressed;
			bPressed = false;
		} else if (button == GLFW_KEY_N) {
			res = nPressed;
			nPressed = false;
		} else if (button == GLFW_KEY_T) {
			res = tPressed;
			tPressed = false;
		} else if (button == GLFW_KEY_1) {
			res = onePressed;
			onePressed = false;
		}
		else if (button == GLFW_KEY_2) {
			res = twoPressed;
			twoPressed = false;
		}
		else if (button == GLFW_KEY_3) {
			res = threePressed;
			threePressed = false;
		} else if (button == GLFW_KEY_4) {
			res = fourPressed;
			fourPressed = false;
		} else if (button == GLFW_KEY_W) {
			res = wPressed;
		} else if (button == GLFW_KEY_A) {
			res = aPressed;
		} else if (button == GLFW_KEY_S) {
			res = sPressed;
		} else if (button == GLFW_KEY_D) {
			res = dPressed;
		} else {
			res = false;
		}
		return res;
	}

private:
	bool clearPoints;
	bool bPressed;
	bool nPressed;
	bool tPressed;
	bool onePressed;
	bool twoPressed;
	bool threePressed;
	bool fourPressed;
	bool wPressed;
	bool aPressed;
	bool sPressed;
	bool dPressed;
	MouseAction lastMouseAction;
	glm::vec2 mousePosition;
	glm::vec2 screenDim;
};

int findPoint(std::vector<glm::vec3>& points, glm::vec2 mousePos, float pointWidth) {
	for (int i = 0; i < points.size(); i++) {
		float distance = glm::distance(glm::vec2(points[i]), mousePos);
		if (distance < pointWidth)
			return i;
	}
	return -1;
}

void calculateBezier(std::vector<glm::vec3> & control_points, int granularity, CPU_Geometry& result) {
	float uStep = 1.0f / granularity;

	int num_points = control_points.size();
	if (num_points < 2) {
		result.verts.clear();
		result.cols.clear();
		return;
	}

	result.verts = std::vector<glm::vec3>(granularity + 1);
	result.cols.resize(granularity + 1, glm::vec3{ 0.0, 0.0, 1.0 });

	for (int k = 0; k <= granularity; k++) {
		float u = k * uStep;
		std::vector<glm::vec3> points = control_points;

		for (int i = 1; i < num_points; i++) {
			for (int j = 0; j < num_points - i; j++) {
				points[j] = (1 - u) * points[j] + u * points[j + 1];
			}
		}

		result.verts[k] = points[0];
	}
}

void calculateBSpline(std::vector<glm::vec3>& control_points, int iterations, CPU_Geometry& result) {
	result.verts = control_points;

	int num_points = control_points.size();
	if (num_points < 2) {
		result.verts.clear();
		result.cols.clear();
		return;
	}
	else if (num_points == 2) {
		result.cols.resize(result.verts.size(), glm::vec3{ 0.0f, 0.0f, 1.0f });
		return;
	}
	
	for (int i = 0; i < iterations; i++) {
		int newPointsSize = 2 * num_points - 2;
		std::vector<glm::vec3> new_points(newPointsSize);

		new_points[0] = result.verts[0];
		new_points[1] = 0.5f * result.verts[0] + 0.5f * result.verts[1];
		for (int j = 1; j < num_points - 2; j++) {
			new_points[2 * j] = 0.75f * result.verts[j] + 0.25f * result.verts[j + 1];
			new_points[2 * j + 1] = 0.25f * result.verts[j] + 0.75f * result.verts[j + 1];
		}
		new_points[new_points.size() - 2] = 0.5f * result.verts[num_points - 1] + 0.5f * result.verts[num_points - 2];
		new_points[new_points.size() - 1] = result.verts[num_points - 1];

		result.verts = new_points;
		num_points = new_points.size();
	}

	result.cols.resize(result.verts.size(), glm::vec3{ 0.0f, 0.0f, 1.0f });
}

void calculateCurve(std::vector<glm::vec3>& control_points, CPU_Geometry& result, CurveType curveType) {
	if (curveType == bezier) {
		calculateBezier(control_points, 50, result);
	}
	else if (curveType == bSpline) {
		calculateBSpline(control_points, 4, result);
	}
}

void calculateSurfaceOfRevolution(std::vector<glm::vec3>& points, int granularity, CPU_Geometry& result) {
	float vStep = 2.0f * PI / granularity;
	int numPoints = points.size();

	std::vector<glm::vec3>& resultVerts = result.verts;
	std::vector<std::vector<glm::vec3>> surface(granularity + 1, std::vector<glm::vec3>(numPoints));
	surface[0] = points;

	for (int k = 1; k <= granularity; k++) {
		float v = vStep * k;

		for (int i = 0; i < numPoints; i++) {
			glm::vec3& basePoint = points[i];
			surface[k][i] = glm::vec3(
				basePoint.x * glm::cos(v),
				basePoint.y,
				basePoint.x * glm::sin(v)
			);

			if (i != 0) {
				resultVerts.push_back(surface[k][i]);
				resultVerts.push_back(surface[k - 1][i]);
				resultVerts.push_back(surface[k][i - 1]);
			}

			if (i != numPoints - 1) {
				resultVerts.push_back(surface[k][i]);
				resultVerts.push_back(surface[k - 1][i + 1]);
				resultVerts.push_back(surface[k - 1][i]);
			}
		}
	}

	result.cols.resize(result.verts.size(), glm::vec3{ 0.0f, 0.0f, 1.0f });
}

void calculateTensorProductSurface(std::vector<std::vector<glm::vec3>>& control_points, CPU_Geometry& result) {
	int uSize = control_points.size();
	if (uSize == 0) return;

	std::vector<CPU_Geometry> intermediateCurves(uSize);
	for (int i = 0; i < uSize; i++) {
		calculateCurve(control_points[i], intermediateCurves[i], bSpline);
	}

	int numIntermediatePoints = intermediateCurves[0].verts.size();
	std::vector<std::vector<glm::vec3>> surface(numIntermediatePoints);

	for (int j = 0; j < numIntermediatePoints; j++) {
		std::vector<glm::vec3> intermediateControlPoints;
		for (int i = 0; i < uSize; i++) {
			intermediateControlPoints.push_back(intermediateCurves[i].verts[j]);
		}

		CPU_Geometry temp;
		calculateCurve(intermediateControlPoints, temp, bSpline);
		surface[j] = temp.verts;
	}

	std::vector<glm::vec3>& resultVerts = result.verts;

	for (int i = 1; i < numIntermediatePoints; i++) {
		for (int j = 0; j < surface[i].size(); j++) {
			if (j != 0) {
				resultVerts.push_back(surface[i][j]);
				resultVerts.push_back(surface[i - 1][j]);
				resultVerts.push_back(surface[i][j - 1]);
			}

			if (j != surface[i].size() - 1) {
				resultVerts.push_back(surface[i][j]);
				resultVerts.push_back(surface[i - 1][j + 1]);
				resultVerts.push_back(surface[i - 1][j]);
			}
		}
	}

	result.cols.resize(result.verts.size(), glm::vec3{ 0.0f, 0.0f, 1.0f });
}

std::vector<std::vector<glm::vec3>> genCustomTensorSurfaceCP() {
	std::vector<std::vector<glm::vec3>> result(3, std::vector<glm::vec3>(4));

	result[0][0] = glm::vec3{ -2.0f, 4.0f, 1.0f };
	result[0][1] = glm::vec3{ -1.0f, 0.0f, 1.0f };
	result[0][2] = glm::vec3{ 0.0f, 0.0f, 1.0f };
	result[0][3] = glm::vec3{ 1.0f, 4.0f, 1.0f };

	result[1][0] = glm::vec3{ -2.0f, 0.0f, 0.0f };
	result[1][1] = glm::vec3{ -1.0f, 0.0f, 0.0f };
	result[1][2] = glm::vec3{ 0.0f, 0.0f, 0.0f };
	result[1][3] = glm::vec3{ 1.0f, 0.0f, 0.0f };

	result[2][0] = glm::vec3{ -2.0f, 4.0f, -1.0f };
	result[2][1] = glm::vec3{ -1.0f, 0.0f, -1.0f };
	result[2][2] = glm::vec3{ 0.0f, 0.0f, -1.0f };
	result[2][3] = glm::vec3{ 1.0f, 4.0f, -1.0f };

	return result;
}

std::vector<std::vector<glm::vec3>> genPredefinedTensorSurfaceCP() {
	std::vector<std::vector<glm::vec3>> result(5, std::vector<glm::vec3>(5));

	result[0][0] = glm::vec3{-2.0f, 0.0f, 2.0f};
	result[0][1] = glm::vec3{ -1.0f, 0.0f, 2.0f };
	result[0][2] = glm::vec3{ 0.0f, 0.0f, 2.0f };
	result[0][3] = glm::vec3{ 1.0f, 0.0f, 2.0f };
	result[0][4] = glm::vec3{ 2.0f, 0.0f, 2.0f };

	result[1][0] = glm::vec3{ -2.0f, 0.0f, 1.0f };
	result[1][1] = glm::vec3{ -2.0f, 1.0f, 1.0f };
	result[1][2] = glm::vec3{ 0.0f, 1.0f, 1.0f };
	result[1][3] = glm::vec3{ 1.0f, 1.0f, 1.0f };
	result[1][4] = glm::vec3{ 2.0f, 0.0f, 1.0f };

	result[2][0] = glm::vec3{ -2.0f, 0.0f, 0.0f };
	result[2][1] = glm::vec3{ -1.0f, 1.0f, 0.0f };
	result[2][2] = glm::vec3{ 0.0f, -1.0f, 0.0f };
	result[2][3] = glm::vec3{ 1.0f, 1.0f, 0.0f };
	result[2][4] = glm::vec3{ 2.0f, 0.0f, 0.0f };

	result[3][0] = glm::vec3{ -2.0f, 0.0f, -1.0f };
	result[3][1] = glm::vec3{ -1.0f, 1.0f, -1.0f };
	result[3][2] = glm::vec3{ 0.0f, 1.0f, -1.0f };
	result[3][3] = glm::vec3{ 1.0f, 1.0f, -1.0f };
	result[3][4] = glm::vec3{ 2.0f, 0.0f, -1.0f };

	result[4][0] = glm::vec3{ -2.0f, 0.0f, -2.0f };
	result[4][1] = glm::vec3{ -1.0f, 0.0f, -2.0f };
	result[4][2] = glm::vec3{ 0.0f, 0.0f, -2.0f };
	result[4][3] = glm::vec3{ 1.0f, 0.0f, -2.0f };
	result[4][4] = glm::vec3{ 2.0f, 0.0f, -2.0f };

	return result;
}

int main() {
	Log::debug("Starting main");

	int screenRes = 800;
	float pointSize = 10.0f;

	Camera cm(glm::vec3{ 0.0f, 0.0f, 3.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });

	// WINDOW
	glfwInit();
	Window window(screenRes, screenRes, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// CALLBACKS
	auto callbacks = std::make_shared<A3Callbacks>(screenRes, screenRes);
	window.setCallbacks(callbacks);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	CPU_Geometry controlPoints;
	GPU_Geometry pointsGPUGeom;
	CPU_Geometry curvePoints;
	GPU_Geometry curveGPUGeom;
	CPU_Geometry surfacePoints;
	GPU_Geometry surfaceGPUGeom;
	CPU_Geometry tensorControlPoints;
	GPU_Geometry tensorGPUGeom;

	int selectedPointIndex = -1;
	glm::vec2 prevMousePos = glm::vec2(0.0f, 0.0f);

	CurveType curveType = bezier;
	int viewType = 1;
	bool isWireframe = false;
	bool isCustomTensorSurface = false;

	controlPoints.verts.push_back(glm::vec3{ -0.5, 0.5, 0 });
	controlPoints.verts.push_back(glm::vec3{ -0.5, -0.5, 0 });
	controlPoints.verts.push_back(glm::vec3{ 0.5, -0.5, 0 });
	controlPoints.verts.push_back(glm::vec3{ 0.5, 0.5, 0 });
	controlPoints.cols.resize(controlPoints.verts.size(), glm::vec3{1.0, 0.0, 0.0});
	updateGPUGeometry(pointsGPUGeom, controlPoints);

	calculateCurve(controlPoints.verts, curvePoints, curveType);
	updateGPUGeometry(curveGPUGeom, curvePoints);

	glPointSize(pointSize);
	shader.use();

	glm::mat4 identity(1.0f);
	glm::mat4 perspectiveProj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

	float* view = &identity[0][0];
	float* projection = &identity[0][0];

	GLint viewLocation = glGetUniformLocation(shader.getProgram(), "view");
	GLint projectionLocation = glGetUniformLocation(shader.getProgram(), "projection");

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		// Changing between views
		if (callbacks->isKeyPressed(GLFW_KEY_1)) {
			viewType = 1;
			view = &identity[0][0];
			projection = &identity[0][0];

			window.unlockCursor();
		} else if (callbacks->isKeyPressed(GLFW_KEY_2)) {
			viewType = 2;
			cm.setPosition(glm::vec3{ 0.0f, 0.0f, 3.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });
			view = &cm.viewMatrix[0][0];
			projection = &perspectiveProj[0][0];

			prevMousePos = callbacks->getMouseCoords();
			window.lockCursor();
		}
		else if (callbacks->isKeyPressed(GLFW_KEY_3)) {
			viewType = 3;
			cm.setPosition(glm::vec3{ 0.0f, 0.0f, 3.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });
			view = &cm.viewMatrix[0][0];
			projection = &perspectiveProj[0][0];
			if (curveType == bezier) {
				curveType = bSpline;
				calculateCurve(controlPoints.verts, curvePoints, curveType);
				updateGPUGeometry(curveGPUGeom, curvePoints);
			}

			surfacePoints = CPU_Geometry();
			calculateSurfaceOfRevolution(curvePoints.verts, 50, surfacePoints);
			updateGPUGeometry(surfaceGPUGeom, surfacePoints);

			prevMousePos = callbacks->getMouseCoords();
			window.lockCursor();
		}
		else if (callbacks->isKeyPressed(GLFW_KEY_4)) {
			viewType = 4;
			cm.setPosition(glm::vec3{ 0.0f, 0.0f, 3.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });
			view = &cm.viewMatrix[0][0];
			projection = &perspectiveProj[0][0];

			std::vector<std::vector<glm::vec3>> control_points;
			if (isCustomTensorSurface)
				control_points = genCustomTensorSurfaceCP();
			else
				control_points = genPredefinedTensorSurfaceCP();

			surfacePoints = CPU_Geometry();
			calculateTensorProductSurface(control_points, surfacePoints);
			updateGPUGeometry(surfaceGPUGeom, surfacePoints);

			for (int i = 0; i < control_points.size(); i++) {
				for (int j = 0; j < control_points[i].size(); j++) {
					tensorControlPoints.verts.push_back(control_points[i][j]);
				}
			}
			tensorControlPoints.cols.resize(tensorControlPoints.verts.size(), glm::vec3{ 1.0f, 0.0f, 0.0f });
			updateGPUGeometry(tensorGPUGeom, tensorControlPoints);

			prevMousePos = callbacks->getMouseCoords();
			window.lockCursor();
		}

		// Toggle between wireframe and fill
		if (callbacks->isKeyPressed(GLFW_KEY_N))
			isWireframe = !isWireframe;

		// Toggle between tensor surfaces only if viewType is 4
		if (callbacks->isKeyPressed(GLFW_KEY_T) && viewType == 4) {
			isCustomTensorSurface = !isCustomTensorSurface;

			std::vector<std::vector<glm::vec3>> control_points;
			if (isCustomTensorSurface)
				control_points = genCustomTensorSurfaceCP();
			else
				control_points = genPredefinedTensorSurfaceCP();

			surfacePoints = CPU_Geometry();
			calculateTensorProductSurface(control_points, surfacePoints);
			updateGPUGeometry(surfaceGPUGeom, surfacePoints);

			tensorControlPoints = CPU_Geometry();
			for (int i = 0; i < control_points.size(); i++) {
				for (int j = 0; j < control_points[i].size(); j++) {
					tensorControlPoints.verts.push_back(control_points[i][j]);
				}
			}
			tensorControlPoints.cols.resize(tensorControlPoints.verts.size(), glm::vec3{ 1.0f, 0.0f, 0.0f });
			updateGPUGeometry(tensorGPUGeom, tensorControlPoints);

			prevMousePos = callbacks->getMouseCoords();
			window.lockCursor();
		}

		// Toggle between bezier / b-spline
		if (callbacks->isKeyPressed(GLFW_KEY_B) && (viewType == 1 || viewType == 2)) {
			if (curveType == bezier)
				curveType = bSpline;
			else
				curveType = bezier;
			calculateCurve(controlPoints.verts, curvePoints, curveType);
			updateGPUGeometry(curveGPUGeom, curvePoints);
		}

		//check to clear points
		if (callbacks->shouldClear() && viewType == 1) {
			controlPoints = CPU_Geometry();
			updateGPUGeometry(pointsGPUGeom, controlPoints);

			calculateCurve(controlPoints.verts, curvePoints, curveType);
			updateGPUGeometry(curveGPUGeom, curvePoints);
		}

		glm::vec2 mousePos = callbacks->getMouseCoords();

		// if in 2D
		if (viewType == 1) {
			// Handle Mouse Button inputs
			MouseAction mouseAction = callbacks->getMouseAction();
			if (mouseAction == leftPressed) { // selecting a point
				selectedPointIndex = findPoint(controlPoints.verts, mousePos, pointSize / screenRes);
				if (selectedPointIndex != -1) {
					controlPoints.cols[selectedPointIndex] = glm::vec3{ 0.0, 1.0, 0.0 };
				}
			}
			else if (mouseAction == leftReleased) {
				if (selectedPointIndex == -1) { // if no point is selected, add a point
					controlPoints.verts.push_back(glm::vec3(mousePos, 0.0f));
					controlPoints.cols.push_back(glm::vec3{ 1.0, 0.0, 0.0 });
					updateGPUGeometry(pointsGPUGeom, controlPoints);

					calculateCurve(controlPoints.verts, curvePoints, curveType);
					updateGPUGeometry(curveGPUGeom, curvePoints);
				}
				else { // if a point was selected, unselect that point
					controlPoints.cols[selectedPointIndex] = glm::vec3{ 1.0, 0.0, 0.0 };
					selectedPointIndex = -1;
					updateGPUGeometry(pointsGPUGeom, controlPoints);
				}
			}
			else if (mouseAction == rightReleased && selectedPointIndex == -1) { // deleting a point
				int indexToDelete = findPoint(controlPoints.verts, mousePos, pointSize / screenRes);
				if (indexToDelete != -1) {
					controlPoints.verts.erase(controlPoints.verts.begin() + indexToDelete);
					controlPoints.cols.erase(controlPoints.cols.begin() + indexToDelete);
					updateGPUGeometry(pointsGPUGeom, controlPoints);

					calculateCurve(controlPoints.verts, curvePoints, curveType);
					updateGPUGeometry(curveGPUGeom, curvePoints);
				}
			}

			// move selected point
			if (selectedPointIndex != -1) {
				controlPoints.verts[selectedPointIndex] = glm::vec3(mousePos, 0);
				updateGPUGeometry(pointsGPUGeom, controlPoints);

				calculateCurve(controlPoints.verts, curvePoints, curveType);
				updateGPUGeometry(curveGPUGeom, curvePoints);
			}
		}
		callbacks->setMouseAction(noAction);

		// Moving camera
		if (viewType == 2 || viewType == 3 || viewType == 4) {
			if (callbacks->isKeyPressed(GLFW_KEY_W))
				cm.moveForward(MOVE_SPEED);
			if (callbacks->isKeyPressed(GLFW_KEY_A))
				cm.moveHorizontal(-MOVE_SPEED);
			if (callbacks->isKeyPressed(GLFW_KEY_S))
				cm.moveForward(-MOVE_SPEED);
			if (callbacks->isKeyPressed(GLFW_KEY_D))
				cm.moveHorizontal(MOVE_SPEED);

			glm::vec2 mouseDiff = mousePos - prevMousePos;
			cm.rotateCamera(mouseDiff.x, mouseDiff.y);
			cm.updateViewMat();
			prevMousePos = mousePos;
		}
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, view);
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (isWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		

		if (viewType == 1 || viewType == 2) {
			glDisable(GL_DEPTH_TEST);

			curveGPUGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(curvePoints.verts.size()));

			pointsGPUGeom.bind();
			glDrawArrays(GL_POINTS, 0, GLsizei(controlPoints.verts.size()));
		}
		else if (viewType == 3) {
			glEnable(GL_DEPTH_TEST);

			surfaceGPUGeom.bind();
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(surfacePoints.verts.size()));

			pointsGPUGeom.bind();
			glDrawArrays(GL_POINTS, 0, GLsizei(controlPoints.verts.size()));
		}
		else if (viewType == 4) {
			glEnable(GL_DEPTH_TEST);

			surfaceGPUGeom.bind();
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(surfacePoints.verts.size()));

			tensorGPUGeom.bind();
			glDrawArrays(GL_POINTS, 0, GLsizei(tensorControlPoints.verts.size()));
		}

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
