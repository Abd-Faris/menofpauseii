#include "MasterHeader.h"


namespace {
	// declares font and mesh variables
	s8 boldPixels;
	AEGfxVertexList* rectMesh;
	
	// exiting game app boolean (to display confirmation screen)
	bool exiting{};

	//enum {NUM_OF_TEXTS = 4, NUM_OF_BUTTONS = 3};
	//std::array<GfxText, NUM_OF_TEXTS>texts{ {
	//	{"Play", -400, -200, 1, 255, 255, 255, 255},
	//	{"Credits", 0, -200, 1, 255, 255, 255, 255},
	//	{"Exit", 400, -200, 1, 255, 255, 255, 255},
	//	{"Gloomy's Revenge", 0, 200, 2.5, 255, 255, 255, 255}
	//}};
	
	// init main menu buttons
	std::vector<GfxButton> mainMenuButtons{
		{{-400, -200}, {300, 100}, nullptr, GS_GAME},
		{{0, -200}, {300, 100}, nullptr},
		{{400, -200}, {300, 100}, nullptr, -1}
	};
	// init main menu texts
	std::vector<GfxText> mainMenuTexts{
		{"Play", 1.f, 255, 255, 255, 255, {-400, -200}},
		{"Credits", 1.f, 255, 255, 255, 255, {0, -200}},
		{"Exit", 1.f, 255, 255, 255, 255, {400, -200}},
		{"Gloomy's Revenge", 2.5f, 255, 255, 255, 255, {0, 200}}
	};

	// init exit confirmation button
	std::vector<GfxButton> exitingButtons{
		{{-350, -200}, {400, 100}, nullptr, GS_RESTART},
		{{350, -200}, {400, 100}, nullptr, GS_QUIT}
	};
	// init exit confirmation text
	std::vector<GfxText> exitingTexts{
		{"Wait No!", 1.f, 255, 255, 255, 255, {-350, -200}},
		{"Yes Pls", 1.f, 255, 255, 255, 255, {350, -200}},
		{"Exit Game?", 2.5f, 255, 255, 255, 255, {0, 200}}
	};

	// prints main menu UI
	void printMainMenuUI() {
		for (GfxButton &button : mainMenuButtons) {
			Gfx::printButton(button);
		}
		for (GfxText &text : mainMenuTexts) {
			Gfx::printText(text, boldPixels);
		}
	}

	void printExitConfirmation() {
		for (GfxButton &button : exitingButtons) {
			Gfx::printButton(button);
		}
		for (GfxText &text : exitingTexts) {
			Gfx::printText(text, boldPixels);
		}
	}

	void exitConfirmation(int id) {
		// if id == -1, flip boolean
		if (id == -1) exiting = true;
		// if id == -2, close app
		else if (id == -2) {
			// change to exit game state
			GS_next = GS_QUIT;
		}
	}

	// checks if button is clicked and changes to corresponding game state
	void clickToNextState() {
		// skip if left click not triggered
		if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;
		// gets mouse SCREEN coords
		s32 mouseX, mouseY;
		AEInputGetCursorPosition(&mouseX, &mouseY);
		AEVec2 mousepos{(float)mouseX, (float)mouseY};
		// convert to world coords
		Comp::screenToWorld(mousepos);

		std::vector <GfxButton>& buttons = exiting ? exitingButtons : mainMenuButtons;

		// for each button, check collision
		for (GfxButton &btn : buttons) {
			
			// if not colliding, continue
			if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

			// if less than 0, do exit screen logic
			if (btn.nextGS < 0) {
				exitConfirmation(btn.nextGS);
				break;
			}
			// if collide, set next GS to button next GS, break loop
			GS_next = btn.nextGS;
			break;
		}
		return;
	}
}

void LoadMainMenu() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
	// creates rect mesh
	rectMesh = Gfx::createRectMesh();
}

void InitializeMainMenu() {
	// Inits exiting boolean
	exiting = false;
	// Inits button meshes
	for (GfxButton &button : mainMenuButtons) {
		button.mesh = rectMesh;
	}
	for (GfxButton& button : exitingButtons) {
		button.mesh = rectMesh;
	}
}

void UpdateMainMenu() {
	// check for change in game state
	clickToNextState();
}

void DrawMainMenu() {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);
	// bg
	AEGfxSetBackgroundColor(0.52f, 0.f, 0.f);

	// if not exiting
	if (!exiting) {
		printMainMenuUI();
	}
	else {
		printExitConfirmation();
	}
}

void UnloadMainMenu() {
	// unload font
	AEGfxDestroyFont(boldPixels);
}