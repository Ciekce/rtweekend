#pragma once

#include "types.h"

namespace cpurt
{
	class Timer
	{
	public:
		Timer();
		~Timer() = default;

		[[nodiscard]] f64 time() const;

	private:
#ifdef _WIN32
		u64 m_initTime{};
		f64 m_frequency;
#else
		f64 m_initTime;
#endif
	};
}
