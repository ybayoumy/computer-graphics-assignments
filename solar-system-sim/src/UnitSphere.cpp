#include "UnitSphere.h"
#include <glm/gtx/transform.hpp>
#include "constants.h"
#include <iostream>

UnitSphere::UnitSphere(int granularity) : granularity(granularity) {
	generateGeometry();
}

void UnitSphere::generateGeometry() {
	float angleStep = PI / granularity;

	std::vector<glm::vec3> verts;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> faces;

	// calculating points
	for (int j = 0; j < 2 * granularity + 1; j++) {
		float phi = angleStep * j;
		for (int i = 0; i < granularity + 1; i++) {
			float theta = angleStep * i;
			glm::vec3 point(glm::sin(theta) * glm::sin(phi), glm::cos(theta), glm::sin(theta) * glm::cos(phi));
			verts.push_back(point);
			normals.push_back(point);
			texCoords.push_back(glm::vec2{ phi / (2 * PI), 1 - theta / PI });
		}
	}

	// creating faces using vertex indices
	for (int i = 1; i < granularity + 1; i++) {
		for (int j = 0; j < 2 * granularity + 1; j++) {
			if (j != 0) {
				faces.push_back((granularity + 1) * j + i);
				faces.push_back((granularity + 1) * (j - 1) + i);
				faces.push_back((granularity + 1) * j + i - 1);
			}

			if (j != 2 * granularity) {
				faces.push_back((granularity + 1) * j + i);
				faces.push_back((granularity + 1) * j + i - 1);
				faces.push_back((granularity + 1) * (j + 1) + i - 1);
			}
		}
	}

	m_gpu_geom.bind();
	m_gpu_geom.setVerts(verts);
	m_gpu_geom.setTexCoords(texCoords);
	m_gpu_geom.setNormals(normals);
	m_gpu_geom.setFaces(faces);

	m_size = faces.size();
}
