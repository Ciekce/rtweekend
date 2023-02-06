#pragma once

#include "types.h"

#include <glm/vec3.hpp>

#include "ray.h"
#include "rng.h"

namespace cpurt
{
	class Camera
	{
	public:
		explicit Camera(u32 width, u32 height, f32 fovY, f32 aperture, f32 focalLength);
		~Camera() = default;

		inline void width(u32 width) { m_width = width; }
		[[nodiscard]] inline auto width() const { return m_width; }

		inline void height(u32 height) { m_height = height; }
		[[nodiscard]] inline auto height() const { return m_height; }

		inline void fovY(f32 fovY) { m_fovY = fovY; }
		[[nodiscard]] inline auto fovY() const { return m_fovY; }

		[[nodiscard]] inline auto &pos() { return m_pos; }
		[[nodiscard]] inline const auto &pos() const { return m_pos; }

		[[nodiscard]] inline auto &target() { return m_target; }
		[[nodiscard]] inline const auto &target() const { return m_target; }

		void update();

		[[nodiscard]] inline Ray ray(Rng &rng, u32 x, u32 y) const
		{
			const auto origin = m_lensRadius * rng.nextInUnitDisk();
			const auto offset = m_u * origin.x + m_v * origin.y;

			const auto u = static_cast<f32>(x) * m_invSize.x;
			const auto v = static_cast<f32>(m_height - y - 1) * m_invSize.y;

			return {
				.origin = m_pos + offset,
				.dir = m_lowerLeft + u * m_horizontal + v * m_vertical - offset
			};
		}

	private:
		u32 m_width, m_height;
		f32 m_fovY;

		f32 m_aperture;
		f32 m_focalLength;

		glm::vec3 m_pos{};
		glm::vec3 m_target{0.0F, 0.0F, -1.0F};

		glm::vec2 m_invSize{};

		glm::vec3 m_u{};
		glm::vec3 m_v{};
		glm::vec3 m_w{};

		glm::vec3 m_horizontal{};
		glm::vec3 m_vertical{};

		glm::vec3 m_lowerLeft{};

		f32 m_lensRadius{};
	};
}
