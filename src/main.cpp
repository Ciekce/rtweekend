#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

#include <SDL.h>

#include "types.h"
#include "scene.h"
#include "camera.h"
#include "render.h"
#include "rng.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rdparty/stb_image_write.h"

using namespace cpurt;

namespace
{
	class SdlInitGuard
	{
	public:
		explicit SdlInitGuard(u32 flags = SDL_INIT_EVERYTHING)
			: m_success{!SDL_Init(flags)} {}

		~SdlInitGuard()
		{
			SDL_Quit();
		}

		inline explicit operator bool() const
		{
			return m_success;
		}

		SdlInitGuard(const SdlInitGuard &) = delete;
		SdlInitGuard(SdlInitGuard &&) = delete;

	private:
		bool m_success;
	};

	class SdlWindow
	{
	public:
		SdlWindow(const std::string &title, i32 x, i32 y, u32 width, u32 height, u32 flags)
			: m_window{SDL_CreateWindow(title.c_str(), x, y, static_cast<i32>(width), static_cast<i32>(height), flags)}
		{}

		~SdlWindow()
		{
			SDL_DestroyWindow(m_window);
		}

		inline operator SDL_Window *() // NOLINT
		{
			return m_window;
		}

	private:
		SDL_Window *m_window;
	};

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

	void initBigScene(Scene &scene)
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
				else std::cout << "skipping sphere" << std::endl;
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

int main(int argc, char *argv[])
{
	SdlInitGuard sdlInitGuard{SDL_INIT_VIDEO | SDL_INIT_EVENTS};

	if (!sdlInitGuard)
	{
		std::cerr << "failed to initialize SDL" << std::endl;
		return 1;
	}

	constexpr u32 Width = 1200;
	constexpr u32 Height = 800;

	SdlWindow window{"traes rey", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_SHOWN};

	auto *surface = SDL_GetWindowSurface(window);

	Scene scene{};
//	const auto &sphere = initTestScene(scene);
	initBigScene(scene);

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

	{
		std::vector<u32> buffer{};
		buffer.resize(Width * Height);

		renderer.draw(camera, buffer.data(), Width, Height);

		writeToFile(Width, Height, buffer.data());

		SDL_PumpEvents();

		SDL_LockSurface(surface);
		auto *surfacePixels = static_cast<u32 *>(surface->pixels);

		for (size_t i = 0; i < buffer.size(); ++i)
		{
			// renderer outputs rgba, sdl wants bgra
			surfacePixels[i] = 0xFF000000 | (__builtin_bswap32(buffer[i]) >> 8);
		}

		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);
	}

	bool running = true;

	SDL_Event event{};
	while (running && SDL_WaitEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_WINDOWEVENT:
			{
				switch (event.window.type)
				{
				case SDL_WINDOWEVENT_CLOSE:
					running = false;
					break;
				}
			}
			break;
		case SDL_KEYDOWN:
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_F9:
					running = false;
					break;
				}
			}
			break;
		}
	}

	return 0;
}
