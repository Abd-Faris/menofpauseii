#pragma once
#include "MasterHeader.h"

namespace Computation {
	f32 denormalizePoint(f32& x, f32& y);
	f32 denormalizePoint(Point &x);
	f32 screenToWorld(f32& x, f32& y);
	f32 screenToWorld(Point & x);
	f32 calcSquaredDistance(Point a, Point b);
	//f32 collisionAABB(Point a, Point b);
}
