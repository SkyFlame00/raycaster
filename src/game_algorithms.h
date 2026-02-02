#pragma once

#include <vector>

#include "math/math.h"

struct Vec2;
struct Vec2i;
class Level;

namespace ray
{
	struct WallRaycastHit
	{
		Vec2 point;
		Vec2i cell;
	};

	void WallRaycastH(const Vec2& origin, float angle, const Level& level, std::vector<WallRaycastHit>& o_Hits);

	void WallRaycastV(const Vec2& origin, float angle, const Level& level, std::vector<WallRaycastHit>& o_Hits);

	void WallRaycast(const Vec2& origin, float angle, const Level& level, std::vector<WallRaycastHit>& o_Hits);
}

