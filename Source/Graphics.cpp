#include "MasterHeader.h"

namespace Graphics {
	void printText(gfxtext &text, s8 font) {
		// Normalise Coordinates
		f32 x = text.x / 800.f;
		f32 y = text.y / 800.f;
		// Normalise RGBA
		f32 r = text.r / 255.f;
		f32 g = text.g / 255.f;
		f32 b = text.b / 255.f;
		f32 a = text.a / 255.f;
		AEGfxPrint(font, text.text, x, y, text.scale, r, g, b, a);
	}
}

