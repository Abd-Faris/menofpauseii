#include "MasterHeader.h"

namespace {
    AEGfxVertexList* pBgMesh = nullptr;
    AEGfxTexture* pBgTex = nullptr;
    AEGfxVertexList* pBtnMesh = nullptr;
    AEGfxTexture* pBtnNormalTex = nullptr;
    AEGfxTexture* pBtnHoverTex = nullptr;
    AEGfxVertexList* rectMesh = nullptr;

    std::vector<GfxButton> controlsButtons{
        {{0, -350}, {300, 100}, nullptr, GS_MAIN_MENU} // Back
    };
    std::vector<GfxText> controlsTexts{
        {"Back", 1.f, 0, 0, 0, 255, {0, -350}},
    };

    void drawTexturedButton(GfxButton& btn, AEVec2& mousepos) {
        bool hovered = Comp::collisionPointRect(mousepos, btn.pos, btn.size);
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(hovered ? pBtnHoverTex : pBtnNormalTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        AEMtx33 s, t, f;
        AEMtx33Scale(&s, btn.size.x, btn.size.y);
        AEMtx33Trans(&t, btn.pos.x, btn.pos.y);
        AEMtx33Concat(&f, &t, &s);
        AEGfxSetTransform(f.m);
        AEGfxMeshDraw(pBtnMesh, AE_GFX_MDM_TRIANGLES);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    }

    void clickToNextState() {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;
        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);
        for (GfxButton& btn : controlsButtons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;
            GS_next = btn.nextGS;
            break;
        }
    }
}

void LoadControls() {
    pBgTex = AEGfxTextureLoad("./Assets/sandbg.png");
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

    rectMesh = Gfx::createRectMesh();
}

void InitializeControls() {
    for (GfxButton& btn : controlsButtons)
        btn.mesh = rectMesh;
}

void UpdateControls() {
    clickToNextState();
}

void DrawControls() {
    // background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBgTex, 0, 0);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.f);
    AEMtx33 s, t, f;
    AEMtx33Scale(&s, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
    AEMtx33Trans(&t, 0.f, 0.f);
    AEMtx33Concat(&f, &t, &s);
    AEGfxSetTransform(f.m);
    AEGfxMeshDraw(pBgMesh, AE_GFX_MDM_TRIANGLES);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // controls text
    GfxText text{ "", 1.f };
    text.pos = { 0, 300 };
    text.text += "CONTROLS\n\n\n";
    text.text += "WASD            -    Movement\n\n\n";
    text.text += "Left Click / Space  -    Shoot\n\n\n";
    text.text += "F11             -    Fullscreen\n\n\n";
    Gfx::printMultiline(text, boldPixels);

    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);

    for (GfxButton& btn : controlsButtons) drawTexturedButton(btn, mousepos);
    for (GfxText& txt : controlsTexts)   Gfx::printText(txt, boldPixels);
}

void FreeControls() {}

void UnloadControls() {
    if (rectMesh) { AEGfxMeshFree(rectMesh);           rectMesh = nullptr; }
    if (pBgMesh) { AEGfxMeshFree(pBgMesh);            pBgMesh = nullptr; }
    if (pBtnMesh) { AEGfxMeshFree(pBtnMesh);           pBtnMesh = nullptr; }
    if (pBgTex) { AEGfxTextureUnload(pBgTex);        pBgTex = nullptr; }
    if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex); pBtnNormalTex = nullptr; }
    if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);  pBtnHoverTex = nullptr; }
}