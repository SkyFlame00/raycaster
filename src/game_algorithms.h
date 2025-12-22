#pragma once

//#include "./math/math.h"

struct Vec2;
struct IVec2;
class Level;

bool FindHIntersectionPoint(const Vec2& origin, float angleRad, const Level& level, Vec2& o_VPoint, IVec2& o_Cell);

bool FindVIntersectionPoint(const Vec2& origin, float angleRad, const Level& level, Vec2& o_VPoint, IVec2& o_Cell);

bool FindIntersectionPoint(const Vec2& origin, float angleRad, const Level& level, Vec2& o_Point, IVec2& o_Cell);

