#include <iostream>
#include <cmath>

float DegToRad(float deg, float r)
{
	return deg * ((M_PI * r) / 180.0f);
}

struct Vec2
{
	float x;
	float y;

	Vec2 operator+(const Vec2& other)
	{
		return {x + other.x, y + other.y};
	}

	Vec2 operator-(const Vec2& other)
	{
		return {x - other.x, y - other.y};
	}
};

float StdAngle(float angle)
{
	float x = fmodf(angle, 360.0f);
	x = x + (x < 0.0 ? 360.0f : 0.0f);
	return x;
}

void PrintAngle(float angleFrom, float angleTo)
{
	std::cout << angleFrom << " => " << angleTo << std::endl;
}

void Convert(float angle)
{
	PrintAngle(angle, StdAngle(angle));
}

float near = 20.0f;
float far = 700.0f;
float DistNorm(float dist, float near, float far)
{
	return (dist - near) / (far - near);
}

float HeightRatio(float dist, float near, float far)
{
	return 1.0f - DistNorm(dist, near, far);
}

int main()
{
	for (float dist = 20.0f; dist < 301.0f; dist += 10.0f)
	{
		float k = near / dist;

		std::printf("Dist=%.2f, HeightRatio=%.2f, k=%.2f\n", dist, HeightRatio(dist, 20.0f, 300.0f), k);
	}

	std::printf("\n\n====================\n\n");
	for (float dist = 20.0f; dist < 701.0f; dist += 10.0f)
	{
		float k = near / dist;

		std::printf("Dist=%.2f, HeightRatio=%.2f, k=%.2f\n", dist, HeightRatio(dist, 20.0f, 700.0f), k);
	}
	return 0;
}
