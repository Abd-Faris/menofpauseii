#include "MasterHeader.h"

namespace Computation {

	// converts normalised to world coords
	// overload f32, f32
	f32 denormalizePoint(f32 &x, f32 &y) {
		x = x * (AEGfxGetWindowWidth() / 2);
		y = y * (AEGfxGetWindowHeight() /2);
	}
	// Overload Point struct
	f32 denormalizePoint(Point &x) {
		x.x = x.x * (AEGfxGetWindowWidth() / 2);
		x.y = x.y * (AEGfxGetWindowHeight() / 2);
	}

	// converts screen to world coords
	// overload f32, f32
	f32 screenToWorld(f32& x, f32& y) {
		x = x + (AEGfxGetWindowWidth() / 2);
		y = y + (AEGfxGetWindowHeight() /2);
	}
	// overload Point struct
	f32 screenToWorld(Point &x) {
		x.x = x.x + (AEGfxGetWindowWidth() / 2);
		x.y = x.y + (AEGfxGetWindowHeight() / 2);
	}

	// calculates squared distance for collision detection
	f32 calcSquaredDistance(Point a, Point b) {
		f32 sqDist{ ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)) };
		return sqDist;
	}

	// 
	//f32 collsionPointRect(Point mousepos, Point rect, f32 size) {
	//	f32 half = size / 2;
	//	f32 left = rect.x - half;
	//	f32 top = rect.y
	//}
}

