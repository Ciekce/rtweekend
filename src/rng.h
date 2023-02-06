#pragma once

#include "types.h"

#include <optional>
#include <bit>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace cpurt
{
	class Rng // jsf32
	{
	public:
		explicit Rng(std::optional<u32> seed = {})
			: m_a{0xF1EA5EED}
		{
			if (!seed)
				seed = nextSeed();

			m_b = m_c = m_d = *seed;

			for (i32 i = 0; i < 20; ++i)
			{
				(void)nextU32();
			}
		}

		[[nodiscard]] inline u32 nextU32()
		{
			const auto e = m_a - std::rotl(m_b, 27);
			m_a = m_b ^ std::rotl(m_c, 17);
			m_b = m_c + m_d;
			m_c = m_d + e;
			m_d = e + m_a;
			return m_d;
		}

		[[nodiscard]] inline u32 nextU32(u32 range)
		{
			auto x = nextU32();
			auto m = static_cast<u64>(x) * static_cast<u64>(range);
			auto l = static_cast<u32>(m);

			if (l < range)
			{
				auto t = -range;

				if (t >= range)
				{
					t -= range;
					if (t >= range)
						t %= range;
				}

				while (l < t)
				{
					x = nextU32();
					m = static_cast<u64>(x) * static_cast<u64>(range);
					l = static_cast<u32>(m);
				}
			}

			return static_cast<u32>(m >> 32);
		}

		[[nodiscard]] inline f32 nextF32()
		{
			return static_cast<f32>(nextU32() >> 8) * 0x1.0p-24F;
		}

		[[nodiscard]] inline glm::vec3 nextVector()
		{
			return glm::vec3{nextF32() - 0.5F, nextF32() - 0.5F, nextF32() - 0.5F};
		}

		[[nodiscard]] inline glm::vec3 nextUnitOrLess()
		{
			while (true)
			{
				const auto candidate = nextVector();
				if (glm::length2(candidate) <= 1.0F)
					return candidate;
			}
		}

		[[nodiscard]] inline glm::vec3 nextUnit()
		{
			return glm::normalize(nextVector());
		}

		[[nodiscard]] inline glm::vec2 nextInUnitDisk()
		{
			while (true)
			{
				const glm::vec2 candidate{nextF32() * 2.0F - 1.0F, nextF32() * 2.0F - 1.0F};
				if (glm::length2(candidate) < 1.0F)
					return candidate;
			}
		}

		[[nodiscard]] inline glm::vec3 nextColor()
		{
			return glm::vec3{nextF32(), nextF32(), nextF32()};
		}

	private:
		static u32 nextSeed();

		u32 m_a, m_b, m_c, m_d;
	};
}
