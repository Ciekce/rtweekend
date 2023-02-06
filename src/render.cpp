#include "render.h"

#include <limits>
#include <iostream>

#include "config.h"
#include "ray.h"
#include "timer.h"

namespace cpurt
{
	namespace
	{
		constexpr auto ScatterEpsilon = 0.000000001F;

		const auto Gamma3 = glm::vec3{Gamma};
		const auto InvGamma = 1.0F / Gamma3;

		constexpr bool Tonemap = false;
		constexpr bool GammaCorrect = true;

		inline u32 toColor(glm::vec3 rgb)
		{
			rgb = glm::clamp(rgb, 0.0F, 1.0F);
			return 0xFF000000
				| (static_cast<u32>(rgb.r * 255.0F) <<  0)
				| (static_cast<u32>(rgb.g * 255.0F) <<  8)
				| (static_cast<u32>(rgb.b * 255.0F) << 16);
		}

		inline f32 schlick(f32 cosTheta, f32 refractiveIndex)
		{
			auto r0 = (1.0F - refractiveIndex) / (1.0F + refractiveIndex);
			r0 *= r0;
			return r0 + (1.0F - r0) * glm::pow(1.0F - cosTheta, 5.0F);
		}

		glm::vec3 trace(const Scene &scene, const Ray &initial, Rng &rng)
		{
			glm::vec3 color{1.0F};

			TraceResult result{};
			Ray ray{initial};

			for (u32 i = 0; i <= Bounces; ++i)
			{
				scene.traceRay(result, ray);

				if (!result.hitMaterial)
				{
					color *= result.missColor;
					break;
				}

				bool front = true;
				auto normal = result.hitNormal;

				if (glm::dot(ray.dir, result.hitNormal) > 0.0F)
				{
					front = false;
					normal = -normal;
				}

				ray.origin = result.hitPos;

				const auto &material = *result.hitMaterial;

				bool bounce = true;

				switch (material.type)
				{
				case MaterialType::Diffuse:
					color *= material.diffuse.albedo;

					ray.dir = result.hitNormal + rng.nextUnit();

					if (ray.dir.x < ScatterEpsilon
						&& ray.dir.y < ScatterEpsilon
						&& ray.dir.z < ScatterEpsilon)
						ray.dir = result.hitNormal;

					break;

				case MaterialType::Metal:
					{
						color *= material.metal.albedo;

						const auto dir = glm::normalize(ray.dir);

						ray.dir = glm::reflect(dir, result.hitNormal) + material.metal.roughness * rng.nextUnit();
						bounce = glm::dot(ray.dir, result.hitNormal) > 0.0F;
					}
					break;

				case MaterialType::Dielectric:
					{
						const auto dir = glm::normalize(ray.dir);

						const auto ratio = front
							? 1.0F / material.dielectric.refractiveIndex
							: material.dielectric.refractiveIndex;

						const auto cosTheta = std::min(glm::dot(-dir, normal), 1.0F);
						const auto sinTheta = glm::sqrt(1.0F - cosTheta * cosTheta);

						if (ratio * sinTheta > 1.0F || schlick(cosTheta, ratio) > rng.nextF32())
							ray.dir = glm::reflect(dir, normal);
						else
						{
						//	if (!front)
						//		color *= material.dielectric.color; // not physically correct
							ray.dir = glm::refract(dir, normal, ratio);
						}
					}
					break;

				case MaterialType::Light:
					color *= material.light.emitted;
					bounce = false;
					break;
				}

				if (!bounce)
					break;
			}

			if (result.hitMaterial && result.hitMaterial->type != MaterialType::Light)
				color = glm::vec3{};

			return color;
		}
	}

	Renderer::Renderer(const Scene &scene)
		: m_scene{scene} {}

	Renderer::~Renderer()
	{
		for (const auto &thread : m_threads)
		{
			m_queue.push({
				.target = nullptr
			});
		}

		for (auto &thread : m_threads)
		{
			thread.join();
		}
	}

	void Renderer::draw(const Camera &camera, u32 *data, u32 width, u32 height)
	{
		if (m_threads.empty())
		{
			const u32 threadCount = Threads == 0 ? std::thread::hardware_concurrency() : Threads;

			std::cout << "launching " << threadCount << " threads" << std::endl;

			m_threads.reserve(threadCount);

			for (i32 i = 0; i < threadCount; ++i)
			{
				m_threads.emplace_back([this, &camera]
				{
					Rng rng{};

					while (true)
					{
						auto tile = m_queue.wait();

						if (!tile.target)
							break;

						for (u32 y = tile.startY; y < tile.endY; ++y)
						{
							for (u32 x = tile.startX; x < tile.endX; ++x)
							{
								glm::vec3 result{};

								for (u32 i = 0; i < Samples; ++i)
								{
									const auto ray = camera.ray(rng, x, y);
									result += trace(m_scene, ray, rng);
								}

								result /= static_cast<f32>(Samples);

								result = glm::max(result, glm::vec3{});

								if constexpr(Tonemap)
									result = result / (1.0F + result); // reinhard

								if constexpr(GammaCorrect)
									result = glm::pow(result, InvGamma);

								tile.target[y * camera.width() + x] = toColor(result);
							}
						}

						{
							std::scoped_lock lock{m_mutex};
							--m_tileCounter;
							m_signal.notify_all();
						}
					}
				});
			}
		}

		const auto totalTiles = ((width + TileSize - 1) / TileSize) * ((height + TileSize - 1) / TileSize);

		std::cout << "total tiles: " << totalTiles << std::endl;

		m_tileCounter.store(totalTiles);

		for (u32 y = 0; y < height; y += TileSize)
		{
			for (u32 x = 0; x < width; x += TileSize)
			{
				m_queue.push({
					.target = data,
					.startX = x,
					.endX = std::min(width, x + TileSize),
					.startY = y,
					.endY = std::min(height, y + TileSize)
				});
			}
		}

		std::unique_lock lock{m_mutex};

		Timer timer{};

		const auto start = timer.time();

		m_signal.wait(lock, [this, totalTiles, &timer, start]
		{
			static auto prevRemaining = totalTiles;
			static auto prevTotalTime = 0.0;

			const auto remainingTiles = m_tileCounter.load();

			if (remainingTiles > 0)
			{
				const auto time = timer.time();

				const auto totalTime = time - start;
				const auto timeSinceLast = totalTime - prevTotalTime;

				if (false
				//	|| (remainingTiles < prevRemaining && remainingTiles % 256 == 0)
					|| timeSinceLast > 4.0
				)
				{
					const auto tilesPerSec = static_cast<f64>(prevRemaining - remainingTiles) / timeSinceLast;

					std::cout << "remaining tiles: " << remainingTiles
						<< " (total time " << (totalTime * 1000.0) << " ms, "
						<< tilesPerSec << " tiles/sec, estimated "
						<< (static_cast<f64>(remainingTiles) / tilesPerSec) << " sec remaining)" << std::endl;

					prevTotalTime = totalTime;
					prevRemaining = remainingTiles;
				}

				return false;
			}

			return true;
		});

		const auto totalTime = timer.time() - start;
		const auto tilesPerSec = static_cast<f64>(totalTiles) / totalTime;

		std::cout << "render time: " << (totalTime * 1000.0) << " ms, " << tilesPerSec << " tiles/sec" << std::endl;
	}
}
