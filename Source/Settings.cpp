// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Settings.cpp
// Authors: [Men of Pause II]
// Brief:   Settings screen — volume sliders (SFX / BGM), fullscreen toggle,
//          and back-to-menu navigation.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// GLOBALS
// =============================================================================

float sfxVolume{ 0.5f };  // SFX volume (0.0 – 1.0)
float bgmVolume{ 1.0f };  // BGM volume (0.0 – 1.0)

// =============================================================================
// ANONYMOUS NAMESPACE — file-private data and helpers
// =============================================================================

namespace {

    // -------------------------------------------------------------------------
    // MESHES & TEXTURES
    // -------------------------------------------------------------------------

    AEGfxVertexList* rectMesh = nullptr;  // plain colour rect (slider track)
    AEGfxVertexList* pBgMesh = nullptr;  // full-screen background quad
    AEGfxVertexList* pBtnMesh = nullptr;  // button / slider thumb quad

    AEGfxTexture* pBgTex = nullptr;  // background texture
    AEGfxTexture* pBtnNormalTex = nullptr;  // button idle state
    AEGfxTexture* pBtnHoverTex = nullptr;  // button hovered state

    // -------------------------------------------------------------------------
    // CONSTANTS
    // -------------------------------------------------------------------------

    // ---- Slider track dimensions ----
    constexpr float TRACK_W = 400.f;
    constexpr float TRACK_H = 20.f;
    constexpr float THUMB_W = 30.f;
    constexpr float THUMB_H = 50.f;

    // ---- Slider row positions ----
    constexpr float SFX_Y = 150.f;   // Y centre of the SFX slider row
    constexpr float BGM_Y = 0.f;   // Y centre of the BGM slider row
    constexpr float TRACK_X = -200.f;   // left-edge anchor for both tracks

    // -------------------------------------------------------------------------
    // DRAG STATE
    // -------------------------------------------------------------------------

    bool draggingSFX{ false };  // true while the user holds and drags the SFX thumb
    bool draggingBGM{ false };  // true while the user holds and drags the BGM thumb

    // TODO: btnsfx is declared but never referenced — remove or wire up
    bool btnsfx{ false };

    // -------------------------------------------------------------------------
    // UI DATA
    // -------------------------------------------------------------------------

    // Button list: { position, size, mesh (assigned in Init), nextGS }
    // nextGS < 0 triggers a special action in handleButton() instead of a state transition
    std::vector<GfxButton> settingsButtons{
        {{0, -150}, {300, 100}, nullptr, -10},          // Fullscreen toggle
        {{0, -280}, {300, 100}, nullptr, GS_MAIN_MENU}  // Back to main menu
    };

    // Text labels rendered each frame; index [0] is updated dynamically to reflect fullscreen state
    std::vector<GfxText> settingsTexts{
        {"Fullscreen: OFF", 0.5f, 0, 0, 0, 255, {0,    -150}},
        {"Back",            1.0f, 0, 0, 0, 255, {0,    -280}},
        {"SFX",             1.0f, 0, 0, 0, 255, {-320,  150}},
        {"BGM",             1.0f, 0, 0, 0, 255, {-320,    0}},
        {"Volume Controls", 1.0f, 0, 0, 0, 255, {0,     300}},
    };

    // -------------------------------------------------------------------------
    // SLIDER HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Convert a normalised volume (0.0–1.0) to the thumb's world X position.
    float volumeToThumbX(float vol) {
        return TRACK_X + vol * TRACK_W;
    }

    // ~ Brief: Convert a cursor world X position to a normalised volume (clamped 0.0–1.0).
    float thumbXToVolume(float cursorX) {
        float t = (cursorX - TRACK_X) / TRACK_W;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        return t;
    }

    // ~ Brief: Draw a volume slider (track + draggable thumb) at the given vertical centre.
    //          Thumb texture switches to the hover state when the cursor is over it.
    void drawSlider(float centreY, float volume, AEVec2& mousepos) {
        // --- Track ---
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToAdd(0.4f, 0.4f, 0.4f, 1.f);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        {
            AEMtx33 s, t, f;
            AEMtx33Scale(&s, TRACK_W, TRACK_H);
            AEMtx33Trans(&t, TRACK_X + TRACK_W * 0.5f, centreY);
            AEMtx33Concat(&f, &t, &s);
            AEGfxSetTransform(f.m);
            AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
        }

        // --- Thumb ---
        float thumbX = volumeToThumbX(volume);
        bool hovered = (mousepos.x >= thumbX - THUMB_W * 0.5f &&
            mousepos.x <= thumbX + THUMB_W * 0.5f &&
            mousepos.y >= centreY - THUMB_H * 0.5f &&
            mousepos.y <= centreY + THUMB_H * 0.5f);

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(hovered ? pBtnHoverTex : pBtnNormalTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        {
            AEMtx33 s, t, f;
            AEMtx33Scale(&s, THUMB_W, THUMB_H);
            AEMtx33Trans(&t, thumbX, centreY);
            AEMtx33Concat(&f, &t, &s);
            AEGfxSetTransform(f.m);
            AEGfxMeshDraw(pBtnMesh, AE_GFX_MDM_TRIANGLES);
        }
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);  // restore colour mode
    }

    // -------------------------------------------------------------------------
    // BUTTON HELPERS
    // -------------------------------------------------------------------------

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

    // ~ Brief: Execute the special action for a button whose nextGS is negative.
    //          Currently handles fullscreen toggle (id == -10).
    void handleButton(int id) {
        if (id == -10) {
            fullscreen = !fullscreen;
            updateFullScreen();
        }
    }

    // -------------------------------------------------------------------------
    // INPUT HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Return true if the cursor is within the draggable region of a slider track.
    bool isOverTrack(AEVec2& mousepos, float centreY) {
        return (mousepos.x >= TRACK_X - THUMB_W * 0.5f &&
            mousepos.x <= TRACK_X + TRACK_W + THUMB_W * 0.5f &&
            mousepos.y >= centreY - THUMB_H * 0.5f &&
            mousepos.y <= centreY + THUMB_H * 0.5f);
    }

    // ~ Brief: Begin dragging a slider thumb on left-click, release on button-up,
    //          and update the corresponding volume while dragging.
    void handleSliderInput(AEVec2& mousepos) {
        // Start drag when the mouse is pressed over a track
        if (AEInputCheckTriggered(AEVK_LBUTTON)) {
            if (isOverTrack(mousepos, SFX_Y)) draggingSFX = true;
            if (isOverTrack(mousepos, BGM_Y)) draggingBGM = true;
        }

        // Release drag when the mouse button is no longer held
        if (!AEInputCheckCurr(AEVK_LBUTTON)) {
            draggingSFX = false;
            draggingBGM = false;
        }

        // Update volumes while dragging
        if (draggingSFX) sfxVolume = thumbXToVolume(mousepos.x);
        if (draggingBGM) bgmVolume = thumbXToVolume(mousepos.x);
    }

    // ~ Brief: On left-click, check each settings button for a hit.
    //          Negative nextGS values route to handleButton(); positive values trigger a game-state transition.
    void clickButtons(AEVec2& mousepos) {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

        for (GfxButton& btn : settingsButtons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

            SFX::playSFX(SFX_UI_BUTTON_SELECT);

            if (btn.nextGS < 0) { handleButton(btn.nextGS); break; }
            GS_next = btn.nextGS;
            break;
        }
    }

}  // namespace

// =============================================================================
// LOAD / UNLOAD
// =============================================================================

// ~ Brief: Load all textures and build meshes needed for the settings screen.
void LoadSettings() {
    pBgTex = AEGfxTextureLoad("./Assets/sandbg.png");
    pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
    pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");

    // Background quad
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBgMesh = AEGfxMeshEnd();

    // Button / thumb quad (identical UV layout to background)
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBtnMesh = AEGfxMeshEnd();

    // Plain colour rect used for the slider tracks
    rectMesh = Gfx::createRectMesh();
}

// ~ Brief: Free all meshes and textures owned by the settings screen to prevent memory leaks.
void UnloadSettings() {
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

// ~ Brief: Reset drag state, sync the fullscreen label to the current setting,
//          and assign the shared rect mesh to every button.
void InitializeSettings() {
    draggingSFX = false;
    draggingBGM = false;

    // Sync fullscreen label to current state
    settingsTexts[0].text = fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF";

    // Assign shared mesh to all buttons
    for (GfxButton& btn : settingsButtons)
        btn.mesh = rectMesh;
}

// ~ Brief: Nothing to free between game-state transitions for this screen.
void FreeSettings() {}

// =============================================================================
// UPDATE
// =============================================================================

// ~ Brief: Poll cursor position, handle slider drag input, and process button clicks.
void UpdateSettings() {
    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);

    handleSliderInput(mousepos);
    clickButtons(mousepos);
}

// =============================================================================
// DRAW
// =============================================================================

// ~ Brief: Render the settings screen in layer order:
//          background, sliders, buttons, text labels, volume percentages.
void DrawSettings() {
    // Draw background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBgTex, 0, 0);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.f);
    {
        AEMtx33 s, t, f;
        AEMtx33Scale(&s, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
        AEMtx33Trans(&t, 0.f, 0.f);
        AEMtx33Concat(&f, &t, &s);
        AEGfxSetTransform(f.m);
        AEGfxMeshDraw(pBgMesh, AE_GFX_MDM_TRIANGLES);
    }

    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);

    // Draw sliders
    drawSlider(SFX_Y, sfxVolume, mousepos);
    drawSlider(BGM_Y, bgmVolume, mousepos);

    // Draw buttons
    for (GfxButton& btn : settingsButtons) drawTexturedButton(btn, mousepos);

    // Draw text labels
    for (GfxText& txt : settingsTexts) Gfx::printText(txt, boldPixels);

    // Draw volume percentages next to each slider track
    char sfxBuf[16], bgmBuf[16];
    sprintf_s(sfxBuf, "%d%%", (int)(sfxVolume * 100.f));
    sprintf_s(bgmBuf, "%d%%", (int)(bgmVolume * 100.f));

    GfxText sfxPct{ sfxBuf, 0.5f, 0, 0, 0, 255, {TRACK_X + TRACK_W + 60.f, SFX_Y} };
    GfxText bgmPct{ bgmBuf, 0.5f, 0, 0, 0, 255, {TRACK_X + TRACK_W + 60.f, BGM_Y} };

    Gfx::printText(sfxPct, boldPixels);
    Gfx::printText(bgmPct, boldPixels);

    // Keep fullscreen label in sync with the current toggle state
    settingsTexts[0].text = fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF";
}