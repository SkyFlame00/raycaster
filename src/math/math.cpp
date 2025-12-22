#include "math.h"

float DegreesToRadians(float deg)
{
	return deg * (M_PI / 180.0f);
}

float RadiansToDegrees(float rad)
{
	return rad * (180.0f / M_PI);
}

float Distance(const Vec2& v1, const Vec2& v2)
{
	return (v1 - v2).Length();
}

