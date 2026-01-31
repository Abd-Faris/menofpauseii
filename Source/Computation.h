#pragma once
#include "MasterHeader.h"

namespace Computation {
	// normalizes RGBA
	void normalizeRGBA(f32& r, f32& g, f32& b, f32& a);
	// converts normalised to world coords
	void denormalizePoint(f32& x, f32& y);
	void denormalizePoint(AEVec2 &x);
	// converts screen coords to world coords
	void screenToWorld(f32& x, f32& y);
	void screenToWorld(AEVec2 &x);
	// normalises world coords
	void normalizePoint(f32& x, f32& y);
	void normalizePoint(AEVec2& x);
	// Collision Detection
	bool collisionPointRect(AEVec2 mousepos, AEVec2 rect, f32 sizex, f32 sizey);
}
namespace Comp = Computation;
