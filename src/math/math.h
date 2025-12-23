#pragma once

#include <cstdint>
#include <cmath>

struct Vec2
{
	float x, y;
	
	Vec2 operator+(const Vec2& other) const
	{
		return {x + other.x, y + other.y};
	}

	Vec2 operator-(const Vec2& other) const
	{
		return {x - other.x, y - other.y};
	}

	Vec2 operator*(float scalar) const
	{
		return {scalar * x, scalar * y};
	}

	float Length()
	{
		return sqrtf(x*x + y*y);
	}
};

struct IVec2
{
	int32_t x, y;
	
	IVec2 operator+(const IVec2& other) const
	{
		return {x + other.x, y + other.y};
	}

	IVec2 operator-(const IVec2& other) const
	{
		return {x - other.x, y - other.y};
	}
};

float DegreesToRadians(float deg);

float RadiansToDegrees(float rad);

float Distance(const Vec2& v1, const Vec2& v2);

