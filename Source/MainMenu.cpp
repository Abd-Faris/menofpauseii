#include "MasterHeader.h"


namespace {
	// declares font and mesh variables
	s8 boldPixels;
	AEGfxVertexList* rectMesh;

	//enum {NUM_OF_TEXTS = 4, NUM_OF_BUTTONS = 3};
	//std::array<GfxText, NUM_OF_TEXTS>texts{ {
	//	{"Play", -400, -200, 1, 255, 255, 255, 255},
	//	{"Credits", 0, -200, 1, 255, 255, 255, 255},
	//	{"Exit", 400, -200, 1, 255, 255, 255, 255},
	//	{"Gloomy's Revenge", 0, 200, 2.5, 255, 255, 255, 255}
	//}};
	
	std::vector<GfxButton> mainMenuButtons{
		{{-400, -200}, {300, 100}, rectMesh},
		{{0, -200}, {300, 100}, rectMesh},
		{{400, -200}, {300, 100}, rectMesh}
	};
	// init shop texts
	std::vector<GfxText> mainMenuTexts{
		{"Play", 1.f, 255, 255, 255, 255, {-400, -200}},
		{"Credits", 1.f, 255, 255, 255, 255, {0, -200}},
		{"Exit", 1.f, 255, 255, 255, 255, {400, -200}},
		{"Gloomy's Revenge", 2.5f, 255, 255, 255, 255, {0, 200}}
	};
}

void LoadMainMenu() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
	// creates rect mesh
	rectMesh = Gfx::createRectMesh();
}

void InitializeMainMenu() {
	// Inits button meshes
	for (GfxButton &button : mainMenuButtons) {
		button.mesh = rectMesh;
	}
}

void DrawMainMenu() {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);
	// gray bg
	AEGfxSetBackgroundColor(0.52f, 0.f, 0.f);

	for (GfxButton button : mainMenuButtons) {
		Gfx::printButton(button);
	}
	for (GfxText text : mainMenuTexts) {
		Gfx::printText(text, boldPixels);
	}
	
	//for (int i{}; i < NUM_OF_BUTTONS; ++i) {
	//	Gfx::printButton(buttons[i]);
	//}
	//for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
	//	//Gfx::printButton(buttons[i]);
	//	Graphics::printText(texts[i], boldPixels);
	//}
}

void FreeMainMenu() {
	// free font
	AEGfxDestroyFont(boldPixels);
}