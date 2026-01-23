#pragma once
#include "AEEngine.h"

struct gfxtext {
	const char* text;
	f32 x, y, scale, r, g, b, a;
};

struct Point {
	f32 x, y;
};

namespace Graphics {
	void printText(gfxtext &text, s8 font);
}
