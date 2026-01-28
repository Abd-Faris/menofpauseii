#include "MasterHeader.h"


namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 1};
	gfxtext texts[NUM_OF_TEXTS] = {
		{"Card Screen", 0, 0, 1, 255, 255, 255, 255}
	};
}

void LoadDebug4() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void InitializeDebug4() {
	
}

void DrawDebug4() {
	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
		Graphics::printText(texts[i], boldPixels);
	}
}

void FreeDebug4() {
	// free font
	AEGfxDestroyFont(boldPixels);
}