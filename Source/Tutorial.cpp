// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Tutorial.cpp
// Authors: [Men of Pause II]
// Brief:   Tutorial overlay — displays context-sensitive prompt text during
//          Wave 1 (movement and shooting hints) and Wave 2 (card shop guide).
//          Steps advance on player input and persist across frames via a
//          shared index.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"
#include "Tutorial.h"

// =============================================================================
// ANONYMOUS NAMESPACE — file-private data
// =============================================================================

namespace {

    int tutorialIndex{ 0 };  // current tutorial step; persists between frames

}  // namespace

// =============================================================================
// TUTORIAL FUNCTIONS
// =============================================================================

// ~ Brief: Reset the tutorial step counter to the beginning.
//          Call this whenever a new game session starts.
void resetTutorial() {
    tutorialIndex = 0;
}

// ~ Brief: Draw the tutorial overlay for the current wave and tutorial step.
//          Wave 1 steps (0–2) cover movement and shooting controls.
//          Wave 2 steps (2–6) walk the player through the card shop.
//          Each step advances on Space or Left Click, except the final hint
//          of each wave which waits for the player to act in-world.
void printtutorial() {
    // Fetch camera position so UI text can be offset to stay screen-locked
    f32 camx, camy;
    AEGfxGetCamPosition(&camx, &camy);

    // -------------------------------------------------------------------------
    // WAVE 1 — Movement and combat hints
    // -------------------------------------------------------------------------

    if (currentWave == 1) {
        GfxText text{ "", 1.f, 0, 0, 0, 255, {0, 0} };

        switch (tutorialIndex) {
        case 0:  // Movement hint — advances on any WASD key press
            text.text = "Press WASD to move";
            text.scale = 0.7f;
            text.pos = { 0 - camx, 200 - camy };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_W) || AEInputCheckTriggered(AEVK_A) ||
                AEInputCheckTriggered(AEVK_S) || AEInputCheckTriggered(AEVK_D))
                ++tutorialIndex;
            break;

        case 1:  // Shooting hint — advances on Space or Left Click
            text.text = "Hold Spacebar or LButton to Shoot";
            text.scale = 0.7f;
            text.pos = { 0 - camx, 200 - camy };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;

        case 2:  // Enemy indicator hint — persists until wave ends; no advance trigger
            text.text = "The arrows indicate enemies location! \n Shoot all the boxes & enemies!";
            text.scale = 0.7f;
            text.pos = { 0 - camx, 200 - camy };
            Gfx::printMultiline(text, boldPixels);
            break;
        }
    }

    // -------------------------------------------------------------------------
    // WAVE 2 — Card shop guide (steps 2–6 share the index with Wave 1)
    // -------------------------------------------------------------------------

    else if (currentWave == 2) {
        GfxText text{ "", 1.f, 255, 255, 255, 255, {0, 0} };

        switch (tutorialIndex) {
        case 2:  // Shop introduction — advances on Space or Left Click
            text.text = "Welcome to\nthe Card Shop!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;

        case 3:  // Buy limit reminder — advances on Space or Left Click
            text.text = "You get 1 buy\nper shop phase.\n\nSpend it wisely!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;

        case 4:  // Drag-to-equip hint — advances on Space or Left Click
            text.text = "DRAG a card\nfrom the shop\nto equip it!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;

        case 5:  // Card buff explanation — advances on Space or Left Click
            text.text = "CARDS\nbuff your stats\nevery wave!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;

        case 6:  // Exit shop prompt — advances on Space or Left Click
            text.text = "When you are\ndone, click\nthe SHOP to\ncontinue!\n\nGood Luck!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;
        }
    }
}