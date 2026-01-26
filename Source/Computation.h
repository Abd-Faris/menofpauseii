#pragma once
#include "MasterHeader.h"

namespace Computation {
	void denormalizePoint(f32& x, f32& y);
	void denormalizePoint(AEVec2 &x);
	void screenToWorld(f32& x, f32& y);
	void screenToWorld(AEVec2 &x);
	void normalizePoint(f32& x, f32& y);
	void normalizePoint(AEVec2& x);
	f32 calcSquaredDistance(Point a, Point b);
}
namespace Comp = Computation;
