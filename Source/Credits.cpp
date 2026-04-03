// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Credits.cpp
// Authors: [Men of Pause II]
// Brief:   Handles the loading, scrolling logic, rendering, and cleanup of 
//          the end-game credits sequence.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// STATIC GLOBALS
// =============================================================================

// ~ Brief: Texture and mesh used for the credits menu background.
static AEGfxTexture* pSandTex = nullptr;
static AEGfxVertexList* pSandMesh = nullptr;

// ~ Brief: Collection of credit strings, styling, and initial position offsets.
std::vector<GfxText> CreditTexts{
		{"Men of Pause Production",    1.f, 0, 0, 0, 255},
		{"Digipen Executives\n\nPresident Claude Comair\n\nPrasanna Ghali", 1.f, 0, 0, 0, 255},
		{"Instructors\n\nGerald\n\nTommy\n\nDr. Sooroor\n",    1.f, 0, 0, 0, 255},
		{"Our Team\n\nFaris\n\nIzzat the goat\n\nShao Wei\n\nZi Hao\n",    1.f, 0, 0, 0, 255},
		{"Assets\n\n\nGraphics\n\nPiskel\n\nFontMeme\n\nKenney\n",    1.f, 0, 0, 0, 255},
		{"Audio\n\nBosca Ceoil\n\nSoundly\n",    1.f, 0, 0, 0, 255},
};

// =============================================================================
// FUNCTIONS
// =============================================================================

// ~ Brief: Load the background sand texture and build the full-screen quad mesh.
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

// ~ Brief: Reset the positions of all credit text blocks to their starting scroll values.
void InitializeCredits() {
	struct AEVec2 init[] = {
		{ 0, 0 },
		{ 0, -400 },
		{ 0, -800 },
		{ 0, -1300 },
		{ 0, -1900 },
		{ 0, -2500 },

	};
	for (int i = 0; i < CreditTexts.size(); i++) {
		CreditTexts[i].pos = init[i];
	}
}

// ~ Brief: Update scroll positions and handle input for skipping or exiting the credits.
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

// ~ Brief: Render the background texture and the scrolling multi-line text blocks.
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

// ~ Brief: Free the background mesh from memory.
void FreeCredits() {
	if (pSandMesh) { AEGfxMeshFree(pSandMesh); pSandMesh = nullptr; }
}

// ~ Brief: Unload the background texture from memory.
void UnloadCredits() {
	if (pSandTex) { AEGfxTextureUnload(pSandTex); pSandTex = nullptr; }
}