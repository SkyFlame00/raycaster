#pragma once

#include <cstdint>
#include <cmath>

const float ANGLE_0 = 0.0f;
const float ANGLE_1 = M_PI / 180.0f;
const float ANGLE_90 = 90.0f * ANGLE_1;
const float ANGLE_180 = 180.0f * ANGLE_1;
const float ANGLE_270 = 270.0f * ANGLE_1;
const float ANGLE_360 = 360.0f * ANGLE_1;
const float EPSILON = 1e-5f;

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

bool IsZero(float x, float epsilon = EPSILON);

bool NearZero(float x, float epsilon = EPSILON);

bool NearlyEqual(float a, float b, float epsilon = EPSILON);

float DotProduct(const Vec2& v1, const Vec2& v2);

float NormalizeAngle(float angle);

float NormalizeAngleRad(float angleRad);

