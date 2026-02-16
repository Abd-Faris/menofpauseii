#pragma once
#include "MasterHeader.h"

// USAGE: Comp::<function-name>
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
	// converts between radian and degree
	f32 toRadian(f32 degree);
	f32 toDegree(f32 radian);
	// Collision Detection
	//void computeBoundingBox(AABB boundingbox, AEVec2 pos, AEVec2 size, f32 scale = 1.f);
	bool collisionPointRect(AEVec2 mousepos, AEVec2 rect, f32 sizex, f32 sizey);
}
namespace Comp = Computation;
