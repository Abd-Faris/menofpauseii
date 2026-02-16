#include "MasterHeader.h"


namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 1};
	GfxText texts[NUM_OF_TEXTS] = {
		{"Debug 5"}
	};
}

void LoadDebug5() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void DrawDebug5() {

	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	for (GfxText text : texts) Gfx::printText(text, boldPixels);
}

void FreeDebug5() {
	// free font
	AEGfxDestroyFont(boldPixels);
}