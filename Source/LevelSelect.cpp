// -----------------------------Gloomy's Revenge---------------------------- //
// File:    LevelSelect.cpp
// Authors: [Men of Pause II]
// Brief:   Level select screen — New Game / Continue buttons, tutorial and
//          cheat toggles, and back-to-menu navigation.
//          Continue is hidden when no save file exists.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// GLOBALS
// =============================================================================

bool tutorialOn{ false };  // true when tutorial mode is enabled
bool cheatsOn{ false };  // true when cheat mode is enabled

// =============================================================================
// ANONYMOUS NAMESPACE — file-private data and helpers
// =============================================================================

namespace {

    // -------------------------------------------------------------------------
    // MESHES & TEXTURES
    // -------------------------------------------------------------------------

    AEGfxVertexList* rectMesh = nullptr;  // plain colour rect (shared by all buttons)
    AEGfxVertexList* pBgMesh = nullptr;  // full-screen background quad
    AEGfxVertexList* pBtnMesh = nullptr;  // button quad
    AEGfxTexture* pBgTex = nullptr;  // background texture
    AEGfxTexture* pBtnNormalTex = nullptr;  // button idle state
    AEGfxTexture* pBtnHoverTex = nullptr;  // button hovered state
    AEGfxTexture* pBgExitTex = nullptr;  // unused exit-confirmation background (reserved)

    // -------------------------------------------------------------------------
    // STATE
    // -------------------------------------------------------------------------

    bool hasSave = false;  // true when a save file was detected on Init; gates the Continue button

    // -------------------------------------------------------------------------
    // UI DATA
    // -------------------------------------------------------------------------

    // Button list: { position, size, mesh (assigned in Init), nextGS }
    // Negative nextGS values route to handleButton() for special actions
    std::vector<GfxButton> mainMenuButtons{
        {{0,    -50}, {300, 100}, nullptr, -3         },  // ID -3 = New Game
        {{0,     50}, {300, 100}, nullptr, -4         },  // ID -4 = Continue (hidden if no save)
        {{-200, -200}, {300, 100}, nullptr, -1        },  // ID -1 = Toggle Tutorial
        {{200,  -200}, {300, 100}, nullptr, -2        },  // ID -2 = Toggle Cheats
        {{0,   -350}, {300, 100}, nullptr, GS_MAIN_MENU}, // Back to main menu
    };

    // Text labels rendered each frame; indices [2] and [3] are updated dynamically
    std::vector<GfxText> mainMenuTexts{
        {"New Game",      0.67f, 0, 0, 0, 255, {0,    -50}},
        {"Continue",      0.67f, 0, 0, 0, 255, {0,     50}},
        {"Tutorial: OFF", 0.5f,  0, 0, 0, 255, {-200, -200}},
        {"Cheats: OFF",   0.5f,  0, 0, 0, 255, {200,  -200}},
        {"Back",          1.f,   0, 0, 0, 255, {0,   -350}},
    };

    // -------------------------------------------------------------------------
    // BUTTON HELPERS
    // -------------------------------------------------------------------------

    // TODO: btnsfx is declared but never referenced — remove or wire up
    bool btnsfx{ false };

    // ~ Brief: Draw a textured button, switching to the hover texture when the cursor is over it.
    //          Triggers a hover SFX on the first frame the cursor enters the button region.
    void drawTexturedButton(GfxButton& btn, AEVec2& mousepos) {
        bool hovered = Comp::collisionPointRect(mousepos, btn.pos, btn.size);

        // Play hover SFX on the leading edge of a hover (first frame only)
        if (hovered && !btn.hovered)
            SFX::playSFX(SFX_UI_BUTTON_HOVER);

        btn.hovered = hovered;

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(hovered ? pBtnHoverTex : pBtnNormalTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        {
            // Anon scope — matrix locals (scale, trans, final) are discarded after the draw call
            AEMtx33 scale, trans, final;
            AEMtx33Scale(&scale, btn.size.x, btn.size.y);
            AEMtx33Trans(&trans, btn.pos.x, btn.pos.y);
            AEMtx33Concat(&final, &trans, &scale);
            AEGfxSetTransform(final.m);
            AEGfxMeshDraw(pBtnMesh, AE_GFX_MDM_TRIANGLES);
        }
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);  // restore colour mode
    }

    // -------------------------------------------------------------------------
    // DRAW HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Draw all level-select buttons and text labels.
    //          The Continue button and its label are skipped entirely when no save file exists.
    void printMainMenuUI(AEVec2& mousepos) {
        for (int i = 0; i < (int)mainMenuButtons.size(); i++) {
            if (mainMenuButtons[i].nextGS == -4 && !hasSave) continue;  // hide Continue if no save
            drawTexturedButton(mainMenuButtons[i], mousepos);
        }
        for (int i = 0; i < (int)mainMenuTexts.size(); i++) {
            if (i == 1 && !hasSave) continue;  // hide Continue label if no save
            Gfx::printText(mainMenuTexts[i], boldPixels);
        }
    }

    // -------------------------------------------------------------------------
    // INPUT HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Execute the special action for a button whose nextGS is negative.
    //          Handles tutorial toggle, cheat toggle, new game, and continue.
    void handleButton(int id) {
        switch (id) {
        case -1:  // Toggle Tutorial
            tutorialOn = !tutorialOn;
            mainMenuTexts[2].text = tutorialOn ? "Tutorial: ON" : "Tutorial: OFF";
            break;

        case -2:  // Toggle Cheats
            cheatsOn = !cheatsOn;
            mainMenuTexts[3].text = cheatsOn ? "Cheats: ON" : "Cheats: OFF";
            break;

        case -3:  // New Game — wipe save and start fresh
            DeleteSave();
            hasSave = false;
            GS_next = GS_GAME;
            break;

        case -4:  // Continue — load into game only if a save file exists
            if (hasSave)
                GS_next = GS_GAME;
            break;
        }
    }

    // ~ Brief: On left-click, check each button for a hit.
    //          Negative nextGS values route to handleButton(); positive values trigger a state transition.
    void clickToNextState() {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;  // skip if left click not triggered

        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        for (GfxButton& btn : mainMenuButtons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;
            SFX::playSFX(SFX_UI_BUTTON_SELECT);

            if (btn.nextGS < 0) { handleButton(btn.nextGS); break; }  // special action
            GS_next = btn.nextGS;                                       // state transition
            break;
        }
    }

}  // namespace

// =============================================================================
// LOAD / UNLOAD
// =============================================================================

// ~ Brief: Load all textures and build meshes needed for the level-select screen.
void LoadLevelSelect() {
    pBgTex = AEGfxTextureLoad("./Assets/menu.png");
    pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
    pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");
    pBgExitTex = AEGfxTextureLoad("./Assets/gameexit.png");

    // UV-mapped mesh for the background quad
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBgMesh = AEGfxMeshEnd();

    // UV-mapped mesh for buttons (identical UV layout to background)
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBtnMesh = AEGfxMeshEnd();

    // Plain colour rect used as the shared button mesh
    rectMesh = Gfx::createRectMesh();

    // Clear any card state carried over from a previous game session
    Cards::resetCards();
}

// ~ Brief: Free all meshes and textures owned by the level-select screen to prevent memory leaks.
void UnloadLevelSelect() {
    if (rectMesh) { AEGfxMeshFree(rectMesh);           rectMesh = nullptr; }
    if (pBgMesh) { AEGfxMeshFree(pBgMesh);            pBgMesh = nullptr; }
    if (pBtnMesh) { AEGfxMeshFree(pBtnMesh);           pBtnMesh = nullptr; }
    if (pBgTex) { AEGfxTextureUnload(pBgTex);        pBgTex = nullptr; }
    if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex); pBtnNormalTex = nullptr; }
    if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);  pBtnHoverTex = nullptr; }
    if (pBgExitTex) { AEGfxTextureUnload(pBgExitTex);    pBgExitTex = nullptr; }
}

// =============================================================================
// INIT / FREE
// =============================================================================

// ~ Brief: Check for an existing save file, sync toggle labels to current state,
//          and assign the shared rect mesh to all buttons.
void InitializeLevelSelect() {
    hasSave = SaveExists();  // re-check on every entry so Continue reflects the latest state

    // Sync toggle labels to the current global state
    mainMenuTexts[2].text = tutorialOn ? "Tutorial: ON" : "Tutorial: OFF";
    mainMenuTexts[3].text = cheatsOn ? "Cheats: ON" : "Cheats: OFF";

    // Assign shared mesh to all buttons
    for (GfxButton& button : mainMenuButtons)
        button.mesh = rectMesh;
}

// ~ Brief: Nothing to free between game-state transitions for this screen.
void FreeLevelSelect() {}

// =============================================================================
// UPDATE
// =============================================================================

// ~ Brief: Poll for button clicks and handle game-state transitions.
void UpdateLevelSelect() {
    clickToNextState();
}

// =============================================================================
// DRAW
// =============================================================================

// ~ Brief: Render the level-select screen in layer order:
//          background, then the button set and text labels.
void DrawLevelSelect() {
    // Draw background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBgTex, 0, 0);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.f);
    {
        // Anon scope — matrix locals (scale, trans, final) are discarded after the draw call
        AEMtx33 scale, trans, final;
        AEMtx33Scale(&scale, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
        AEMtx33Trans(&trans, 0.f, 0.f);
        AEMtx33Concat(&final, &trans, &scale);
        AEGfxSetTransform(final.m);
        AEGfxMeshDraw(pBgMesh, AE_GFX_MDM_TRIANGLES);
    }
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);  // restore colour mode

    // Get mouse position for hover detection
    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);

    printMainMenuUI(mousepos);
}