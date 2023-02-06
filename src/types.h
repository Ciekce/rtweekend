#pragma once

#include <cstdint>
#include <type_traits>

namespace cpurt
{
	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;

	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using f32 = float;
	using f64 = double;

	template <typename T>
	constexpr auto toUnderlying(T v)
	{
		return static_cast<std::underlying_type_t<T>>(v);
	}

	template <typename T>
	constexpr auto EnumCount = static_cast<u32>(T::_last);
}

#define CR_STRINGIFY_V(X) #X
#define CR_STRINGIFY(X) AD_STRINGIFY_V(X)
