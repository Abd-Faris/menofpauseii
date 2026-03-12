#include "MasterHeader.h"

// 1. DEFINE THE VARIABLE HERE!
bool gameWon = false;

namespace {
    s8 resultsFont;
}

void LoadResults() {
    // Load your font
    resultsFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void UpdateResults() {
    // Simple logic: Click anywhere to go back to the Main Menu
    if (AEInputCheckTriggered(AEVK_LBUTTON)) {
        GS_next = GS_MAIN_MENU;
    }
}

void DrawResults() {
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetTransparency(1.0f);

    // 2. CHECK IF WE WON OR LOST
    if (gameWon) {
        // --- VICTORY SCREEN ---
        AEGfxSetBackgroundColor(0.0f, 0.4f, 0.0f); // Green background

        GfxText winText = { "VICTORY!", 2.0f, 255, 255, 255, 255, {0.0f, 100.0f} };
        Gfx::printText(winText, resultsFont);
    }
    else {
        // --- DEFEAT SCREEN ---
        AEGfxSetBackgroundColor(0.4f, 0.0f, 0.0f); // Red background

        GfxText loseText = { "GAME OVER", 2.0f, 255, 255, 255, 255, {0.0f, 100.0f} };
        Gfx::printText(loseText, resultsFont);
    }

    // Shared text for both screens
    GfxText clickText = { "Click anywhere to return to Menu", 1.0f, 200, 200, 200, 255, {0.0f, -100.0f} };
    Gfx::printText(clickText, resultsFont);
}

void FreeResults() {
    AEGfxDestroyFont(resultsFont);
}