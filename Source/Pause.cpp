#include "MasterHeader.h"

namespace PauseScreen {
    // Define the variable here!
    bool isPaused = false;

    s8 pauseFont;
    AEGfxVertexList* pauseMesh = nullptr;

    std::vector<GfxButton> pauseButtons{
        {{0, 50}, {300, 100}, nullptr, 0},  // ID 0 = Resume
        {{0, -100}, {300, 100}, nullptr, 1} // ID 1 = Quit to Main Menu
    };

    std::vector<GfxText> pauseTexts{
        {"Resume", 1.f, 255, 255, 255, 255, {0, 50}},
        {"Main Menu", 1.f, 255, 255, 255, 255, {0, -100}},
        {"PAUSED", 2.5f, 255, 255, 255, 255, {0, 200}}
    };

    void LoadPause() {
        pauseFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
        pauseMesh = Gfx::createRectMesh();
        for (GfxButton& button : pauseButtons) {
            button.mesh = pauseMesh;
        }

        // Ensure it starts unpaused
        isPaused = false;
    }

    void UpdatePause() {
        // Toggle pause menu with ESCAPE key
        if (AEInputCheckTriggered(AEVK_ESCAPE)) {
            isPaused = !isPaused;
        }

        // Only check for clicks if the pause menu is open
        if (!isPaused) return;
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        // Button Click Logic
        for (GfxButton& btn : pauseButtons) {
            if (Comp::collisionPointRect(mousepos, btn.pos, btn.size)) {
                if (btn.nextGS == 0) {
                    // Resume
                    isPaused = false;
                }
                else if (btn.nextGS == 1) {
                    // Quit to Main Menu
                    isPaused = false; // Reset for next time
                    GS_next = GS_MAIN_MENU;
                }
                break;
            }
        }
    }

    void DrawPause() {
        // Only draw if paused
        if (!isPaused) return;

        AEGfxSetCamPosition(0.0f, 0.0f);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        // Dark dimming overlay
        AEGfxSetTransparency(0.7f);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
        Gfx::printMesh(pauseMesh, { 0.f, 0.f }, { 2000.f, 2000.f }, 0.f);

        // Buttons
        AEGfxSetTransparency(1.0f);
        for (GfxButton& button : pauseButtons) {
            Gfx::printButton(button);
        }

        // Text
        for (GfxText& text : pauseTexts) {
            Gfx::printText(text, pauseFont);
        }
    }

    void FreePause() {
        AEGfxDestroyFont(pauseFont);
        if (pauseMesh) {
            AEGfxMeshFree(pauseMesh);
            pauseMesh = nullptr;
        }
    }
}