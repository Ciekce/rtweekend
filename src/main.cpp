#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

#include "types.h"

#include "config.h"
#include "scene.h"
#include "camera.h"
#include "render.h"
#include "rng.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rdparty/stb_image_write.h"

using namespace cpurt;

namespace
{
	const Sphere &initTestScene(Scene &scene)
	{
		const auto ground = scene.createDiffuse({0.8F, 0.8F, 0.0F}).id;

	//	const auto left = scene.createMetal({0.8F, 0.8F, 0.8F}, 0.3F).id;
		const auto left = scene.createDielectric({1.0F, 1.0F, 1.0F}, 1.52F).id;

	//	const auto center = scene.createDiffuse({0.7F, 0.3F, 0.3F}).id;
		const auto center = scene.createDiffuse({0.1F, 0.2F, 0.5F}).id;

	//	const auto right = scene.createMetal({0.8F, 0.6F, 0.2F}, 0.0F).id;
	//	const auto right = scene.createDielectric({1.0F, 1.0F, 1.0F}, 1.5F).id;
	//	const auto right = scene.createLight({4.8F, 3.6F, 1.2F}).id;
		const auto right = scene.createLight({-4.8F, -3.6F, -1.2F}).id;

		scene.createSphere({ // ground
			.pos = {0.0F, -100.5F, 0.0F},
			.radius = 100.0F,
			.materialId = ground
		});

		scene.createSphere({ // left
			.pos = {-1.0F, 0.0F, 0.0F},
			.radius = 0.5F,
			.materialId = left
		});

		const auto &centerSphere = scene.createSphere({ // center
			.pos = {0.0F, 0.0F, 0.0F},
			.radius = 0.5F,
			.materialId = center
		});

		scene.createSphere({ // right
			.pos = {1.0F, 0.0F, 0.0F},
			.radius = 0.5F,
			.materialId = right
		});

		return centerSphere;
	}

	void initRandomScene(Scene &scene)
	{
		Rng rng{0x696969};

		const auto groundMaterial = scene.createDiffuse({0.5F, 0.5F, 0.5F}).id;
		scene.createSphere({
			.pos = {0.0F, -1000.0F, 0.0F},
			.radius = 1000.0F,
			.materialId = groundMaterial
		});

		const auto glass = scene.createDielectric({1.0F, 1.0F, 1.0F}, 1.52F).id;

		for (i32 a = -11; a < 11; ++a)
		{
			for (i32 b = -11; b < 11; ++b)
			{
				const glm::vec3 center {
					static_cast<f32>(a) + 0.9F * rng.nextF32(),
					0.2F,
					static_cast<f32>(b) + 0.9F * rng.nextF32()
				};

				if (glm::length(center - glm::vec3{4.0F, 0.2F, 0.0F}) > 0.9F)
				{
					const auto materialSelector = rng.nextF32();
					u32 material;

					if (materialSelector < 0.8F)
						material = scene.createDiffuse(rng.nextColor() * rng.nextColor()).id;
					else if (materialSelector < 0.95F)
						material = scene.createMetal(rng.nextColor() * 0.5F + 0.5F, rng.nextF32() * 0.5F).id;
					else material = glass;

					scene.createSphere({
						.pos = center,
						.radius = 0.2F,
						.materialId = material
					});
				}
			//	else std::cout << "skipping sphere" << std::endl;
			}
		}

		scene.createSphere({
			.pos = {0.0F, 1.0F, 0.0F},
			.radius = 1.0F,
			.materialId = glass
		});

		const auto material2 = scene.createDiffuse({0.4F, 0.2F, 0.1F}).id;
		scene.createSphere({
			.pos = {-4.0F, 1.0F, 0.0F},
			.radius = 1.0F,
			.materialId = material2
		});

		const auto material3 = scene.createMetal({0.7F, 0.6F, 0.5F}, 0.0F).id;
		scene.createSphere({
			.pos = {4.0F, 1.0F, 0.0F},
			.radius = 1.0F,
			.materialId = material3
		});
	}

	void writeToFile(u32 width, u32 height, const u32 *data)
	{
		const auto time = std::time(nullptr);
		const auto *tm = std::localtime(&time);

		std::ostringstream filename{};
		filename << std::put_time(tm, "%Y-%m-%d_%H.%M.%S") << ".png";

		if (stbi_write_png(filename.str().c_str(),
			static_cast<i32>(width), static_cast<i32>(height),
			4, data, static_cast<i32>(width * sizeof(u32))))
			std::cout << "wrote to " << filename.view() << std::endl;
		else std::cerr << "failed to write to " << filename.view() << std::endl;
	}
}

int main()
{
	Scene scene{};
//	const auto &sphere = initTestScene(scene);
	initRandomScene(scene);

	scene.buildBvh();

	Renderer renderer{scene};

//	Camera camera{Width, Height, 90.0F, 0.001F, 1.0F};
	Camera camera{Width, Height, 20.0F, 0.1F, 10.0F};

//	camera.pos() = {0.0F, 0.0F, 2.0F};
//	camera.target() = {0.0F, 0.0F, -1.0F};
//	camera.target() = sphere.pos;

	camera.pos() = {13.0F, 2.0F, 3.0F};
	camera.target() = {0.0F, 0.0F, 0.0F};

	camera.update();

	std::vector<u32> buffer{};
	buffer.resize(Width * Height);

	renderer.draw(camera, buffer.data(), Width, Height);

	writeToFile(Width, Height, buffer.data());

	return 0;
}
