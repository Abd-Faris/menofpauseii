#include "MasterHeader.h"
#include <string> // Added so we can use std::to_string for stats

extern int currentWave;
AEGfxTexture* pPauseBgTex = nullptr;
AEGfxVertexList* pPauseBgMesh = nullptr;

namespace PauseScreen {
    bool isPaused = false;

    s8 pauseFont;
    AEGfxVertexList* pauseMesh = nullptr;

    GfxButton PauseBtn = { {-750, 400}, {50, 50}, nullptr, 0 };
    GfxText PauseTxt = { "||", 0.8f, 0, 0, 0, 255, {-748, 402} };

    // 1. STATS PANEL (Tightened up to match button width!)
      
    GfxButton StatsPanel = { {0, 85}, {700, 180}, nullptr, -1 };

    GfxText StatsTitle = { "-- RUN STATS --", 0.6f, 0, 0, 0, 255, {0,   200} };

    // Left column
    GfxText StatsLine1 = { "Wave: 0",      0.5f, 0, 0, 0, 255, {-150, 150} };
    GfxText StatsLine3 = { "HP: 0",        0.5f, 0, 0, 0, 255, {-150,  110} };
    GfxText StatsLine5 = { "SPD: 0",       0.5f, 0, 0, 0, 255, {-150,  70} };
    GfxText StatsLine7 = { "XP GAIN: 0",   0.5f, 0, 0, 0, 255, {-150,  30} };

    // Right column
    GfxText StatsLine2 = { "Enemies: 0",   0.5f, 0, 0, 0, 255, { 120, 150} };
    GfxText StatsLine4 = { "DMG: 0",       0.5f, 0, 0, 0, 255, { 120,  110} };
    GfxText StatsLine6 = { "F-RATE: 0/s",  0.5f, 0, 0, 0, 255, { 120,  70} };

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
        // load pause background texture
        pPauseBgTex = AEGfxTextureLoad("./Assets/pausebg.png");

        // UV mesh for pause background
        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        pPauseBgMesh = AEGfxMeshEnd();

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
            int activeEnemyCount = 0;
            for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i)
                if (enemyPool[i].alive) activeEnemyCount++;
            if (currentboss.alive) activeEnemyCount++;

            StatsLine1.text = "Wave: " + std::to_string(currentWave);
            StatsLine2.text = "Enemies left: " + std::to_string(activeEnemyCount);
            StatsLine3.text = "HP: " + std::to_string((int)calculate_max_stats(0));
            StatsLine4.text = "DMG: " + std::to_string((int)calculate_max_stats(1));
            StatsLine5.text = "SPD: " + std::to_string((int)calculate_max_stats(2));

            float shotsPerSec = 1.0f / calculate_max_stats(3);
            char frBuf[32];
            sprintf_s(frBuf, "F-RATE: %.1f/s", shotsPerSec);
            StatsLine6.text = frBuf;

            StatsLine7.text = "XP GAIN: " + std::to_string((int)calculate_max_stats(4));
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
                    //reset game stats
                    reset_game();
                    //reset card stats
                    Cards::resetCards();
                    resetTutorial();
                    tutorialOn = false;
                    cheatsOn = false;
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

        // -- draw pause background texture only over pause menu area --
        float camX, camY;
        AEGfxGetCamPosition(&camX, &camY);

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(pPauseBgTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.0f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        Gfx::printMesh(pPauseBgMesh, { camX, camY + 50.f },
            { 700.f, 700.f },
            0.f, { 0.f, 0.f }, true);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        // 1. Get Mouse Position BEFORE the loop
        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

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
        Gfx::printText(StatsLine3, pauseFont);
        Gfx::printText(StatsLine4, pauseFont);
        Gfx::printText(StatsLine5, pauseFont);
        Gfx::printText(StatsLine6, pauseFont);
        Gfx::printText(StatsLine7, pauseFont);
    }

    void FreePause() {
        AEGfxDestroyFont(pauseFont);
        if (pauseMesh) {
            AEGfxMeshFree(pauseMesh);
            pauseMesh = nullptr;
        }

        if (pPauseBgTex) { AEGfxTextureUnload(pPauseBgTex);  pPauseBgTex = nullptr; }
        if (pPauseBgMesh) { AEGfxMeshFree(pPauseBgMesh);      pPauseBgMesh = nullptr; }

    }
}