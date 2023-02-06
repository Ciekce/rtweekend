#pragma once

#include "types.h"

#include <limits>

namespace cpurt
{
	struct Material;

	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 dir;
	};

	struct TraceResult
	{
		const Material *hitMaterial;
		glm::vec3 missColor;

		glm::vec3 hitPos;
		glm::vec3 hitNormal;

		f32 t{std::numeric_limits<f32>::infinity()};
	};
}
