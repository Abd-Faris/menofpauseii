// ---------------------------- - Gloomy's Escape----------------------------- //
// File:	credits.cpp
// Authors:	[Men of Pause II]
// Brief:	This file contains and displays the credits
// 
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// --- Background Assets ---
static AEGfxTexture* pSandTex = nullptr;
static AEGfxVertexList* pSandMesh = nullptr;

std::vector<GfxText> CreditTexts{
		{"Men of Pause Production",    1.f, 0, 0, 0, 255, {0, -200}},
		{"Digipen Executives\n\nPresident Claude Comair\n\nPrasanna Ghali", 1.f, 0, 0, 0, 255, {0, 0}},
		{"Instructors\n\nGerald\n\nTommy\n\nDr. Sooroor\n",    1.f, 0, 0, 0, 255, {0, 200}},
		{"Our Team\nFaris\nIzzat the goat\nShao Wei\nZi Hao\n",    1.f, 0, 0, 0, 255, {0, 400}},
		{"Assets\n\nGraphics\nPiskel\nFontMeme\nKenney\n",    1.f, 0, 0, 0, 255, {0, 600}},
		{"Audio\nBosca Ceoil\nSoundly\n",    1.f, 0, 0, 0, 255, {0, 800}},
};

void LoadCredits() {
	// 1. Load the Texture
	pSandTex = AEGfxTextureLoad("./Assets/sandbg.png");

	// 2. Create a UV-mapped mesh for the background
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pSandMesh = AEGfxMeshEnd();
}

void InitializeCredits() {
	struct AEVec2 init[] = {
		{ 0, 0 },  // Men of Pause
		// Executives
		{ 0, -400 },
		{ 0, -800 },
		{ 0, -1200 }, // Instructors
		{ 0, -1600 },
		{ 0, -2000 },

	};
	for (int i = 0; i < CreditTexts.size(); i++) {
		CreditTexts[i].pos = init[i];
	}
}
void UpdateCredits() {
	float deltaTime = (float)AEFrameRateControllerGetFrameTime();

	// Scroll up each frame
	for (GfxText& c : CreditTexts) // reference not copy
		c.pos.y += 200.f * deltaTime;

	// Go back to main menu when done
	if (CreditTexts[5].pos.y > 800.f)
		GS_next = GS_MAIN_MENU;

	// Or press any key to skip
	if (AEInputCheckTriggered(AEVK_ESCAPE))
		GS_next = GS_MAIN_MENU;
}
void DrawCredits() {

	if (pSandTex && pSandMesh) {
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxTextureSet(pSandTex, 0, 0);
		AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);

		AEMtx33 scale, trans, final;
		AEMtx33Scale(&scale, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
		AEMtx33Trans(&trans, 0.f, 0.f);
		AEMtx33Concat(&final, &trans, &scale);
		AEGfxSetTransform(final.m);
		AEGfxMeshDraw(pSandMesh, AE_GFX_MDM_TRIANGLES);
	}

	for (GfxText& c : CreditTexts)
		Gfx::printMultiline(c, boldPixels);

}

void FreeCredits() {
	if (pSandMesh) { AEGfxMeshFree(pSandMesh); pSandMesh = nullptr; }
}

void UnloadCredits() {
	if (pSandTex) { AEGfxTextureUnload(pSandTex); pSandTex = nullptr; }
}