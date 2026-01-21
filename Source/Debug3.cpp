#include "MasterHeader.h"


namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 1};
	gfxtext texts[NUM_OF_TEXTS] = {
		{"Debug 3", 0, 0, 1, 255, 255, 255, 255}
	};
}

void LoadDebug3() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void DrawDebug3() {

	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
		Graphics::printText(texts[i], boldPixels);
	}
}

void FreeDebug3() {
	// free font
	AEGfxDestroyFont(boldPixels);
}