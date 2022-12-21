#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/type_ptr.hpp"

#include "Texture.h"
#include "ShaderProgram.h"
#include <iostream>

class CelestialBody {

public:
	CelestialBody(std::string texturePath, GLint interpolation, CelestialBody* parent, float scale, float distance,
		float inclination, float tilt, float spinSpeed, float orbitSpeed,
		float diffuseConstant, float specularConstant, float shininessConstant);

	void setScale(float newScale);
	void setRotationalAxis(float axialTilt);
	void setPosition(glm::vec3 newPos);
	glm::vec3 getAbsolutePosition();

	void spin(float deltaTime);
	void orbit(float deltaTime);

	glm::mat4 getParentTranslation();

	glm::mat4 getModelMatrix();

	void draw(ShaderProgram& shader, GLsizei sphereSize);
	void reset();

private:
	Texture texture;

	float scale;
	glm::vec3 position;

	float spinAngle;

	// in degrees / second
	float spinSpeed;
	float orbitSpeed;

	glm::vec3 orbitalAxis;
	glm::vec3 rotationalAxis;

	CelestialBody* parent;
	float parentDistance;

	float diffuseScattering;
	float specularReflection;
	float shininess;

	// Storing matrices to minimize calculations in the getModelMatrix() function
	glm::mat4 scaleMatrix;
	glm::mat4 axialTiltRotationMatrix;
	glm::mat4 spinRotationMatrix;
	glm::mat4 translationMatrix;
};
