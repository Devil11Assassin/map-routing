#include "math.h"

double math::round2(double n)
{
	return std::round(n * 100) / 100;
}