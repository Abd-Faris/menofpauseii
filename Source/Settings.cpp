#include "MasterHeader.h"

float sfxVolume{ 1.0f };
float bgmVolume{ 1.0f };

namespace {
    // mesh pointers
    AEGfxVertexList* rectMesh = nullptr;
    AEGfxVertexList* pBgMesh = nullptr;
    AEGfxVertexList* pBtnMesh = nullptr;
    AEGfxTexture* pBgTex = nullptr;

    AEGfxTexture* pBtnNormalTex = nullptr;
    AEGfxTexture* pBtnHoverTex = nullptr;

    // slider track dimensions (for volume control)
    constexpr float TRACK_W = 400.f;
    constexpr float TRACK_H = 20.f;
    constexpr float THUMB_W = 30.f;
    constexpr float THUMB_H = 50.f;

    constexpr float SFX_Y = 150.f;    
    constexpr float BGM_Y = 0.f;     
    constexpr float TRACK_X = -200.f; // left edge anchor

    // booleans
    bool draggingSFX{ false };
    bool draggingBGM{ false };

    float volumeToThumbX(float vol) {
        return TRACK_X + vol * TRACK_W;
    }

    float thumbXToVolume(float cursorX) {
        float t = (cursorX - TRACK_X) / TRACK_W;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        return t;
    }

    // slider drwaing
    void drawSlider(float centreY, float volume, AEVec2& mousepos) {
        // --- track ---
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

        // --- thumb ---
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
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    }

    // graphics on buttons
    void drawTexturedButton(GfxButton& btn, AEVec2& mousepos) {
        bool hovered = Comp::collisionPointRect(mousepos, btn.pos, btn.size);
        
        // AE Settings
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(hovered ? pBtnHoverTex : pBtnNormalTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        // anon scope since we wanna trash size, transform, and final later lol
        {
            AEMtx33 s, t, f;
            AEMtx33Scale(&s, btn.size.x, btn.size.y);
            AEMtx33Trans(&t, btn.pos.x, btn.pos.y);
            AEMtx33Concat(&f, &t, &s);
            AEGfxSetTransform(f.m);
            AEGfxMeshDraw(pBtnMesh, AE_GFX_MDM_TRIANGLES);
        }
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    }

    // texts
    std::vector<GfxButton> settingsButtons{
        {{0, -150}, {300, 100}, nullptr, -10},         // Fullscreen Button
        {{0, -280}, {300, 100}, nullptr, GS_MAIN_MENU} // Back Button
    };

    std::vector<GfxText> settingsTexts{
        {"Fullscreen: OFF", 0.5f, 0, 0, 0, 255, {0,   -150}},
        {"Back",            1.0f, 0, 0, 0, 255, {0,   -280}},
        {"SFX",             1.0f, 0, 0, 0, 255, {-320, 150}},
        {"BGM",             1.0f, 0, 0, 0, 255, {-320,  0}},
        {"Volume Controls", 1.0f, 0, 0, 0, 255, {0,  300}},
    };

    void handleButton(int id) {
        if (id == -10) {
            fullscreen = !fullscreen;
            updateFullScreen();
        }
    }

    bool isOverTrack(AEVec2& mousepos, float centreY) {
        return (mousepos.x >= TRACK_X - THUMB_W * 0.5f &&
            mousepos.x <= TRACK_X + TRACK_W + THUMB_W * 0.5f &&
            mousepos.y >= centreY - THUMB_H * 0.5f &&
            mousepos.y <= centreY + THUMB_H * 0.5f);
    }

    void handleSliderInput(AEVec2& mousepos) {
        if (AEInputCheckTriggered(AEVK_LBUTTON)) {
            if (isOverTrack(mousepos, SFX_Y)) draggingSFX = true;
            if (isOverTrack(mousepos, BGM_Y)) draggingBGM = true;
        }
        if (!AEInputCheckCurr(AEVK_LBUTTON)) {
            draggingSFX = false;
            draggingBGM = false;
        }
        if (draggingSFX) sfxVolume = thumbXToVolume(mousepos.x);
        if (draggingBGM) bgmVolume = thumbXToVolume(mousepos.x);
    }

    void clickButtons(AEVec2& mousepos) {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;
        for (GfxButton& btn : settingsButtons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;
            if (btn.nextGS < 0) { handleButton(btn.nextGS); break; }
            GS_next = btn.nextGS;
            break;
        }
    }
}


void LoadSettings() {
    pBgTex = AEGfxTextureLoad("./Assets/sandbg.png");
    pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
    pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBgMesh = AEGfxMeshEnd();

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBtnMesh = AEGfxMeshEnd();

    rectMesh = Gfx::createRectMesh();
}

void InitializeSettings() {
    draggingSFX = false;
    draggingBGM = false;
    settingsTexts[0].text = fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF";
    for (GfxButton& btn : settingsButtons)
        btn.mesh = rectMesh;
}

void UpdateSettings() {
    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);
    handleSliderInput(mousepos);
    clickButtons(mousepos);
}

void DrawSettings() {
    // 1. Draw Background
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

    // 2. Draw Sliders
    drawSlider(SFX_Y, sfxVolume, mousepos);
    drawSlider(BGM_Y, bgmVolume, mousepos);

    // 3. Draw Buttons
    for (GfxButton& btn : settingsButtons)
        drawTexturedButton(btn, mousepos);

    // 4. Draw Text
    for (GfxText& txt : settingsTexts)
        Gfx::printText(txt, boldPixels);

    // 5. Draw Percentages
    char sfxBuf[16], bgmBuf[16];
    sprintf_s(sfxBuf, "%d%%", (int)(sfxVolume * 100.f));
    sprintf_s(bgmBuf, "%d%%", (int)(bgmVolume * 100.f));

    GfxText sfxPct{ sfxBuf, 0.5f, 0, 0, 0, 255, {TRACK_X + TRACK_W + 60.f, SFX_Y} };
    GfxText bgmPct{ bgmBuf, 0.5f, 0, 0, 0, 255, {TRACK_X + TRACK_W + 60.f, BGM_Y} };

    Gfx::printText(sfxPct, boldPixels);
    Gfx::printText(bgmPct, boldPixels);

    settingsTexts[0].text = fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF";
}

void FreeSettings() {}

void UnloadSettings() {
    if (rectMesh) { AEGfxMeshFree(rectMesh); rectMesh = nullptr; }
    if (pBgMesh) { AEGfxMeshFree(pBgMesh); pBgMesh = nullptr; }
    if (pBtnMesh) { AEGfxMeshFree(pBtnMesh); pBtnMesh = nullptr; }
    if (pBgTex) { AEGfxTextureUnload(pBgTex); pBgTex = nullptr; }
    if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex); pBtnNormalTex = nullptr; }
    if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex); pBtnHoverTex = nullptr; }
}