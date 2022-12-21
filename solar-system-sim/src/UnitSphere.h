#pragma once

#include "Geometry.h"
#include <glm/gtx/transform.hpp>


struct UnitSphere {
	GPU_Geometry m_gpu_geom;
	GLsizei m_size;
	int granularity;

	UnitSphere(int granularity);
	void generateGeometry();
};
