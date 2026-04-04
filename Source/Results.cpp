// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Results.cpp
// Authors: [Men of Pause II]
// Brief:   Implements the results screen shown after a win or loss.
//          Displays a victory or game over background with appropriate buttons,
//          and handles navigation back to the main menu or card shop.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// ~ Brief: Global flag set by the game state before transitioning to results.
//          true = player won, false = player lost. Drives which UI is shown.
bool gameWon = false;

namespace {
    // ~ Brief: Shared meshes and textures for the results screen background and buttons.
    AEGfxVertexList* pBgMesh = nullptr;
    AEGfxVertexList* pBtnMesh = nullptr;
    AEGfxTexture* pBgWinTex = nullptr;
    AEGfxTexture* pBgLoseTex = nullptr;
    AEGfxTexture* pBtnNormalTex = nullptr;
    AEGfxTexture* pBtnHoverTex = nullptr;

    // -------------------------------------------------------------------------
    // VICTORY UI
    // -------------------------------------------------------------------------

    // ~ Brief: Buttons shown on the victory screen.
    //          Continue advances to the card shop; Main Menu returns to the main menu.
    std::vector<GfxButton> winButtons{
        {{-200, -200}, {300, 100}, nullptr, GS_CARD_SHOP},
        {{ 200, -200}, {300, 100}, nullptr, GS_MAIN_MENU}
    };

    // ~ Brief: Text labels for the victory screen buttons and title.
    std::vector<GfxText> winButtonTexts{
        {"Continue",  0.75f, 0,   0,   0,   255, {-200, -200}},
        {"Main Menu", 0.75f, 0,   0,   0,   255, { 200, -200}},
        {"VICTORY!",  2.5f,  0,   255, 0,   255, {0,     50}}
    };

    // -------------------------------------------------------------------------
    // DEFEAT UI
    // -------------------------------------------------------------------------

    // ~ Brief: Buttons shown on the defeat screen.
    //          Only option is to return to the main menu.
    std::vector<GfxButton> loseButtons{
        {{0, -200}, {300, 100}, nullptr, GS_MAIN_MENU}
    };

    // ~ Brief: Text labels for the defeat screen button and title.
    std::vector<GfxText> loseButtonTexts{
        {"Main Menu", 0.75f, 0,   0, 0, 255, {0, -200}},
        {"GAME OVER", 2.5f,  255, 0, 0, 255, {0,    50}}
    };

    // -------------------------------------------------------------------------
    // DRAW HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Draw a single textured button, swapping to the hover texture
    //          when the mouse cursor is over it.
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

    // ~ Brief: Draw all buttons and text for the current result screen.
    //          Selects win or lose UI based on the gameWon flag.
    void printResultsUI(AEVec2& mousepos) {
        std::vector<GfxButton>& buttons = gameWon ? winButtons : loseButtons;
        std::vector<GfxText>& texts = gameWon ? winButtonTexts : loseButtonTexts;

        for (GfxButton& button : buttons)
            drawTexturedButton(button, mousepos);
        for (GfxText& text : texts)
            Gfx::printText(text, boldPixels);
    }

    // -------------------------------------------------------------------------
    // INPUT HANDLING
    // -------------------------------------------------------------------------

    // ~ Brief: Check for button clicks and transition to the appropriate game state.
    //          When returning to the main menu, resets all game and card state,
    //          clears the tutorial flag, and deletes the save file on a loss.
    void clickToNextState() {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        std::vector<GfxButton>& buttons = gameWon ? winButtons : loseButtons;

        for (GfxButton& btn : buttons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

            // Clean up all run state before returning to the main menu
            if (btn.nextGS == GS_MAIN_MENU) {
                reset_game();
                Cards::resetCards();
                resetTutorial();
                if (!gameWon) DeleteSave(); // only delete save on loss — win save handled elsewhere
                gamecurrrun = false;
            }

            GS_next = btn.nextGS;
            break;
        }
    }

} // namespace

// =============================================================================
// LOAD / FREE
// =============================================================================

// ~ Brief: Load win and loss background textures, button textures, and build
//          the UV-mapped unit quad meshes for the background and buttons.
void LoadResults() {
    pBgWinTex = AEGfxTextureLoad("./Assets/victory.png");
    pBgLoseTex = AEGfxTextureLoad("./Assets/lose.png");
    pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
    pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");

    // Background mesh
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBgMesh = AEGfxMeshEnd();

    // Button mesh
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBtnMesh = AEGfxMeshEnd();
}

// ~ Brief: Assign the button mesh to all win and lose buttons.
//          Called each time the results state is entered.
void InitializeResults() {
    for (GfxButton& button : winButtons)  button.mesh = pBtnMesh;
    for (GfxButton& button : loseButtons) button.mesh = pBtnMesh;
}

// ~ Brief: Poll for button clicks and handle state transitions.
void UpdateResults() {
    clickToNextState();
}

// ~ Brief: Draw the win or lose background texture scaled to fill the window,
//          then draw the appropriate buttons and text on top.
void DrawResults() {
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
    AEGfxSetTransparency(1.f);

    // Full-screen background — victory or defeat texture based on outcome
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

    // Buttons and text
    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);
    printResultsUI(mousepos);
}

// ~ Brief: Free all meshes and unload all textures used by the results screen.
//          All pointers are set to nullptr after freeing to prevent double-free.
void FreeResults() {
    if (pBgMesh) { AEGfxMeshFree(pBgMesh);              pBgMesh = nullptr; }
    if (pBtnMesh) { AEGfxMeshFree(pBtnMesh);             pBtnMesh = nullptr; }
    if (pBgWinTex) { AEGfxTextureUnload(pBgWinTex);       pBgWinTex = nullptr; }
    if (pBgLoseTex) { AEGfxTextureUnload(pBgLoseTex);      pBgLoseTex = nullptr; }
    if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex);   pBtnNormalTex = nullptr; }
    if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);    pBtnHoverTex = nullptr; }
}