#pragma once

#include "types.h"

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "scene.h"
#include "camera.h"
#include "rng.h"
#include "queue.h"

namespace cpurt
{
	class Renderer
	{
	public:
		explicit Renderer(const Scene &scene);
		~Renderer();

		void draw(const Camera &camera, u32 *data, u32 width, u32 height);

	private:
		const Scene &m_scene;

		struct Tile
		{
			u32 *target;
			u32 startX, endX;
			u32 startY, endY;
		};

		BlockingQueue<Tile> m_queue{};
		std::vector<std::thread> m_threads{};

		std::mutex m_mutex{};
		std::condition_variable m_signal{};
		std::atomic<u32> m_tileCounter{};

		Rng m_rng{};
	};
}
