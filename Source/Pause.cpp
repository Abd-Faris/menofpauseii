#include "MasterHeader.h"
#include <string> // Added so we can use std::to_string for stats

namespace PauseScreen {
    bool isPaused = false;

    s8 pauseFont;
    AEGfxVertexList* pauseMesh = nullptr;

    GfxButton PauseBtn = { {-750, 400}, {50, 50}, nullptr, 0 };
    GfxText PauseTxt = { "||", 0.8f, 0, 0, 0, 255, {-748, 402} };

    // ==========================================
       // 1. STATS PANEL (Tightened up to match button width!)
      
       // ==========================================
    GfxButton StatsPanel = { {0, 110}, {700, 200}, nullptr, -1 };

    // Stats Text (Nudged slightly to sit perfectly centered in the new box)
    // Scaled the title down to 1.0f so it doesn't bleed over the edges
    GfxText StatsTitle = { "-- RUN STATS --", 1.0f, 0, 0, 0, 255, {0, 155} };
    GfxText StatsLine1 = { "Current Wave: 0", 1.0f, 0, 0, 0, 255, {0, 110} };
    GfxText StatsLine2 = { "Enemies Left: 0", 1.0f, 0, 0, 0, 255, {0, 65} };
    // ==========================================
    // 2. STACKED BUTTON LAYOUT
    // ==========================================
    std::vector<GfxButton> pauseButtons{
        {{0, -60}, {300, 60}, nullptr, 0},   // ID 0 = Resume
        {{0, -140}, {300, 60}, nullptr, 2}   // ID 2 = Quit
    };

    // ==========================================
    // 3. TEXT STYLING
    // ==========================================
    std::vector<GfxText> pauseTexts{
        {"Resume", 1.f, 0, 0, 0, 255, {0, -60}},         // Black Text
        {"Quit", 1.f, 0, 0, 0, 255, {0, -140}},          // Black Text
        {"Pause", 2.5f, 255, 140, 0, 255, {0, 280}}      // Orange Title (Pushed higher!)
    };
    void LoadPause() {
        pauseFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
        pauseMesh = Gfx::createRectMesh(0xFFFFFFFF);

        // Apply mesh to UI buttons
        for (GfxButton& button : pauseButtons) {
            button.mesh = pauseMesh;
        }

        // Apply mesh to HUD and Stats panel
        PauseBtn.mesh = pauseMesh;
        StatsPanel.mesh = pauseMesh;

        isPaused = false;
    }

    void UpdatePause() {
        // Toggle pause menu with ESCAPE key
        if (AEInputCheckTriggered(AEVK_ESCAPE)) {
            isPaused = !isPaused;
        }

        // ==========================================
        // DYNAMIC STATS UPDATE
        // Change the text to reflect your actual game variables!
        // ==========================================
        if (isPaused) {
            // Example of how to connect your game data:
            // StatsLine1.text = "Current Wave: " + std::to_string(Cards::currentWave);
            // StatsLine2.text = "Enemies Left: " + std::to_string(Cards::enemiesRemaining);
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
                // NOTE: Restart (btn.nextGS == 1) has been removed!
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

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        // 1. Get Mouse Position BEFORE the loop
        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        // --- DRAW THE STATS PANEL ---
        // Give it a slightly darker grey color than the buttons so it stands out
        AEGfxSetColorToMultiply(0.7f, 0.7f, 0.7f, 1.0f);
        Gfx::printUIButton(StatsPanel);

        // --- DRAW BUTTONS ---
        for (GfxButton& button : pauseButtons) {
            // Hover Logic for clickable buttons
            if (Comp::collisionPointRect(mousepos, button.pos, button.size)) {
                AEGfxSetColorToMultiply(0.9f, 0.9f, 0.9f, 1.0f); // Hover
            }
            else {
                AEGfxSetColorToMultiply(0.8f, 0.8f, 0.8f, 1.0f); // Normal
            }
            Gfx::printUIButton(button);
        }

        // --- DRAW TEXT ---
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f); // Reset to pure white for text

        // Draw Button Text & Title
        for (GfxText& text : pauseTexts) {
            Gfx::printText(text, pauseFont);
        }

        // Draw Stats Text
        Gfx::printText(StatsTitle, pauseFont);
        Gfx::printText(StatsLine1, pauseFont);
        Gfx::printText(StatsLine2, pauseFont);
    }

    void FreePause() {
        AEGfxDestroyFont(pauseFont);
        if (pauseMesh) {
            AEGfxMeshFree(pauseMesh);
            pauseMesh = nullptr;
        }
    }
}