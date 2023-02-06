#include "timer.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace cpurt
{
	Timer::Timer()
	{
		u64 freq{};
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&freq));

		m_frequency = static_cast<f64>(freq);

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_initTime));
	}

	f64 Timer::time() const
	{
		u64 time{};
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&time));

		return static_cast<f64>(time - m_initTime) / m_frequency;
	}
}
#else // assume posix, untested
#include <unistd.h>

namespace cpurt
{
	Timer::Timer()
	{
		timespec time{};
		clock_gettime(CLOCK_MONOTONIC, &time);

		m_initTime = static_cast<f64>(time.tv_sec) + static_cast<f64>(time.tv_nsec) / 1000000000.0;
	}

	f64 Timer::time() const
	{
		timespec time{};
		clock_gettime(CLOCK_MONOTONIC, &time);

		return (static_cast<f64>(time.tv_sec) + static_cast<f64>(time.tv_nsec) / 1000000000.0) - m_initTime;
	}
}
#endif
