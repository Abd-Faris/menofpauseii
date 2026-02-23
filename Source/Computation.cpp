#include "MasterHeader.h"

namespace Computation {

	//--------------- CONVERSIONS ----------------//
	
	// Normalizes RGBA
	void normalizeRGBA(f32 &r, f32 &g, f32 &b, f32 &a) {
		r = r / 255.f;
		g = g / 255.f;
		b = b / 255.f;
		a = a / 255.f;
	}	
	// Denormalizes to World Coordinates
	// overload f32, f32
	void denormalizePoint(f32 &x, f32 &y) {
		x = x * (AEGfxGetWindowWidth() / 2);
		y = y * (AEGfxGetWindowHeight() /2);
	}
	// overload AEVec2 struct
	void denormalizePoint(AEVec2 &x) {
		x.x = x.x * (AEGfxGetWindowWidth() / 2);
		x.y = x.y * (AEGfxGetWindowHeight() / 2);
	}
	// Converts Screen Coordinates to World Coordinates
	// overload f32, f32
	void screenToWorld(f32& x, f32& y) {
		x = x - (AEGfxGetWindowWidth() / 2);
		y = (AEGfxGetWindowHeight() / 2) - y;
	}
	// overload AEVec2 struct
	void screenToWorld(AEVec2 &x) {
		x.x = x.x - (AEGfxGetWindowWidth() / 2);
		x.y = (AEGfxGetWindowHeight() / 2) - x.y;
	}
	// Normalizes World Coordinates
	// overload f32, f32
	void normalizePoint(f32& x, f32& y) {
		x = (x / (AEGfxGetWindowWidth() / 2)) * 1.f;
		y = (y / (AEGfxGetWindowHeight() / 2)) * 1.f;
	}
	// overload AEVec2 struct
	void normalizePoint(AEVec2 &x) {
		x.x = (x.x / (AEGfxGetWindowWidth() / 2)) * 1.f;
		x.y = (x.y / (AEGfxGetWindowHeight() / 2)) * 1.f;
	}
	// Converts between Radian and Degree
	// degree to radian
	f32 toRadian(f32 degree) {
		return degree / 180.f * PI;
	}
	// radian to degree
	f32 toDegree(f32 radian) {
		return radian / PI * 180.f;
	}

	// Fetches Current Cursor Position in WORLD COORDS
	void getCursorPos(AEVec2& inputVec) {
		// gets mouse SCREEN coords
		s32 mouseX, mouseY;
		AEInputGetCursorPosition(&mouseX, &mouseY);
		inputVec = { (float)mouseX, (float)mouseY };
		// convert to WORLD coords
		Comp::screenToWorld(inputVec);
	}
	// Fetches Change in Cursor Position in SCREEN COORDS
	void getDeltaCursorPos(AEVec2& inputVec) {
		// gets mouse SCREEN coords
		s32 mouseX, mouseY;
		AEInputGetCursorPositionDelta(&mouseX, &mouseY);
		inputVec = { (float)mouseX, (float)mouseY };
	}
	
	//---------------- COLLISION -----------------//
	
	// AABB Bounding Box Computation
	void computeBoundingBox(AABB &box, AEVec2 &pos, AEVec2 &size, f32 scale) {
		// Calculate max coords
		box.max.x = pos.x + (size.x * 0.5f * scale);
		box.max.y = pos.y + (size.y * 0.5f * scale);
		// Calculate min coords
		box.min.x = pos.x - (size.x * 0.5f * scale);
		box.min.y = pos.y - (size.y * 0.5f * scale);
	}

	// AABB-Point Collision Detection
	// overload AEVec2 point and AABB bounding box
	bool collisionPointRect(AEVec2& pt, AABB& box) {
		// AABB Collision
		if (pt.x < box.min.x || pt.x > box.max.x ||
			pt.y < box.min.y || pt.y > box.max.y) return false;
		else return true;
	}
	// overload AEVec2 point, mesh position, and mesh size
	bool collisionPointRect(AEVec2 pt, AEVec2 rectpos, AEVec2 size) {
		// Computes bounds of the box
		AABB box{};
		Comp::computeBoundingBox(box, rectpos, size);
		// Computes AABB hit or miss
		return Comp::collisionPointRect(pt, box);
	}
}

