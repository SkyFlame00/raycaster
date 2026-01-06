#include <iostream>
#include <cmath>

#include "src/math/math.h"


int main()
{
	float res = tanf(M_PI);
	float res2 = std::tan(M_PI/2.0);
	bool res3 = std::isinf(res2);
	bool res4 = std::isinf(INFINITY);
	bool res5 = std::isnan(res2);
	std::cout << "INF=" << INFINITY << std::endl;
	return 0;
}
