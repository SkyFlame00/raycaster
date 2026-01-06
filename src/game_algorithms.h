#pragma once

//#include "./math/math.h"

struct Vec2;
struct IVec2;
class Level;

bool FindIntersectionH(const Vec2& origin, float angle, const Level& level, Vec2& o_HPoint, IVec2& o_Cell);

bool FindIntersectionV(const Vec2& origin, float angle, const Level& level, Vec2& o_VPoint, IVec2& o_Cell);

bool FindIntersection(const Vec2& origin, float angle, const Level& level, Vec2& o_Point, IVec2& o_Cell);

