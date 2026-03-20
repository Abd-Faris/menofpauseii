#include "MasterHeader.h"

namespace PauseScreen {
    bool isPaused = false;

    s8 pauseFont;
    AEGfxVertexList* pauseMesh = nullptr;

   
    GfxButton PauseBtn = { {-750, 400}, {50, 50}, nullptr, 0 };
    GfxText PauseTxt = { "||", 0.8f, 0, 0, 0, 255, {-748, 402} };
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
        {"Pause", 2.5f, 255, 140, 0, 255, {0, 200}} // Orange Title
    };

    void LoadPause() {
        pauseFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
        pauseMesh = Gfx::createRectMesh("center", 0xFFFFFFFF);

        for (GfxButton& button : pauseButtons) {
            button.mesh = pauseMesh;
        }
        PauseBtn.mesh = pauseMesh;

        isPaused = false;
    }

    void UpdatePause() {
        // Toggle pause menu with ESCAPE key
        if (AEInputCheckTriggered(AEVK_ESCAPE)) {
            isPaused = !isPaused;
        }

        if (!isPaused) {
            if (AEInputCheckTriggered(AEVK_LBUTTON)) {
                AEVec2 mousepos{};
                Comp::getCursorPos(mousepos);

                // If they click the little top-right button, pause the game!
                if (Comp::collisionPointRect(mousepos, PauseBtn.pos, PauseBtn.size)) {
                    isPaused = true;
                }
            }
            return; // Exit here so we don't accidentally click the big menu!
        }

      
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
                    upgradeFlag = 0;
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

    void DrawPauseButton() {
        if (isPaused) return;

        // ---> REMOVED AEGfxSetCamPosition(0.0f, 0.0f) FROM HERE! <---

        // 1. Get Mouse Position
        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        // 2. The Hover Logic
        if (Comp::collisionPointRect(mousepos, PauseBtn.pos, PauseBtn.size)) {
            AEGfxSetColorToMultiply(0.9f, 0.9f, 0.9f, 1.0f); // Hover
        }
        else {
            AEGfxSetColorToMultiply(0.8f, 0.8f, 0.8f, 1.0f); // Normal
        }

        // Draw the Button Mesh
        Gfx::printUIButton(PauseBtn);

        // Draw the Black "||" Text
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::printText(PauseTxt, pauseFont);
    }

    void DrawPause() {
        if (!isPaused) return;

        // ---> REMOVED AEGfxSetCamPosition(0.0f, 0.0f) FROM HERE! <---
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        // 1. Get Mouse Position BEFORE the loop
        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        // --- DRAW BUTTONS ---
        for (GfxButton& button : pauseButtons) {

            // 2. The Hover Logic for each button
            if (Comp::collisionPointRect(mousepos, button.pos, button.size)) {
                AEGfxSetColorToMultiply(0.9f, 0.9f, 0.9f, 1.0f); // Hover
            }
            else {
                AEGfxSetColorToMultiply(0.8f, 0.8f, 0.8f, 1.0f); // Normal
            }

            Gfx::printUIButton(button);
        }

        // --- DRAW TEXT ---
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