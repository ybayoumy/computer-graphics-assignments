#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#define SHIP_SPEED 0.003
#define SHIP_ROTATION_SPEED 1.5
#define FIREBALL_ROTATION_SPEED 1.5

CPU_Geometry squareGeom() {
	CPU_Geometry retGeom;
	// vertices
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));

	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

struct GameObject {
	GameObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		children(),
		position(0.0f, 0.0f, 0.0f),
		theta(0),
		scale(1),
		translationMatrix(1.f),
		rotationMatrix(1.f),
		scaleMatrix(1.f)
	{}

	Texture texture;
	std::vector<GameObject*> children;

	glm::vec3 position;
	float theta;
	float scale;

	glm::mat4 translationMatrix;
	glm::mat4 rotationMatrix;
	glm::mat4 scaleMatrix;

	void setTheta(float newTheta) {
		theta = newTheta;

		if (theta > 360) theta -= 360;
		else if (theta < 0) theta += 360;

		float C = glm::cos(glm::radians(theta));
		float S = glm::sin(glm::radians(theta));
		rotationMatrix = glm::mat4(
			C, -S, 0.f, 0.f,
			S, C, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}

	void setScale(float newScale) {
		scale = newScale;

		scaleMatrix = glm::mat4(
			scale, 0.f, 0.f, 0.f,
			0.f, scale, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}

	void setPosition(glm::vec3 newPos) {
		position = newPos;

		translationMatrix = glm::mat4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			position.x, position.y, position.z, 1.f
		);
	}

	glm::mat4 genTransformationMatrix() {
		return translationMatrix * rotationMatrix * scaleMatrix;
	}
};

class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader, int screenWidth, int screenHeight) :
		shader(shader),
		wPressed(false),
		sPressed(false),
		restart(false),
		mousePosition(),
		screenDim(screenWidth, screenHeight)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			wPressed = true;
		}
		else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			sPressed = true;
		}
		else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			wPressed = false;
		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			sPressed = false;
		}
		else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			restart = true;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		mousePosition.x = (float) xpos;
		mousePosition.y = (float) ypos;
	}

	bool isKeyPressed(int button) {
		if (button == GLFW_KEY_W) {
			return wPressed;
		}
		else if (button == GLFW_KEY_S) {
			return sPressed;
		}
		else {
			return false;
		}
	}

	glm::vec2 getMouseCoords() {
		glm::vec2 scaledVec = (mousePosition + glm::vec2(0.5f, 0.5f)) / screenDim;
		glm::vec2 flippedY = glm::vec2(scaledVec.x, 1.0 - scaledVec.y);

		return flippedY * 2.0f - glm::vec2(1.0f, 1.0f);
	}

	bool shouldRestart() {
		if (restart) {
			restart = false;
			return true;
		}
		return false;
	}

private:
	ShaderProgram& shader;

	bool wPressed;
	bool sPressed;
	bool restart;
	glm::vec2 mousePosition;
	glm::vec2 screenDim;
};

void moveShip(GameObject& ship, float speed) {
	glm::vec3 direction = ship.rotationMatrix * glm::vec4(0.f, 1.f, 0.f, 0.0f);
	ship.setPosition(ship.position + direction * speed);
}

void rotateShip(GameObject& ship, float rotation) {
	float newTheta = ship.theta + rotation;
	ship.setTheta(newTheta);
}

GameObject setupShip() {
	GameObject ship("textures/ship.png", GL_NEAREST);
	ship.setScale(0.1f);
	ship.setTheta(90.f);

	return ship;
}

std::vector<GameObject> setupFireballs() {
	std::vector<GameObject> fireballs;

	for (int i = 0; i < 5; i++) {
		fireballs.push_back(GameObject("textures/fire.png", GL_NEAREST));
		fireballs[i].setScale(0.04f);
	}
	return fireballs;
}

std::vector<GameObject> setupDiamonds(std::vector<GameObject>& fireballs) {
	std::vector<GameObject> diamonds;

	for (int i = 0; i < 5; i++) {
		diamonds.push_back(GameObject("textures/diamond.png", GL_NEAREST));
		diamonds[i].children.push_back(&fireballs[i]);
		diamonds[i].setScale(0.07f);
	}

	diamonds[0].setPosition(glm::vec3(0.5f, 0.5f, 0.f));
	diamonds[1].setPosition(glm::vec3(-0.5f, 0.5f, 0.f));
	diamonds[2].setPosition(glm::vec3(0.5f, -0.5f, 0.f));
	diamonds[3].setPosition(glm::vec3(-0.5f, -0.5f, 0.f));
	diamonds[4].setPosition(glm::vec3(0.f, 0.75f, 0.f));

	return diamonds;
}

void drawDiamond(GameObject& diamond, GLuint uniformLocation) {
	diamond.texture.bind();
	glm::mat4 transformation = diamond.genTransformationMatrix();
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &transformation[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	diamond.texture.unbind();

	std::vector<GameObject*> children = diamond.children;
	for (int i = 0; i < children.size(); i++) {
		GameObject& child = *children[i];
		float step = diamond.scale + 0.1f * (i + 1);
		child.setPosition(glm::vec3(0.f, 1.f, 0.f) * step);

		child.texture.bind();
		glm::mat4 transformation = diamond.translationMatrix * child.rotationMatrix * child.translationMatrix * child.scaleMatrix;
		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &transformation[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		child.texture.unbind();
	}
}

void drawShip(GameObject& ship, GLuint uniformLocation) {
	ship.texture.bind();
	glm::mat4 transformation = ship.genTransformationMatrix();
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &transformation[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	ship.texture.unbind();

	std::vector<GameObject*> children = ship.children;
	for (int i = 0; i < children.size(); i++) {
		glm::vec3 direction = ship.rotationMatrix * glm::vec4(0.f, 1.f, 0.f, 0.0f);
		float step = ship.scale + 0.1f * (i+1);
		children[i]->setPosition(ship.position - direction * step);
		drawDiamond(*children[i], uniformLocation);
	}
}

std::vector<GameObject*> genPointervector(std::vector<GameObject>& v) {
	std::vector<GameObject*> res;
	for (int i = 0; i < v.size(); i++) {
		res.push_back(&v[i]);
	}
	return res;
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453");

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	std::shared_ptr<MyCallbacks> inputManager = std::make_shared<MyCallbacks>(shader, 800, 800);
	window.setCallbacks(inputManager);

	CPU_Geometry square_cgeom = squareGeom();
	GPU_Geometry square_ggeom;
	square_ggeom.setVerts(square_cgeom.verts);
	square_ggeom.setTexCoords(square_cgeom.texCoords);

	GameObject ship = setupShip();
	std::vector<GameObject> fireballs = setupFireballs();
	std::vector<GameObject> diamonds = setupDiamonds(fireballs);
	std::vector<GameObject*> activeDiamonds = genPointervector(diamonds);

	shader.use();
	square_ggeom.bind();
	GLint uniformLocation = glGetUniformLocation(shader.getProgram(), "trans");

	int score = 0;
	bool gameLost = false;

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		if (inputManager->shouldRestart()) {
			ship = setupShip();
			fireballs = setupFireballs();
			diamonds = setupDiamonds(fireballs);
			activeDiamonds = genPointervector(diamonds);
			score = 0;
			gameLost = false;
		}

		if (inputManager->isKeyPressed(GLFW_KEY_W)) moveShip(ship, (float) SHIP_SPEED);
		else if (inputManager->isKeyPressed(GLFW_KEY_S)) moveShip(ship, (float)  -SHIP_SPEED);

		glm::vec2 mousePos = inputManager->getMouseCoords();
		glm::vec2 newDirection = mousePos - glm::vec2(ship.position);
		float cursorDist = glm::length(newDirection);

		if (cursorDist > ship.scale) {
			glm::vec2 normalizedNewDirection = glm::normalize(newDirection);
			glm::vec2 normalizedCurrDirection = glm::normalize(glm::mat2(ship.rotationMatrix) * glm::vec2(0.f, 1.f));

			float cos_degrees = glm::degrees(glm::acos(glm::dot(normalizedNewDirection, normalizedCurrDirection)));
			float sin_degrees = glm::degrees(glm::asin(normalizedNewDirection.x * normalizedCurrDirection.y - normalizedNewDirection.y * normalizedCurrDirection.x));
			if (cos_degrees > SHIP_ROTATION_SPEED) {
				if (sin_degrees > 0) rotateShip(ship, SHIP_ROTATION_SPEED);
				else if (sin_degrees < 0) rotateShip(ship, -SHIP_ROTATION_SPEED);
			}
		}

		// rotating all fireballs
		for (GameObject& fireball : fireballs) {
			fireball.setTheta(fireball.theta + (float) FIREBALL_ROTATION_SPEED);
		}

		// rotating all diamonds if game is won
		if (activeDiamonds.size() == 0 && gameLost == false) {
			for (GameObject& diamond : diamonds) {
				diamond.setTheta(diamond.theta + (float)FIREBALL_ROTATION_SPEED);
			}
		}

		// Checking for collisions between diamond/fireballs and ship
		for (int i = 0; i < activeDiamonds.size(); i++) {
			GameObject& diamond = *activeDiamonds[i];

			float shipRadius = ship.scale;
			float diamondRadius = diamond.scale;

			float shipDistance = glm::length(ship.position - diamond.position);
			if (shipDistance < shipRadius + diamondRadius) {
				diamond.setScale(diamond.scale / 2);
				for (GameObject* fireball : diamond.children) {
					fireball->setScale(fireball->scale / 2);
				}

				ship.children.push_back(&diamond);
				activeDiamonds.erase(activeDiamonds.begin() + i);
				ship.setScale(ship.scale + 0.02f);
				score++;
			}

			for (GameObject* fireballPointer : diamond.children) {
				GameObject& fireball = *fireballPointer;

				float fireballRadius = fireball.scale;
				glm::vec3 fireballPos = diamond.translationMatrix * fireball.rotationMatrix * fireball.translationMatrix * fireball.scaleMatrix * glm::vec4(fireball.position, 1.0);
				float shipDistance = glm::length(ship.position - fireballPos);

				if (shipDistance < shipRadius + fireballRadius) {
					activeDiamonds.clear();
					gameLost = true;
				}
			}
		}

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw ship
		drawShip(ship, uniformLocation);

		// draw diamonds
		for (int i = 0; i < activeDiamonds.size(); i++) {
			drawDiamond(*activeDiamonds[i], uniformLocation);
		}

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool *)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		if (gameLost == true) {
			ImGui::Text("Oh no, you Lost! Press 'R' to restart the game");
		} else if (activeDiamonds.size() == 0) {
			ImGui::Text("Congratulations, you won! Press 'R' to restart the game");
		} else {
			ImGui::Text("Score: %d", score); // Second parameter gets passed into "%d"
		}

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
