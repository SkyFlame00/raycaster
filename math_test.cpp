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

int main()
{
	float r = 1.0f;
	float deg60 = DegToRad(60, r);
	float deg120 = DegToRad(120, r);
	float deg240 = DegToRad(240, r);
	float deg300 = DegToRad(300, r);

	//std::cout << "tan 60 = " << tan(deg60) << "\n";
	//std::cout << "tan 120 = " << tan(deg120) << "\n";
	//std::cout << "tan 240 = " << tan(deg240) << "\n";
	//std::cout << "tan 300 = " << tan(deg300) << "\n";

 	Convert(-390.0f);
	Convert(390.0f);
	Convert(800.0f);
	Convert(-1102.0f);
	Convert(-30.0f);

	return 0;
}
