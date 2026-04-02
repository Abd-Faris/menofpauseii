#include "MasterHeader.h"

// Global flag to determine screen state
bool gameWon = false;

namespace {
	// declares AE objects
	AEGfxVertexList* pBgMesh = nullptr;
	AEGfxVertexList* pBtnMesh = nullptr;
	AEGfxTexture* pBgWinTex = nullptr;
	AEGfxTexture* pBgLoseTex = nullptr;
	AEGfxTexture* pBtnNormalTex = nullptr;
	AEGfxTexture* pBtnHoverTex = nullptr;

	// --- VICTORY UI ---
	std::vector<GfxButton> winButtons{
		{{-200, -200}, {300, 100}, nullptr, GS_CARD_SHOP},
		{{200, -200}, {300, 100}, nullptr, GS_MAIN_MENU}
	};
	std::vector<GfxText> winButtonTexts{
		{"Continue",  0.75f, 0, 0, 0, 255, {-200, -200}},
		{"Main Menu", 0.75f, 0, 0, 0, 255, {200,  -200}},
		{"VICTORY!",  2.5f, 0, 255, 0, 255, {0, 50}}
	};

	// --- DEFEAT UI ---
	std::vector<GfxButton> loseButtons{
		{{0, -200}, {300, 100}, nullptr, GS_MAIN_MENU}
	};
	std::vector<GfxText> loseButtonTexts{
		{"Main Menu", 0.75f, 0, 0, 0, 255, {0, -200}},
		{"GAME OVER", 2.5f, 255, 0, 0, 255, {0, 50}}
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

	void printResultsUI(AEVec2& mousepos) {
		std::vector<GfxButton>& buttons = gameWon ? winButtons : loseButtons;
		std::vector<GfxText>& texts = gameWon ? winButtonTexts : loseButtonTexts;

		for (GfxButton& button : buttons) {
			drawTexturedButton(button, mousepos);
		}
		for (GfxText& text : texts) {
			Gfx::printText(text, boldPixels);
		}
	}

	void clickToNextState() {
		if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

		AEVec2 mousepos{};
		Comp::getCursorPos(mousepos);

		std::vector<GfxButton>& buttons = gameWon ? winButtons : loseButtons;

		for (GfxButton& btn : buttons) {
			if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

			// Logic for resetting game data when returning to menu
			if (btn.nextGS == GS_MAIN_MENU) {
				reset_game();
				Cards::resetCards();
				resetTutorial();
			}

			GS_next = btn.nextGS;
			break;
		}
	}
}

void LoadResults() {
	// Load textures
	pBgWinTex = AEGfxTextureLoad("./Assets/victory.png");
	pBgLoseTex = AEGfxTextureLoad("./Assets/lose.png");
	pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
	pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");

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
}

void InitializeResults() {
	// Assign meshes to buttons
	for (GfxButton& button : winButtons) {
		button.mesh = pBtnMesh;
	}
	for (GfxButton& button : loseButtons) {
		button.mesh = pBtnMesh;
	}
}

void UpdateResults() {
	clickToNextState();
}

void DrawResults() {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);

	// Draw Background based on win/loss
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxTextureSet(gameWon ? pBgWinTex : pBgLoseTex, 0, 0);
	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	AEMtx33 scale, trans, final;
	AEMtx33Scale(&scale, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
	AEMtx33Trans(&trans, 0.f, 0.f);
	AEMtx33Concat(&final, &trans, &scale);
	AEGfxSetTransform(final.m);
	AEGfxMeshDraw(pBgMesh, AE_GFX_MDM_TRIANGLES);

	AEVec2 mousepos{};
	Comp::getCursorPos(mousepos);

	printResultsUI(mousepos);
}

void FreeResults() {
	if (pBgMesh) { AEGfxMeshFree(pBgMesh);            pBgMesh = nullptr; }
	if (pBtnMesh) { AEGfxMeshFree(pBtnMesh);           pBtnMesh = nullptr; }
	if (pBgWinTex) { AEGfxTextureUnload(pBgWinTex);      pBgWinTex = nullptr; }
	if (pBgLoseTex) { AEGfxTextureUnload(pBgLoseTex);     pBgLoseTex = nullptr; }
	if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex);  pBtnNormalTex = nullptr; }
	if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);   pBtnHoverTex = nullptr; }
}