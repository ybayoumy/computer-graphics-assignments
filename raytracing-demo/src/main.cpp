#define _USE_MATH_DEFINES
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include <glm/gtx/vector_query.hpp>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "imagebuffer.h"
#include "RayTrace.h"
#include "Scene.h"
#include "Lighting.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

int hasIntersection(Scene const &scene, Ray ray, int skipID)
{
	for (auto &shape : scene.shapesInScene)
	{
		Intersection tmp = shape->getIntersection(ray);
		if (
			shape->id != skipID && tmp.numberOfIntersections != 0 && glm::distance(tmp.point, ray.origin) > 0.00001 && glm::distance(tmp.point, ray.origin) < glm::distance(ray.origin, scene.lightPosition) - 0.01)
		{
			return tmp.id;
		}
	}
	return -1;
}

Intersection getClosestIntersection(Scene const &scene, Ray ray, int skipID)
{ // get the nearest
	Intersection closestIntersection;
	float min = std::numeric_limits<float>::max();
	for (auto &shape : scene.shapesInScene)
	{
		if (skipID == shape->id)
		{
			// Sometimes you need to skip certain shapes. Useful to
			// avoid self-intersection. ;)
			continue;
		}
		Intersection p = shape->getIntersection(ray);
		float distance = glm::distance(p.point, ray.origin);
		if (p.numberOfIntersections != 0 && distance < min)
		{
			min = distance;
			closestIntersection = p;
		}
	}
	return closestIntersection;
}

glm::vec3 raytraceSingleRay(Scene const &scene, Ray const &ray, int level, int source_id)
{
	// TODO: Part 3: Somewhere in this function you will need to add the code to determine
	//               if a given point is in shadow or not. Think carefully about what parts
	//               of the lighting equation should be used when a point is in shadow.
	// TODO: Part 4: Somewhere in this function you will need to add the code that does reflections and refractions.
	//               NOTE: The ObjectMaterial class already has a parameter to store the object's
	//               reflective properties. Use this parameter + the color coming back from the
	//               reflected array and the color from the phong shading equation.
	Intersection result = getClosestIntersection(scene, ray, source_id); // find intersection

	PhongReflection phong;
	phong.ray = ray;
	phong.scene = scene;
	phong.material = result.material;
	phong.intersection = result;

	if (result.numberOfIntersections == 0)
		return glm::vec3(0, 0, 0); // black;

	if (level < 1)
	{
		return glm::vec3(0);
	}

	Ray reflectedRay(result.point, normalize(ray.direction - 2.0f * dot(ray.direction, result.normal) * result.normal));
	glm::vec3 reflectedIntensity = raytraceSingleRay(scene, reflectedRay, level - 1, result.id);

	// Only output ambient light if point is under shadow
	Ray shadowRay(result.point, normalize(scene.lightPosition - result.point));
	int shapeID = hasIntersection(scene, shadowRay, result.id);
	if (shapeID != -1)
	{
		return phong.Ia() + phong.material.reflectionStrength * reflectedIntensity;
	}

	return phong.I() + phong.material.reflectionStrength * reflectedIntensity;
}

struct RayAndPixel
{
	Ray ray;
	int x;
	int y;
};

std::vector<RayAndPixel> getRaysForViewpoint(Scene const &scene, ImageBuffer &image, glm::vec3 viewPoint)
{
	int x = 0;
	int y = 0;
	std::vector<RayAndPixel> rays;

	for (float i = -1; x < image.Width(); x++)
	{
		y = 0;
		for (float j = -1; y < image.Height(); y++)
		{
			glm::vec3 direction = normalize(viewPoint + glm::vec3{0, 0, -1} + glm::vec3{i / 2, j / 2, 0});
			Ray r = Ray(viewPoint, direction);
			rays.push_back({r, x, y});
			j += 2.f / image.Height();
		}
		i += 2.f / image.Width();
	}
	return rays;
}

void raytraceImage(Scene const &scene, ImageBuffer &image, glm::vec3 viewPoint)
{
	// Reset the image to the current size of the screen.
	image.Initialize();

	// Get the set of rays to cast for this given image / viewpoint
	std::vector<RayAndPixel> rays = getRaysForViewpoint(scene, image, viewPoint);

	// This loops processes each ray and stores the resulting pixel in the image.
	// final color into the image at the appropriate location.
	//
	// I've written it this way, because if you're keen on this, you can
	// try and parallelize this loop to ensure that your ray tracer makes use
	// of all of your CPU cores
	//
	// Note, if you do this, you will need to be careful about how you render
	// things below too
	// std::for_each(std::begin(rays), std::end(rays), [&] (auto const &r) {
	for (auto const &r : rays)
	{
		glm::vec3 color = raytraceSingleRay(scene, r.ray, 5, -1);
		image.SetPixel(r.x, r.y, color);
	}
}

// EXAMPLE CALLBACKS
class Callbacks : public CallbackInterface
{

public:
	Callbacks()
	{
		viewPoint = glm::vec3(0, 0, 0);
		scene = initScene1();
		raytraceImage(scene, outputImage, viewPoint);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			shouldQuit = true;
		}

		if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		{
			scene = initScene1();
			raytraceImage(scene, outputImage, viewPoint);
		}

		if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		{
			scene = initScene2();
			raytraceImage(scene, outputImage, viewPoint);
		}
	}

	bool shouldQuit = false;

	ImageBuffer outputImage;
	Scene scene;
	glm::vec3 viewPoint;
};
// END EXAMPLES

int main()
{
	Log::debug("Starting main");

	// WINDOW
	glfwInit();

	// Change your image/screensize here.
	int width = 800;
	int height = 800;
	Window window(width, height, "Raytracing Demo");

	GLDebug::enable();

	// CALLBACKS
	std::shared_ptr<Callbacks> cb = std::make_shared<Callbacks>(); // can also update callbacks to new ones
	window.setCallbacks(cb);										   // can also update callbacks to new ones

	// RENDER LOOP
	while (!window.shouldClose() && !cb->shouldQuit)
	{
		glfwPollEvents();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cb->outputImage.Render();

		window.swapBuffers();
	}

	// Save image to file:
	// outpuImage.SaveToFile("foo.png")

	glfwTerminate();
	return 0;
}
