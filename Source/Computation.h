#pragma once
#include "AEEngine.h"

struct Point {
	f32 x, y;
};

namespace MOPMath {
	f32 calcSquaredDistance(Point a, Point b);
}
