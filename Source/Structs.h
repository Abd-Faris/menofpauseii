#pragma once
#include "AEEngine.h"

struct gfxtext {
	const char* text;
	f32 x, y, scale, r, g, b, a;
};

struct Point {
	f32 x, y;
};

struct shape {

	f32 scale;			//scale
	f32 pos_x, pos_y;	// x and y positions

	AEMtx33 transform;  // Final transformation matrix for rendering
};
