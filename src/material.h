#pragma once

#include "types.h"

#include <glm/vec3.hpp>

namespace cpurt
{
	enum class MaterialType : u32
	{
		Diffuse = 0,
		Metal,
		Dielectric,
		Light
	};

	struct DiffuseData
	{
		glm::vec3 albedo;
	};

	struct MetalData
	{
		glm::vec3 albedo;
		f32 roughness;
	};

	struct DielectricData
	{
		glm::vec3 color;
		f32 refractiveIndex;
	};

	struct LightData
	{
		glm::vec3 emitted;
	};

	struct Material
	{
		u32 id;
		MaterialType type;
		union
		{
			DiffuseData diffuse;
			MetalData metal;
			DielectricData dielectric;
			LightData light;
		};
	};
}
