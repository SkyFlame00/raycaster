#include "math.h"

float ToRadians(float deg)
{
	return deg * (M_PI / 180.0f);
}

float ToDegrees(float rad)
{
	return rad * (180.0f / M_PI);
}

float Distance(const Vec2& v1, const Vec2& v2)
{
	return (v1 - v2).Length();
}

// TODO: this one should be deleted and NearZero should be used instead
bool IsZero(float x, float epsilon)
{
	return std::abs(x) < epsilon;
}

bool NearZero(float x, float epsilon)
{
	return std::abs(x) < epsilon;
}

bool NearlyEqual(float a, float b, float epsilon)
{
	return std::abs(a - b) < epsilon;
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

// TODO: this should be just 'NormalizeAngle' as radians should be the default
float NormalizeAngleRad(float angleRad)
{
	angleRad = fmodf(angleRad, ANGLE_360);
	angleRad += (angleRad < 0.0f ? ANGLE_360 : 0.0f);
	return angleRad;
}

