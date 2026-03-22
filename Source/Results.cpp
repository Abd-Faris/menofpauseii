#include "MasterHeader.h"

// 1. DEFINE THE VARIABLE HERE!
bool gameWon = false;

namespace {
    AEGfxVertexList* resultsBtnMesh = nullptr;

    // --- VICTORY BUTTONS ---
    std::vector<GfxButton> winButtons{
        {{-200, -100}, {300, 100}, nullptr, GS_CARD_SHOP}, // ID/NextState = Card Shop
        {{200, -100}, {300, 100}, nullptr, GS_MAIN_MENU}   // ID/NextState = Main Menu
    };
    std::vector<GfxText> winButtonTexts{
        {"Continue", 1.f, 255, 255, 255, 255, {-200, -100}},
        {"Main Menu", 1.f, 255, 255, 255, 255, {200, -100}}
    };

    // --- DEFEAT BUTTONS ---
    std::vector<GfxButton> loseButtons{
        {{0, -100}, {300, 100}, nullptr, GS_MAIN_MENU}     // ID/NextState = Main Menu
    };
    std::vector<GfxText> loseButtonTexts{
        {"Main Menu", 1.f, 255, 255, 255, 255, {0, -100}}
    };
}

void LoadResults() {
    // Create the rectangular mesh for our buttons
    resultsBtnMesh = Gfx::createRectMesh();

    // Assign the mesh to all buttons
    for (GfxButton& button : winButtons) {
        button.mesh = resultsBtnMesh;
    }
    for (GfxButton& button : loseButtons) {
        button.mesh = resultsBtnMesh;
    }
}

void UpdateResults() {
    // Skip if left click is not triggered
    if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

    // Get cursor position
    AEVec2 mousepos{};
    Comp::getCursorPos(mousepos);

    // Pick which list of buttons to check based on if we won or lost
    std::vector<GfxButton>& activeButtons = gameWon ? winButtons : loseButtons;

    // For each active button, check collision
    for (GfxButton& btn : activeButtons) {
        // If not colliding, move to the next button
        if (!Comp::collisionPointRect(mousepos, btn.pos, btn.size)) continue;

        // If clicked Main Menu, NOW we reset the game data!
        if (btn.nextGS == GS_MAIN_MENU) {
            reset_game();
            Cards::resetCards();
            resetTutorial();
        }

        // Change the state
        GS_next = btn.nextGS;
        break;
    }
}

void DrawResults() {
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetTransparency(1.0f);
    AEGfxSetCamPosition(0.0f, 0.0f); // Make sure camera is centered for UI

    // Pick the active button lists
    std::vector<GfxButton>& activeButtons = gameWon ? winButtons : loseButtons;
    std::vector<GfxText>& activeTexts = gameWon ? winButtonTexts : loseButtonTexts;

    // 2. CHECK IF WE WON OR LOST FOR BACKGROUND AND TITLE TEXT
    if (gameWon) {
        // --- VICTORY SCREEN ---
        AEGfxSetBackgroundColor(0.0f, 0.4f, 0.0f); // Green background
        GfxText winText = { "VICTORY!", 2.0f, 255, 255, 255, 255, {0.0f, 100.0f} };
        Gfx::printText(winText, boldPixels);
    }
    else {
        // --- DEFEAT SCREEN ---
        AEGfxSetBackgroundColor(0.4f, 0.0f, 0.0f); // Red background
        GfxText loseText = { "GAME OVER", 2.0f, 255, 255, 255, 255, {0.0f, 100.0f} };
        Gfx::printText(loseText, boldPixels);
    }

    // 3. DRAW THE BUTTONS AND TEXT
    for (GfxButton& button : activeButtons) {
        Gfx::printButton(button);
    }
    for (GfxText& text : activeTexts) {
        Gfx::printText(text, boldPixels);
    }
}

void FreeResults() {
    // Free the button mesh memory!
    if (resultsBtnMesh) {
        AEGfxMeshFree(resultsBtnMesh);
        resultsBtnMesh = nullptr;
    }
}