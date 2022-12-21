#pragma once

//------------------------------------------------------------------------------
// This file contains an implementation of a spherical camera
//------------------------------------------------------------------------------

//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "CelestialBody.h"

class Camera {
public:

	Camera(float t, float p, float r);

	void setFocusBody(CelestialBody* body);

	glm::mat4 getView();
	glm::vec3 getPos();
	void incrementTheta(float dt);
	void incrementPhi(float dp);
	void incrementR(float dr);

private:

	float theta;
	float phi;
	float radius;

	CelestialBody* focusBody;
};
