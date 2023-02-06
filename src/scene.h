#pragma once

#include "types.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstddef>

#include <glm/glm.hpp>

#include "material.h"
#include "ray.h"
#include "rng.h"

namespace cpurt
{
	struct Aabb
	{
		glm::vec3 min{};
		glm::vec3 max{};
	};

	struct SphereData
	{
		glm::vec3 pos;
		f32 radius;
		u32 materialId;
	};

	struct Sphere
	{
		glm::vec3 pos;
		f32 radius, radius2;
		u32 materialId;

		[[nodiscard]] inline Aabb aabb() const
		{
			return Aabb {
				.min = pos - radius,
				.max = pos + radius
			};
		}
	};

	struct Node
	{
		Aabb aabb{};

		const Sphere *sphere{};

		u32 left{};
		u32 right{};
	};

	struct TraceContext
	{
		const Sphere *sphere{};
		f32 t{std::numeric_limits<f32>::infinity()};
	};

	struct InvRay
	{
		glm::vec3 origin;
		glm::vec3 dir;

		explicit InvRay(const Ray &ray)
			: origin{ray.origin},
			  dir{1.0F / ray.dir} {}
	};

	class Scene
	{
	public:
		Scene();
		~Scene() = default;

		[[nodiscard]] inline auto &createDiffuse(glm::vec3 albedo)
		{
			Material material {
				.id = m_nextMaterialId++,
				.type = MaterialType::Diffuse,
				.diffuse = {
					.albedo = glm::clamp(albedo, {}, glm::vec3{1.0F})
				}
			};

			return m_materials.emplace_back(material);
		}

		[[nodiscard]] inline auto &createMetal(glm::vec3 albedo, f32 roughness)
		{
			Material material {
				.id = m_nextMaterialId++,
				.type = MaterialType::Metal,
				.metal = {
					.albedo = glm::clamp(albedo, {}, glm::vec3{1.0F}),
					.roughness = std::clamp(roughness, 0.0F, 1.0F)
				}
			};

			return m_materials.emplace_back(material);
		}

		[[nodiscard]] inline auto &createLight(glm::vec3 emitted)
		{
			Material material {
				.id = m_nextMaterialId++,
				.type = MaterialType::Light,
				.light = {
					.emitted = emitted
				}
			};

			return m_materials.emplace_back(material);
		}

		[[nodiscard]] inline auto &createDielectric(glm::vec3 color, f32 refractiveIndex)
		{
			Material material {
				.id = m_nextMaterialId++,
				.type = MaterialType::Dielectric,
				.dielectric = {
					.color = color,
					.refractiveIndex = refractiveIndex
				}
			};

			return m_materials.emplace_back(material);
		}

		Sphere &createSphere(const SphereData &data);

		[[nodiscard]] inline const auto &material(u32 id) const
		{
			return m_materials[id];
		}

		void buildBvh();

		void traceRay(TraceResult &result, const Ray &ray) const;

	private:
		void traceBvh(TraceContext &ctx, const Ray &ray, const InvRay &invRay, u32 node) const;

		[[nodiscard]] u32 allocNode();

		void populateInternalNode(u32 id, std::vector<const Sphere *> &spheres, u32 start, u32 end);
		void populateLeafNode(u32 id, const Sphere &sphere);

		std::vector<Material> m_materials{};
		u32 m_nextMaterialId{};

		std::vector<Sphere> m_spheres{};

		std::vector<Node> m_nodes{};
		u32 m_nextNodeId{};
	};
}
