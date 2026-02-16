#include "MasterHeader.h"

namespace Computation {

	// normalizes rgba
	void normalizeRGBA(f32 &r, f32 &g, f32 &b, f32 &a) {
		r = r / 255.f;
		g = g / 255.f;
		b = b / 255.f;
		a = a / 255.f;
	}
	
	// converts normalised to world coords
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

	// converts screen to world coords
	// overload f32, f32
	void screenToWorld(f32& x, f32& y) {
		x = x + (AEGfxGetWindowWidth() / 2);
		y = y + (AEGfxGetWindowHeight() /2);
	}
	// overload AEVec2 struct
	void screenToWorld(AEVec2 &x) {
		x.x = x.x + (AEGfxGetWindowWidth() / 2);
		x.y = x.y + (AEGfxGetWindowHeight() / 2);
	}

	// converts world to normalised
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

	// degree to radian
	f32 toRadian(f32 degree) {
		return degree / 180.f * PI;
	}

	// radian to degree
	f32 toDegree(f32 radian) {
		return radian / PI * 180.f;
	}

	// detects collision between point and rectangle
	bool collisionPointRect(AEVec2 mousepos, AEVec2 rect, f32 sizex, f32 sizey) {
		// calculates half of width and height
		f32 halfx = sizex / 2;
		f32 halfy = sizey / 2;
		// determines limits (x and y) for the rectangle
		f32 left = rect.x - halfx;
		f32 right = rect.x + halfx;
		f32 top = rect.y + halfy;
		f32 bottom = rect.y - halfy;
		// AABB collision
		if (mousepos.x < left || mousepos.x > right || mousepos.y > top || mousepos.y < bottom) {
			return false;
		}
		return true;
	}
}

