#include "rng.h"

#include <mutex>
#include <chrono>

namespace cpurt
{
	namespace
	{
		class SeedGenerator // sfc32
		{
		public:
			explicit SeedGenerator(u32 seed)
				: m_a{seed},
				  m_b{seed},
				  m_c{seed},
				  m_d{1}
			{
				for (i32 i = 0; i < 12; ++i)
				{
					(void)nextU32();
				}
			}

			[[nodiscard]] inline u32 nextU32()
			{
				const auto e = m_a + m_b + m_d++;
				m_a = m_b ^ (m_b >> 9);
				m_b = m_c + (m_c << 3);
				m_c = std::rotl(m_c, 21) + e;
				return e;
			}

		private:
			u32 m_a;
			u32 m_b;
			u32 m_c;
			u32 m_d;
		};
	}

	u32 Rng::nextSeed()
	{
		static std::mutex mutex{};
		static SeedGenerator generator{0x69C6278F};

		u32 seed;

		{
			std::scoped_lock lock{mutex};
			seed = generator.nextU32();
		}

		seed ^= static_cast<u32>(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count());

		return seed;
	}
}
