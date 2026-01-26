#include "MasterHeader.h"

namespace Computation {

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

	// calculates squared distance between 2 points
	f32 calcSquaredDistance(Point a, Point b) {
		f32 sqDist{ ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)) };
		return sqDist;
	}

	// detects collision between point and rectangle
	void collsionPointRect(Point mousepos, Point rect, f32 size) {
		f32 half = size / 2;
		f32 left = rect.x - half;
		f32 top = rect.y;
	}
}

