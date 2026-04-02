#include "MasterHeader.h"

bool tutorialOn{ false };
bool cheatsOn{ false };

namespace {
    AEGfxVertexList* rectMesh = nullptr;
    AEGfxVertexList* pBgMesh = nullptr;
    AEGfxVertexList* pBtnMesh = nullptr;
    AEGfxTexture* pBgTex = nullptr;
    AEGfxTexture* pBtnNormalTex = nullptr;
    AEGfxTexture* pBtnHoverTex = nullptr;
    AEGfxTexture* pBgExitTex = nullptr;

    bool hasSave = false;

    // Main buttons — Continue only shown if save exists
    std::vector<GfxButton> mainMenuButtons{
        {{0,   -50}, {300, 100}, nullptr, -3},  // ID -3 = New Game
        {{0,    50}, {300, 100}, nullptr, -4},  // ID -4 = Continue (greyed out if no save)
        {{-200,-200}, {300, 100}, nullptr, -1}, // ID -1 = Toggle Tutorial
        {{200, -200}, {300, 100}, nullptr, -2}, // ID -2 = Toggle Cheats
        {{0,  -350}, {300, 100}, nullptr, GS_MAIN_MENU} // Back
    };

    std::vector<GfxText> mainMenuTexts{
        {"New Game",      0.67f,  0, 0, 0, 255, {0,    -50}},
        {"Continue",      0.67f,  0, 0, 0, 255, {0,     50}},
        {"Tutorial: OFF", 0.5f, 0, 0, 0, 255, {-200, -200}},
        {"Cheats: OFF",   0.5f, 0, 0, 0, 255, {200,  -200}},
        {"Back",          1.f,  0, 0, 0, 255, {0,   -350}},
    };

    void drawTexturedButton(GfxButton& btn, AEVec2& mousepos, bool greyed = false) {
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

    void printMainMenuUI(AEVec2& mousepos) {
        for (int i = 0; i < (int)mainMenuButtons.size(); i++) {
            // Skip Continue button entirely if no save
            if (mainMenuButtons[i].nextGS == -4 && !hasSave) continue;

            drawTexturedButton(mainMenuButtons[i], mousepos);
        }
        for (int i = 0; i < (int)mainMenuTexts.size(); i++) {
            // Skip Continue text entirely if no save
            if (i == 1 && !hasSave) continue;

            Gfx::printText(mainMenuTexts[i], boldPixels);
        }
    }

    void handleButton(int id) {
        switch (id) {
        case -1: // Toggle Tutorial
            tutorialOn = !tutorialOn;
            mainMenuTexts[2].text = tutorialOn ? "Tutorial: ON" : "Tutorial: OFF";
            break;

        case -2: // Toggle Cheats
            cheatsOn = !cheatsOn;
            mainMenuTexts[3].text = cheatsOn ? "Cheats: ON" : "Cheats: OFF";
            break;

        case -3: // New Game — delete save and start fresh
            DeleteSave();
            hasSave = false;
            GS_next = GS_GAME;
            break;

        case -4: // Continue — only if save exists
            if (hasSave)
                GS_next = GS_GAME;
            break;
        }
    }

    void clickToNextState() {
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        for (GfxButton& btn : mainMenuButtons) {
            if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

            if (btn.nextGS < 0) {
                handleButton(btn.nextGS);
                break;
            }
            GS_next = btn.nextGS;
            break;
        }
    }
}

void LoadLevelSelect() {
    pBgTex = AEGfxTextureLoad("./Assets/menu.png");
    pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
    pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");
    pBgExitTex = AEGfxTextureLoad("./Assets/gameexit.png");

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

    Cards::resetCards();
}

void InitializeLevelSelect() {
    // Check for save file every time this screen is entered
    hasSave = SaveExists();

    mainMenuTexts[2].text = tutorialOn ? "Tutorial: ON" : "Tutorial: OFF";
    mainMenuTexts[3].text = cheatsOn ? "Cheats: ON" : "Cheats: OFF";

    for (GfxButton& button : mainMenuButtons)
        button.mesh = rectMesh;
}

void UpdateLevelSelect() {
    clickToNextState();
}

void DrawLevelSelect() {
    // Background
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBgTex, 0, 0);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.f);
    AEMtx33 scale, trans, final;
    AEMtx33Scale(&scale, (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight());
    AEMtx33Trans(&trans, 0.f, 0.f);
    AEMtx33Concat(&final, &trans, &scale);
    AEGfxSetTransform(final.m);
    AEGfxMeshDraw(pBgMesh, AE_GFX_MDM_TRIANGLES);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);
    printMainMenuUI(mousepos);
}

void FreeLevelSelect() {}

void UnloadLevelSelect() {
    if (rectMesh) { AEGfxMeshFree(rectMesh);              rectMesh = nullptr; }
    if (pBgMesh) { AEGfxMeshFree(pBgMesh);               pBgMesh = nullptr; }
    if (pBtnMesh) { AEGfxMeshFree(pBtnMesh);              pBtnMesh = nullptr; }
    if (pBgTex) { AEGfxTextureUnload(pBgTex);           pBgTex = nullptr; }
    if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex);    pBtnNormalTex = nullptr; }
    if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);     pBtnHoverTex = nullptr; }
    if (pBgExitTex) { AEGfxTextureUnload(pBgExitTex);       pBgExitTex = nullptr; }
}