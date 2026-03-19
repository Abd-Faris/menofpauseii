#include "MasterHeader.h"

namespace PauseScreen {
    bool isPaused = false;

    s8 pauseFont;
    AEGfxVertexList* pauseMesh = nullptr;

    // ==========================================
    // 1. STACKED BUTTON LAYOUT (Grey Rectangles)
    // ==========================================
    std::vector<GfxButton> pauseButtons{
        {{0, 70}, {300, 60}, nullptr, 0},   // ID 0 = Resume
        {{0, -10}, {300, 60}, nullptr, 1},  // ID 1 = Options
        {{0, -90}, {300, 60}, nullptr, 2}   // ID 2 = Quit
    };

    // ==========================================
    // 2. TEXT STYLING
    // Format: String, Scale, R, G, B, Alpha, Position
    // ==========================================
    std::vector<GfxText> pauseTexts{
        {"Resume", 1.f, 0, 0, 0, 255, {0, 70}},         // Black Text
        {"Restart", 1.f, 0, 0, 0, 255, {0, -10}},       // Black Text
        {"Quit", 1.f, 0, 0, 0, 255, {0, -90}},          // Black Text
        {"Pause Menu", 2.5f, 255, 140, 0, 255, {0, 200}} // Orange Title
    };

    void LoadPause() {
        pauseFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
        pauseMesh = Gfx::createRectMesh("center", 0xFFFFFFFF);

        for (GfxButton& button : pauseButtons) {
            button.mesh = pauseMesh;
        }

        isPaused = false;
    }

    void UpdatePause() {
        // Toggle pause menu with ESCAPE key
        if (AEInputCheckTriggered(AEVK_ESCAPE)) {
            isPaused = !isPaused;
        }

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
                    /*isPaused = false;
                    GS_next = GS_RESTART;*/
                    // 1. Unpause the engine
                    isPaused = false;

                    // 2. Wipe stats so you don't instantly Game Over!
                    reset_game();
                    Cards::resetCards();

                    // 3. Force the engine safely back to the Game State
                    GS_next = GS_GAME;
                

                }
                else if (btn.nextGS == 2) {
                    // Quit to Main Menu
                    isPaused = false;
                    GS_next = GS_MAIN_MENU;
                }
                break;
            }
        }
    }

    void DrawPause() {
        if (!isPaused) return;

        AEGfxSetCamPosition(0.0f, 0.0f);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        // ---> NO BLEND MODE USED HERE <---
        // This guarantees everything is 100% solid and opaque.

        // --- DRAW BUTTONS ---
        // Paint the button meshes Light Grey to match the reference
        AEGfxSetColorToMultiply(0.8f, 0.8f, 0.8f, 1.0f);
        for (GfxButton& button : pauseButtons) {
            Gfx::printButton(button);
        }

        // --- DRAW TEXT ---
        // Reset the paint back to pure white so the specific RGB colors 
        // we set in the pauseTexts array (Black and Orange) render correctly!
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
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