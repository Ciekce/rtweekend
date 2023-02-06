#include "scene.h"

#include <array>
#include <iostream>

#include <glm/gtx/norm.hpp>

namespace cpurt
{
	namespace
	{
		constexpr bool TraceBvh = true;
		constexpr bool Skybox = true;

		constexpr auto HitEpsilon = 0.001F;

		__attribute__((always_inline)) void closestHit(TraceResult &result,
			const Scene &scene, const Ray &ray, const Sphere &hit, f32 distance)
		{
			const auto pos = ray.origin + ray.dir * distance;
			const auto normal = glm::normalize(pos - hit.pos);

			result.hitMaterial = &scene.material(hit.materialId);

			result.hitPos = pos;
			result.hitNormal = normal;
		}

		__attribute__((always_inline)) void miss(TraceResult &result, const Scene &scene, const Ray &ray)
		{
			result.hitMaterial = nullptr;

			if constexpr(Skybox)
			{
				static const glm::vec3 SkyA{1.0F, 1.0F, 1.0F};
				static const glm::vec3 SkyB{0.5F, 0.7F, 1.0F};

				const auto t = glm::normalize(ray.dir).y * 0.5F + 0.5F;

				result.missColor = glm::mix(SkyA, SkyB, t);
			}
			else result.missColor = glm::vec3{};
		}

		namespace intersection
		{
			__attribute__((always_inline))  bool aabb(const InvRay &ray, const Aabb &aabb, f32 t)
			{
				const auto tx1 = (aabb.min.x - ray.origin.x) * ray.dir.x;
				const auto tx2 = (aabb.max.x - ray.origin.x) * ray.dir.x;

				auto tMin = std::min(tx1, tx2);
				auto tMax = std::max(tx1, tx2);

				const auto ty1 = (aabb.min.y - ray.origin.y) * ray.dir.y;
				const auto ty2 = (aabb.max.y - ray.origin.y) * ray.dir.y;

				tMin = std::max(tMin, std::min(ty1, ty2));
				tMax = std::min(tMax, std::max(ty1, ty2));

				const auto tz1 = (aabb.min.z - ray.origin.z) * ray.dir.z;
				const auto tz2 = (aabb.max.z - ray.origin.z) * ray.dir.z;

				tMin = std::max(tMin, std::min(tz1, tz2));
				tMax = std::min(tMax, std::max(tz1, tz2));

				return tMax >= std::max(HitEpsilon, tMin) && tMin < t;
			}

			__attribute__((always_inline)) f32 sphere(const Ray &ray, const Sphere &sphere)
			{
				const auto origin = ray.origin - sphere.pos;

				const auto a = glm::length2(ray.dir);
				const auto b = glm::dot(origin, ray.dir);
				const auto c = glm::length2(origin) - sphere.radius2;

				const auto d = b * b - a * c;

				if (d < 0.0F)
					return -1.0F;

				const auto h = glm::sqrt(d);

				auto t = (-b - h) / a;

				if (t <= HitEpsilon)
				{
					t = (-b + h) / a;
					if (t <= HitEpsilon)
						return -1.0F;
				}

				return t;
			}
		}

		template <i32 Axis>
		inline bool compareAabb(const Aabb &a, const Aabb &b)
		{
			return a.min[Axis] < b.min[Axis];
		}

		template <i32 Axis>
		inline bool compareSphereAabb(const Sphere *a, const Sphere *b)
		{
			return compareAabb<Axis>(a->aabb(), b->aabb());
		}

		inline Aabb boundingAabb(const Aabb &a, const Aabb &b)
		{
			return Aabb {
				.min = glm::min(a.min, b.min),
				.max = glm::max(a.max, b.max)
			};
		}
	}

	Scene::Scene()
	{
		// "missing texture" material
		(void)createMetal({1.0F, 0.0F, 1.0F}, 0.0F);
	}

	Sphere &Scene::createSphere(const SphereData &data)
	{
		auto &sphere = m_spheres.emplace_back(Sphere {
			.pos = data.pos,
			.radius = data.radius,
			.radius2 = data.radius * data.radius,
			.materialId = data.materialId
		});

		/*
		sphere.aabb = {
			.min = {
				sphere.pos.x - sphere.radius,
				sphere.pos.y - sphere.radius,
				sphere.pos.z - sphere.radius
			},
			.max = {
				sphere.pos.x + sphere.radius,
				sphere.pos.y + sphere.radius,
				sphere.pos.z + sphere.radius
			}
		};
		*/

		return sphere;
	}

	void Scene::buildBvh()
	{
		m_nodes.clear();
		m_nextNodeId = 0;

		if (m_spheres.empty())
		{
			std::cerr << "cannot build bvh for empty scene" << std::endl;
			return;
		}
		else std::cout << m_spheres.size() << " spheres" << std::endl;

		const auto root = allocNode(); // always 0

		if (m_spheres.size() == 1)
			m_nodes[root].sphere = &m_spheres[0];
		else
		{
			std::vector<const Sphere *> spherePtrs{};
			spherePtrs.reserve(m_spheres.size());

			for (const auto &sphere : m_spheres)
			{
				spherePtrs.push_back(&sphere);
			}

			populateInternalNode(root, spherePtrs, 0, m_spheres.size());
		}
	}

	void Scene::traceRay(TraceResult &result, const Ray &ray) const
	{
		if constexpr(TraceBvh)
		{
			TraceContext ctx{};
			InvRay invRay{ray};

			traceBvh(ctx, ray, invRay, 0);

			if (ctx.sphere)
				closestHit(result, *this, ray, *ctx.sphere, ctx.t);
			else miss(result, *this, ray);
		}
		else
		{
			const Sphere *hit = nullptr;
			auto tMin = std::numeric_limits<f32>::infinity();

		//	const InvRay invRay{ray};

			for (const auto &sphere: m_spheres)
			{
			//	if (intersection::aabb(invRay, sphere.aabb, tMin))
				{
					const auto t = intersection::sphere(ray, sphere);
					if (t > 0.0F && t < tMin)
					{
						hit = &sphere;
						tMin = t;
					}
				}
			}

			if (hit)
				closestHit(result, *this, ray, *hit, tMin);
			else miss(result, *this, ray);
		}
	}

	void Scene::traceBvh(TraceContext &ctx, const Ray &ray, const InvRay &invRay, u32 node) const
	{
		const auto &n = m_nodes[node];

		if (n.sphere)
		{
			const auto t = intersection::sphere(ray, *n.sphere);

			if (t > 0.0F && t < ctx.t)
			{
				ctx.sphere = n.sphere;
				ctx.t = t;
			}

			return;
		}

		if (!intersection::aabb(invRay, n.aabb, ctx.t))
			return;

		traceBvh(ctx, ray, invRay, n.left);
		traceBvh(ctx, ray, invRay, n.right);
	}

	u32 Scene::allocNode()
	{
		const u32 id = m_nodes.size();
		m_nodes.emplace_back();
		return id;
	}

	// basic kd tree split by the largest dimension
	// (i.e. the "next week" bvh with modifications)
	void Scene::populateInternalNode(u32 id, std::vector<const Sphere *> &spheres, u32 start, u32 end)
	{
		const auto count = end - start;

		if (count == 1)
			populateLeafNode(id, *spheres[start]);
		else
		{
			m_nodes[id].aabb = spheres[start]->aabb();

			for (u32 i = start + 1; i < end; ++i)
			{
				m_nodes[id].aabb = boundingAabb(m_nodes[id].aabb, spheres[i]->aabb());
			}

			const auto size = m_nodes[id].aabb.max - m_nodes[id].aabb.min;

			i32 axis = 0;
			auto maxSize = size.x;

			for (i32 i = 1; i < 3; ++i)
			{
				if (size[i] > maxSize)
				{
					axis = i;
					maxSize = size[i];
				}
			}

			const auto comparator = axis == 0
				? compareSphereAabb<0>
				: axis == 1
					? compareSphereAabb<1>
					: compareSphereAabb<2>;

			m_nodes[id].left = allocNode();
			m_nodes[id].right = allocNode();

			if (count == 2)
			{
				if (comparator(spheres[start], spheres[start + 1]))
				{
					populateLeafNode(m_nodes[id].left, *spheres[start]);
					populateLeafNode(m_nodes[id].right, *spheres[start + 1]);
				}
				else
				{
					populateLeafNode(m_nodes[id].left, *spheres[start + 1]);
					populateLeafNode(m_nodes[id].right, *spheres[start]);
				}
			}
			else
			{
				std::sort(spheres.begin() + start, spheres.begin() + end, comparator);

				const auto mid = start + count / 2;

				populateInternalNode(m_nodes[id].left, spheres, start, mid);
				populateInternalNode(m_nodes[id].right, spheres, mid, end);
			}
		}
	}

	void Scene::populateLeafNode(u32 id, const Sphere &sphere)
	{
		auto &node = m_nodes[id];

		node.sphere = &sphere;
		node.aabb = sphere.aabb();
	}
}
