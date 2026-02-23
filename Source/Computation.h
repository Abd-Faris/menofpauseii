#pragma once
#include "MasterHeader.h"

// USAGE: Comp::<function-name>
namespace Computation {
	//--------------- CONVERSIONS ----------------//

	// Normalizes RGBA
	void normalizeRGBA(f32& r, f32& g, f32& b, f32& a);
	// Denormalizes to World Coordinates
	void denormalizePoint(f32& x, f32& y);
	void denormalizePoint(AEVec2 &x);
	// Converts Screen Coordinates to World Coordinates
	void screenToWorld(f32& x, f32& y);
	void screenToWorld(AEVec2 &x);
	// Normalizes World Coordinates
	void normalizePoint(f32& x, f32& y);
	void normalizePoint(AEVec2& x);
	// Converts between Radian and Degree
	f32 toRadian(f32 degree);
	f32 toDegree(f32 radian);

	// Fetches Current Cursor Position in WORLD COORDS
	void getCursorPos(AEVec2& inputVec);
	void getDeltaCursorPos(AEVec2& inputVec);

	//---------------- COLLISION -----------------//
	
	// AABB Bounding Box Computation
	void computeBoundingBox(AABB& box, AEVec2& pos, AEVec2& size, f32 scale = 10);

	// AABB-Point Collision Detection
	bool collisionPointRect(AEVec2& pt, AABB& box);
	bool collisionPointRect(AEVec2 mousepos, AEVec2 rectpos, AEVec2 size);
}
namespace Comp = Computation;
