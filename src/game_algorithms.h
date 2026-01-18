#pragma once

struct Vec2;
struct Vec2i;
class Level;

bool WallRaycastH(const Vec2& origin, float angle, const Level& level, Vec2& o_HPoint, Vec2i& o_Cell);

bool WallRaycastV(const Vec2& origin, float angle, const Level& level, Vec2& o_VPoint, Vec2i& o_Cell);

bool WallRaycast(const Vec2& origin, float angle, const Level& level, Vec2& o_Point, Vec2i& o_Cell);

