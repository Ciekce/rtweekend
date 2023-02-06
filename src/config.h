#pragma once

#include "types.h"

namespace cpurt
{
	constexpr u32 Width = 1200;
	constexpr u32 Height = 800;

	constexpr u32 Samples = 500;
	constexpr u32 Bounces = 50;

	constexpr u32 Threads = 0; // 0 for core count
	constexpr u32 TileSize = 16;

	constexpr f32 Gamma = 2.2F;
}
