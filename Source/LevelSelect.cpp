#include "MasterHeader.h"

// Externs
bool tutorialOn{ false };
bool cheatsOn{ false };

namespace {
	// declares AE objects
	AEGfxVertexList* rectMesh = nullptr;
	AEGfxVertexList* pBgMesh = nullptr;
	AEGfxVertexList* pBtnMesh = nullptr;
	AEGfxTexture* pBgTex = nullptr;
	AEGfxTexture* pBtnNormalTex = nullptr;
	AEGfxTexture* pBtnHoverTex = nullptr;
	AEGfxTexture* pBgExitTex = nullptr;

	// exiting game app boolean (to display confirmation screen)
	bool exiting{};

	// init main menu buttons
	std::vector<GfxButton> mainMenuButtons{
		{{0, 100}, {300, 100}, nullptr, GS_GAME},
		{{0, -50}, {300, 100}, nullptr, -1},
		{{0, -200}, {300, 100}, nullptr, -2},
		{{400, -200}, {300, 100}, nullptr, GS_MAIN_MENU}
	};
	// init main menu texts
	std::vector<GfxText> mainMenuTexts{
		{"START",			1.f, 0, 0, 0, 255, {0, 100}},
		{"Tutorial: OFF",	.5f, 0, 0, 0, 255, {0, -50}},
		{"Cheats: OFF",		.5f, 0, 0, 0, 255, {0,-200}},
		{"Back",			1.f, 0, 0, 0, 255, {400,-200}}
	};

	// init exit confirmation button
	std::vector<GfxButton> exitingButtons{
		{{-350, -200}, {400, 100}, nullptr, GS_RESTART},
		{{350, -200}, {400, 100}, nullptr, GS_QUIT}
	};
	// init exit confirmation text
	std::vector<GfxText> exitingTexts{
		{"Wait No!", 1.f, 0, 0, 0, 255, {-350, -200}},
		{"Yes Pls",  1.f, 0, 0, 0, 255, {350,  -200}},
		{"Exit Game?", 2.5f, 0, 0, 0, 255, {0, 280}}
	};

	// draws a textured button, swapping to hover texture if mouse is over it
	void drawTexturedButton(GfxButton& btn, AEVec2& mousepos) {
		bool hovered = Comp::collisionPointRect(mousepos, btn.pos, btn.size);

		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxTextureSet(hovered ? pBtnHoverTex : pBtnNormalTex, 0, 0);
		AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.f);

		AEMtx33 scale, trans, final;
		AEMtx33Scale(&scale, btn.size.x, btn.size.y);
		AEMtx33Trans(&trans, btn.pos.x, btn.pos.y);
		AEMtx33Concat(&final, &trans, &scale);
		AEGfxSetTransform(final.m);
		AEGfxMeshDraw(pBtnMesh, AE_GFX_MDM_TRIANGLES);

		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	}

	// prints main menu UI
	void printMainMenuUI(AEVec2& mousepos) {
		for (GfxButton& button : mainMenuButtons) {
			drawTexturedButton(button, mousepos);
		}
		for (GfxText& text : mainMenuTexts) {
			Gfx::printText(text, boldPixels);
		}
	}

	void exitConfirmation(int id) {
		if (id == -1) {
			tutorialOn = !tutorialOn;
			// update button text to reflect state
			mainMenuTexts[1].text = tutorialOn ? "Tutorial: ON" : "Tutorial: OFF";
		}
		else if (id == -2) {
			cheatsOn = !cheatsOn;
			// update button text to reflect state
			mainMenuTexts[2].text = cheatsOn ? "Cheats: ON" : "Cheats: OFF";
		}
	}

	// checks if button is clicked and changes to corresponding game state
	void clickToNextState() {
		// skip if left click not triggered
		if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;
		// get cursor position
		AEVec2 mousepos{};
		Comp::getCursorPos(mousepos);

		std::vector <GfxButton>& buttons = exiting ? exitingButtons : mainMenuButtons;

		// for each button, check collision
		for (GfxButton& btn : buttons) {

			// if not colliding, continue
			if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

			// if less than 0, do non-gamestate logic
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

void LoadLevelSelect() {
	// load textures
	pBgTex = AEGfxTextureLoad("./Assets/menu.png");
	pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
	pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");
	pBgExitTex = AEGfxTextureLoad("./Assets/gameexit.png");

	// UV mapped mesh for background
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pBgMesh = AEGfxMeshEnd();

	// UV mapped mesh for buttons
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pBtnMesh = AEGfxMeshEnd();

	// creates rect mesh
	rectMesh = Gfx::createRectMesh();

	// clears cards from game session (if any)
	Cards::resetCards();
}

void InitializeLevelSelect() {
	// Inits exiting boolean
	exiting = false;
	// Inits button meshes
	for (GfxButton& button : mainMenuButtons) {
		button.mesh = rectMesh;
	}
	for (GfxButton& button : exitingButtons) {
		button.mesh = rectMesh;
	}
}

void UpdateLevelSelect() {
	// check for change in game state
	clickToNextState();
}

void DrawLevelSelect() {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);

	// draw background texture
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxTextureSet(exiting ? pBgExitTex : pBgTex, 0, 0);
	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.f);
	AEMtx33 scale, trans, final;
	AEMtx33Scale(&scale, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
	AEMtx33Trans(&trans, 0.f, 0.f);
	AEMtx33Concat(&final, &trans, &scale);
	AEGfxSetTransform(final.m);
	AEGfxMeshDraw(pBgMesh, AE_GFX_MDM_TRIANGLES);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	// get mouse position for hover detection
	AEVec2 mousepos{};
	Comp::getCursorPos(mousepos);
	printMainMenuUI(mousepos);
}

void FreeLevelSelect() {
	//
}

void UnloadLevelSelect() {

	if (rectMesh) { AEGfxMeshFree(rectMesh); rectMesh = nullptr; }
	if (pBgMesh) { AEGfxMeshFree(pBgMesh); pBgMesh = nullptr; }
	if (pBtnMesh) { AEGfxMeshFree(pBtnMesh); pBtnMesh = nullptr; }
	if (pBgTex) { AEGfxTextureUnload(pBgTex); pBgTex = nullptr; }
	if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex); pBtnNormalTex = nullptr; }
	if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);  pBtnHoverTex = nullptr; }
	if (pBgExitTex) { AEGfxTextureUnload(pBgExitTex); pBgExitTex = nullptr; }
}