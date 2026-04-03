// -----------------------------Gloomy's Revenge---------------------------- //
// File:    MainMenu.cpp
// Authors: [Men of Pause II]
// Brief:   Main menu screen — navigation buttons (Play, Credits, Controls,
//          Settings, Exit) and exit-confirmation overlay.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

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
    AEGfxTexture* pBgTex = nullptr;  // main menu background texture
    AEGfxTexture* pBtnNormalTex = nullptr;  // button idle state
    AEGfxTexture* pBtnHoverTex = nullptr;  // button hovered state
    AEGfxTexture* pBgExitTex = nullptr;  // exit-confirmation background texture

    // -------------------------------------------------------------------------
    // STATE
    // -------------------------------------------------------------------------

    bool exiting{};  // true when the exit-confirmation overlay is visible

    // -------------------------------------------------------------------------
    // UI DATA
    // -------------------------------------------------------------------------

    // Main menu button list: { position, size, mesh (assigned in Init), nextGS }
    // nextGS < 0 triggers special logic in exitConfirmation() instead of a state transition
    std::vector<GfxButton> mainMenuButtons{
        {{-400, -200}, {300, 100}, nullptr, GS_LEVEL_SELECT},  // Play
        {{0,    -200}, {300, 100}, nullptr, GS_CREDITS},        // Credits
        {{400,  -200}, {300, 100}, nullptr, GS_CONTROLS},       // Controls
        {{-200, -350}, {300, 100}, nullptr, GS_SETTINGS},       // Settings
        {{200,  -350}, {300, 100}, nullptr, -1}                 // Exit (special action)
    };

    // Text labels for the main menu buttons
    std::vector<GfxText> mainMenuTexts{
        {"Play",     0.9f, 0, 0, 0, 255, {-400, -200}},
        {"Credits",  0.9f, 0, 0, 0, 255, {0,    -200}},
        {"Controls", 0.9f, 0, 0, 0, 255, {400,  -200}},
        {"Settings", 0.9f, 0, 0, 0, 255, {-200, -350}},
        {"Exit",     0.9f, 0, 0, 0, 255, {200,  -350}},
    };

    // Exit-confirmation button list: { position, size, mesh, nextGS }
    // nextGS -2 routes to GS_QUIT; nextGS GS_RESTART cancels the exit
    std::vector<GfxButton> exitingButtons{
        {{-350, -200}, {400, 100}, nullptr, GS_RESTART},  // Cancel — stay in game
        {{350,  -200}, {400, 100}, nullptr, GS_QUIT}      // Confirm — quit application
    };

    // Text labels for the exit-confirmation overlay
    std::vector<GfxText> exitingTexts{
        {"Wait No!",   1.f,  0, 0, 0, 255, {-350, -200}},
        {"Yes Pls",    1.f,  0, 0, 0, 255, {350,  -200}},
        {"Exit Game?", 2.5f, 0, 0, 0, 255, {0,     280}},
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

    // ~ Brief: Draw all main menu buttons and their text labels.
    void printMainMenuUI(AEVec2& mousepos) {
        btnsfx = false;
        for (GfxButton& button : mainMenuButtons)
            drawTexturedButton(button, mousepos);
        for (GfxText& text : mainMenuTexts)
            Gfx::printText(text, boldPixels);
    }

    // ~ Brief: Draw the exit-confirmation overlay buttons and text labels.
    void printExitConfirmation(AEVec2& mousepos) {
        for (GfxButton& button : exitingButtons)
            drawTexturedButton(button, mousepos);
        for (GfxText& text : exitingTexts)
            Gfx::printText(text, boldPixels);
    }

    // -------------------------------------------------------------------------
    // INPUT HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Execute the special action for a button whose nextGS is negative.
    //          id == -1 shows the exit-confirmation overlay; id == -2 quits the application.
    void exitConfirmation(int id) {
        if (id == -1)
            exiting = true;              // show exit-confirmation overlay
        else if (id == -2)
            GS_next = GS_QUIT;          // transition to quit game state
    }

    // ~ Brief: On left-click, resolve which button set is active, check each button for a hit,
    //          and either trigger a special action or set the next game state.
    void clickToNextState() {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;  // skip if left click not triggered

        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        // Select the active button set based on overlay state
        std::vector<GfxButton>& buttons = exiting ? exitingButtons : mainMenuButtons;

        for (GfxButton& btn : buttons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

            SFX::playSFX(SFX_UI_BUTTON_SELECT);

            if (btn.nextGS < 0) { exitConfirmation(btn.nextGS); break; }  // special action
            GS_next = btn.nextGS;                                           // state transition
            break;
        }
    }

}  // namespace

// =============================================================================
// LOAD / UNLOAD
// =============================================================================

// ~ Brief: Load all textures and build meshes needed for the main menu screen.
void LoadMainMenu() {
    // Load textures
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

// ~ Brief: Free all meshes and textures owned by the main menu screen to prevent memory leaks.
void UnloadMainMenu() {
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

// ~ Brief: Reset the exit-confirmation flag, assign the shared rect mesh to all buttons,
//          and begin BGM playback.
void InitializeMainMenu() {
    exiting = false;  // ensure the exit overlay is hidden on entry

    // Assign shared mesh to main menu buttons
    for (GfxButton& button : mainMenuButtons)
        button.mesh = rectMesh;

    // Assign shared mesh to exit-confirmation buttons
    for (GfxButton& button : exitingButtons)
        button.mesh = rectMesh;

    SFX::playBGM();
}

// =============================================================================
// UPDATE
// =============================================================================

// ~ Brief: Poll for button clicks and handle game-state transitions.
void UpdateMainMenu() {
    clickToNextState();
}

// =============================================================================
// DRAW
// =============================================================================

// ~ Brief: Render the main menu in layer order:
//          background (normal or exit overlay), then the active button set and labels.
void DrawMainMenu() {
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
    AEGfxSetTransparency(1.f);

    // Draw background — swaps to the exit-confirmation texture when the overlay is active
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(exiting ? pBgExitTex : pBgTex, 0, 0);
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

    // Draw the active UI layer depending on overlay state
    if (!exiting)
        printMainMenuUI(mousepos);
    else
        printExitConfirmation(mousepos);
}