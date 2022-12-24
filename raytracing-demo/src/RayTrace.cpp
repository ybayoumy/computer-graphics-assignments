#include <iostream>
#include <math.h>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RayTrace.h"


using namespace std;
using namespace glm;

Sphere::Sphere(vec3 c, float r, int ID){
	centre = c;
	radius = r;
	id = ID;
}

Intersection Sphere::getIntersection(Ray ray){
	Intersection i{};
	i.id = id;
	i.material = material;

	vec3 D = ray.origin - this->centre;
	float determinant = (pow(dot(ray.direction, D), 2.0f) - pow(length(D), 2.0f) + pow(this->radius, 2.0f));

	if (determinant < 0) {
		return i;
	}

	float t;
	float t1 = -dot(ray.direction, D) - sqrt(determinant);
	float t2 = -dot(ray.direction, D) + sqrt(determinant);

	if (t1 < 0 && t2 < 0)
		return i;
	else if (t1 < 0)
		t = t2;
	else if (t2 < 0)
		t = t1;
	else
		t = std::min(t1, t2);

	i.numberOfIntersections = 1;
	i.point = ray.origin + t * ray.direction;
	i.normal = normalize(i.point - this->centre);

	return i;
}

//------------------------------------------------------------------------------
Cylinder::Cylinder(vec3 c, float r, int ID)
{
	center = c;
	radius = r;
	id = ID;
}

Intersection Cylinder::getIntersection(Ray ray)
{
	Intersection i{};
	i.id = id;
	i.material = material;

	float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
	float b = 2.0f * ((ray.origin.x - center.x) * ray.direction.x + (ray.origin.z - center.z) * ray.direction.z);
	float c = pow(ray.origin.x - center.x, 2.0f) + pow(ray.origin.z - center.z, 2.0f) - radius * radius;

	float determinant = pow(b, 2) - 4.0f * a * c;

	if (determinant < 0) {
		return i;
	}

	float t;
	float t1 = (- b - sqrt(determinant)) / (2.0f * a);
	float t2 = (-b + sqrt(determinant)) / (2.0f * a);

	if (t1 < 0 && t2 < 0)
		return i;
	else if (t1 <= 0)
		t = t2;
	else if (t2 <= 0)
		t = t1;
	else
		t = std::min(t1, t2);

	i.numberOfIntersections = 1;
	i.point = ray.origin + t * ray.direction;

	vec3 n = i.point - this->center;
	n.y = 0.0f;
	i.normal = normalize(n);

	// limiting height of the cylinder
	if (i.point.y < -height || i.point.y > height) {
		i.numberOfIntersections = 0;
		return i;
	}

	return i;
}

Plane::Plane(vec3 p, vec3 n, int ID){
	point = p;
	normal = n;
	id = ID;
}


float dot_normalized(vec3 v1, vec3 v2){
	return glm::dot(glm::normalize(v1), glm::normalize(v2));
}

void debug(char* str, vec3 a){
	cout << "debug:" << str << ": " << a.x <<", " << a.y <<", " << a.z << endl;
}
// --------------------------------------------------------------------------
void Triangles::initTriangles(int num, vec3 * t, int ID){
	id = ID;
	for(int i = 0; i< num; i++){
		triangles.push_back(Triangle(*t, *(t+1), *(t+2)));
		t+=3;
	}
}

Intersection Triangles::intersectTriangle(Ray ray, Triangle triangle){
	// From https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	const float EPSILON = 0.0000001;
	auto vertex0 = triangle.p1;
	auto vertex1 = triangle.p2;
	auto vertex2 = triangle.p3;

	glm::vec3 edge1, edge2, h, s, q;
	float a,f,u,v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = glm::cross(ray.direction, edge2);
	a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON) {
		return Intersection{}; // no intersection
	}
	f = 1.0/a;
	s = ray.origin - vertex0;
	u = f * glm::dot(s, h);
	if (u < 0.0 || u > 1.0) {
		return Intersection{}; // no intersection
	}
	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.direction, q);
	if (v < 0.0 || u + v > 1.0) {
		return Intersection{}; // no intersection
	}
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * glm::dot(edge2, q);
	// ray intersection
	if (t > EPSILON) {
		Intersection p;
		p.point = ray.origin + ray.direction * t;
		p.normal = glm::normalize(glm::cross(edge1, edge2));
		p.material = material;
		p.numberOfIntersections = 1;
		p.id = id;
		return p;
	} else {
		// This means that there is a line intersection but not a ray intersection.
		return Intersection{}; // no intersection
	}
}


Intersection Triangles::getIntersection(Ray ray){
	Intersection result{};
	result.material = material;
	result.id = id;
	float min = 9999;
	result = intersectTriangle(ray, triangles.at(0));
	if(result.numberOfIntersections!=0)min = glm::distance(result.point, ray.origin);
	for(int i = 1; i<triangles.size() ;i++){
		Intersection p = intersectTriangle(ray, triangles.at(i));
		if(p.numberOfIntersections !=0 && glm::distance(p.point, ray.origin) < min){
			min = glm::distance(p.point, ray.origin);
			result = p;
		}
	}

	result.material = material;
	result.id = id;
	return result;
}

Intersection Plane::getIntersection(Ray ray){
	Intersection result;
	result.material = material;
	result.id = id;
	result.normal = normal;
	if(dot(normal, ray.direction)>=0)return result;
	float s = dot(point - ray.origin, normal)/dot(ray.direction, normal);
	//if(s<0.00001)return result;
	result.numberOfIntersections = 1;
	result.point = ray.origin + s*ray.direction;
	return result;
}
