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

bool IsZero(float x, float epsilon)
{
	return std::abs(x) < epsilon;
}

float DotProduct(const Vec2& v1, const Vec2& v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

float NormalizeAngle(float angle)
{
	angle = fmodf(angle, 360.0f);
	angle += (angle < 0 ? 360.0f : 0.0f);
	return angle;
}

