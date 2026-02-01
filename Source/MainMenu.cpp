#include "MasterHeader.h"


namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 4, NUM_OF_BUTTONS = 3};
	std::array<gfxtext, NUM_OF_TEXTS>texts{ {
		{"Play", -400, -200, 1, 255, 255, 255, 255},
		{"Credits", 0, -200, 1, 255, 255, 255, 255},
		{"Exit", 400, -200, 1, 255, 255, 255, 255},
		{"<pending-name>", 0, 200, 3, 255, 255, 255, 255}
	}};
	std::array<gfxbutton, NUM_OF_BUTTONS>buttons{};
}

void LoadMainMenu() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void InitializeMainMenu() {
	// Inits buttons
	for (int i{}; i < NUM_OF_BUTTONS; ++i) {
		buttons[i].mesh = Gfx::createRectMesh();
		buttons[i].text = texts[i];
		buttons[i].pos = {texts[i].x, texts[i].y};
		buttons[i].size = {300.f, 100.f};
	}
}

void DrawMainMenu() {

	// gray bg
	AEGfxSetBackgroundColor(0.52f, 0.f, 0.f);
	for (int i{}; i < NUM_OF_BUTTONS; ++i) {
		Gfx::printButton(buttons[i]);
	}
	for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
		//Gfx::printButton(buttons[i]);
		Graphics::printText(texts[i], boldPixels);
	}
}

void FreeMainMenu() {
	// free font
	AEGfxDestroyFont(boldPixels);
}