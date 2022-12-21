#include "CelestialBody.h"

CelestialBody::CelestialBody(std::string texturePath, GLint interpolation, CelestialBody* parent, float scale, float distance,
	float inclination, float tilt, float spinSpeed, float orbitSpeed,
	float diffuseConstant, float specularConstant, float shininessConstant)
	: texture(texturePath, interpolation)
	, spinAngle(0.0f)
	, spinSpeed(spinSpeed)
	, orbitSpeed(orbitSpeed)
	, scaleMatrix(1.0f)
	, axialTiltRotationMatrix(1.0f)
	, spinRotationMatrix(1.0f)
	, translationMatrix(1.0f)
	, parent(parent)
	, parentDistance(distance)
	, diffuseScattering(diffuseConstant)
	, specularReflection(specularConstant)
	, shininess(shininessConstant)
{
	setScale(scale);

	orbitalAxis = glm::vec3{ 0.0f, 1.0f, 0.0f };
	if (parent != nullptr)
		orbitalAxis = glm::rotate(glm::mat4(1.0f), glm::radians(inclination), glm::vec3{ 0.0f, 0.0f, 1.0f }) * glm::vec4(parent->rotationalAxis, 0.0f);

	setRotationalAxis(tilt);

	setPosition(glm::normalize(glm::cross(orbitalAxis, glm::vec3{ 0.0f, 0.0f, 1.0f })) * parentDistance);
}

void CelestialBody::setScale(float newScale)
{
	scale = newScale;
	scaleMatrix = glm::mat4(
		scale, 0.f, 0.f, 0.f,
		0.f, scale, 0.f, 0.f,
		0.f, 0.f, scale, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
}

void CelestialBody::setRotationalAxis(float axialTilt)
{
	rotationalAxis = glm::rotate(glm::mat4(1.0f), glm::radians(axialTilt), glm::vec3{0.0f, 0.0f, 1.0f}) * glm::vec4(orbitalAxis, 0.0f);
	axialTiltRotationMatrix = glm::inverse(glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, -glm::vec3{ 0.0f, 0.0f, 1.0f }, rotationalAxis));
}

void CelestialBody::setPosition(glm::vec3 newPos)
{
	position = newPos;
	translationMatrix = glm::mat4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		position.x, position.y, position.z, 1.f
	);
}

glm::vec3 CelestialBody::getAbsolutePosition()
{
	if (parent == nullptr)
		return position;
	return position + parent->getAbsolutePosition();
}

void CelestialBody::spin(float deltaTime) {
	spinAngle += deltaTime * spinSpeed;

	if (spinAngle >= 360) spinAngle -= 360;
	else if (spinAngle < 0) spinAngle += 360;

	spinRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(spinAngle), rotationalAxis);
}

void CelestialBody::orbit(float deltaTime) {
	setPosition(glm::rotate(glm::mat4(1.0f), glm::radians(deltaTime * orbitSpeed), orbitalAxis) * glm::vec4(position, 1.0f));
}

glm::mat4 CelestialBody::getParentTranslation() {
	if (parent == nullptr)
		return glm::mat4(1.0f);
	else
		return parent->getParentTranslation() * parent->translationMatrix;
}

glm::mat4 CelestialBody::getModelMatrix()
{
	glm::mat4 parentTranslation = getParentTranslation();
	return parentTranslation * translationMatrix * spinRotationMatrix * axialTiltRotationMatrix * scaleMatrix;
}

void CelestialBody::draw(ShaderProgram& shader, GLsizei sphereSize)
{
	texture.bind();
	GLint location = glGetUniformLocation(shader, "M");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(getModelMatrix()));

	location = glGetUniformLocation(shader, "diffuse");
	glUniform1f(location, diffuseScattering);

	location = glGetUniformLocation(shader, "specular");
	glUniform1f(location, specularReflection);

	location = glGetUniformLocation(shader, "shininess");
	glUniform1f(location, shininess);

	glDrawElements(GL_TRIANGLES, sphereSize, GL_UNSIGNED_INT, 0);
	texture.unbind();
}

void CelestialBody::reset() {
	setPosition(glm::normalize(glm::cross(orbitalAxis, glm::vec3{ 0.0f, 0.0f, 1.0f })) * parentDistance);
	spinAngle = 0;
}
