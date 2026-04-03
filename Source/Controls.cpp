// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Controls.cpp
// Authors: [Men of Pause II]
// Brief:   Controls screen — displays key bindings and provides a
//          back-to-menu button.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// ANONYMOUS NAMESPACE — file-private data and helpers
// =============================================================================

namespace {

    // -------------------------------------------------------------------------
    // MESHES & TEXTURES
    // -------------------------------------------------------------------------

    AEGfxVertexList* pBgMesh = nullptr;  // full-screen background quad
    AEGfxVertexList* pBtnMesh = nullptr;  // button quad
    AEGfxVertexList* rectMesh = nullptr;  // plain colour rect (shared by all buttons)
    AEGfxTexture* pBgTex = nullptr;  // background texture
    AEGfxTexture* pBtnNormalTex = nullptr;  // button idle state
    AEGfxTexture* pBtnHoverTex = nullptr;  // button hovered state

    // -------------------------------------------------------------------------
    // UI DATA
    // -------------------------------------------------------------------------

    // Button list: { position, size, mesh (assigned in Init), nextGS }
    std::vector<GfxButton> controlsButtons{
        {{0, -350}, {300, 100}, nullptr, GS_MAIN_MENU}  // Back to main menu
    };

    // Text labels rendered each frame
    std::vector<GfxText> controlsTexts{
        {"Back", 1.f, 0, 0, 0, 255, {0, -350}},
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
            // Anon scope — matrix locals (s, t, f) are discarded after the draw call
            AEMtx33 s, t, f;
            AEMtx33Scale(&s, btn.size.x, btn.size.y);
            AEMtx33Trans(&t, btn.pos.x, btn.pos.y);
            AEMtx33Concat(&f, &t, &s);
            AEGfxSetTransform(f.m);
            AEGfxMeshDraw(pBtnMesh, AE_GFX_MDM_TRIANGLES);
        }
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);  // restore colour mode
    }

    // -------------------------------------------------------------------------
    // INPUT HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: On left-click, check each controls button for a hit and set the next game state.
    void clickToNextState() {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;  // skip if left click not triggered

        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        for (GfxButton& btn : controlsButtons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;
            SFX::playSFX(SFX_UI_BUTTON_SELECT);
            GS_next = btn.nextGS;
            break;
        }
    }

}  // namespace

// =============================================================================
// LOAD / UNLOAD
// =============================================================================

// ~ Brief: Load all textures and build meshes needed for the controls screen.
void LoadControls() {
    pBgTex = AEGfxTextureLoad("./Assets/sandbg.png");
    pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
    pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");

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
}

// ~ Brief: Free all meshes and textures owned by the controls screen to prevent memory leaks.
void UnloadControls() {
    if (rectMesh) { AEGfxMeshFree(rectMesh);           rectMesh = nullptr; }
    if (pBgMesh) { AEGfxMeshFree(pBgMesh);            pBgMesh = nullptr; }
    if (pBtnMesh) { AEGfxMeshFree(pBtnMesh);           pBtnMesh = nullptr; }
    if (pBgTex) { AEGfxTextureUnload(pBgTex);        pBgTex = nullptr; }
    if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex); pBtnNormalTex = nullptr; }
    if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);  pBtnHoverTex = nullptr; }
}

// =============================================================================
// INIT / FREE
// =============================================================================

// ~ Brief: Assign the shared rect mesh to all controls buttons.
void InitializeControls() {
    for (GfxButton& btn : controlsButtons)
        btn.mesh = rectMesh;
}

// ~ Brief: Nothing to free between game-state transitions for this screen.
void FreeControls() {}

// =============================================================================
// UPDATE
// =============================================================================

// ~ Brief: Poll for button clicks and handle game-state transitions.
void UpdateControls() {
    clickToNextState();
}

// =============================================================================
// DRAW
// =============================================================================

// ~ Brief: Render the controls screen in layer order:
//          background, key-binding text block, back button and its label.
void DrawControls() {
    // Draw background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBgTex, 0, 0);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.f);
    {
        // Anon scope — matrix locals (s, t, f) are discarded after the draw call
        AEMtx33 s, t, f;
        AEMtx33Scale(&s, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
        AEMtx33Trans(&t, 0.f, 0.f);
        AEMtx33Concat(&f, &t, &s);
        AEGfxSetTransform(f.m);
        AEGfxMeshDraw(pBgMesh, AE_GFX_MDM_TRIANGLES);
    }
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);  // restore colour mode

    // Build and draw the key-binding text block
    GfxText text{ "", 1.f };
    text.pos = { 0, 300 };
    text.text += "CONTROLS\n\n\n";
    text.text += "WASD            -    Movement\n\n\n";
    text.text += "Left Click / Space  -    Shoot\n\n\n";
    text.text += "F11             -    Fullscreen\n\n\n";
    Gfx::printMultiline(text, boldPixels);

    // Get mouse position for hover detection
    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);

    // Draw back button and its label
    for (GfxButton& btn : controlsButtons) drawTexturedButton(btn, mousepos);
    for (GfxText& txt : controlsTexts)   Gfx::printText(txt, boldPixels);
}