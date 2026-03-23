#include "MasterHeader.h"
#include "Tutorial.h"

namespace {
	int tutorialIndex{ 0 }; // persists between frames
}

void resetTutorial() {
	tutorialIndex = 0;
}

void printtutorial() {
	// get cam position
	f32 camx, camy;
	AEGfxGetCamPosition(&camx, &camy);
	
	// tutorial index control variable
	if (currentWave == 1) {
		GfxText text{ "", 1.f, 0, 0, 0, 255, {0, 0} };
		switch (tutorialIndex) {
		case 0: // print WASD controls
			text.text = "Press WASD to move";
			text.scale = 0.7f;
			text.pos = { 0 - camx, 200 - camy };
			Gfx::printMultiline(text, boldPixels);
			if (AEInputCheckTriggered(AEVK_W) || AEInputCheckTriggered(AEVK_A) || 
				AEInputCheckTriggered(AEVK_S) || AEInputCheckTriggered(AEVK_D)) 
				++tutorialIndex;
			break;
		case 1:
			text.text = "Hold Spacebar or LButton to Shoot";
			text.scale = 0.7f;
			text.pos = { 0 - camx, 200 - camy };
			Gfx::printMultiline(text, boldPixels);
			if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
				++tutorialIndex;
			break;
		case 2:
			text.text = "Shoot all the Boxes and Enemies!";
			text.scale = 0.7f;
            text.pos = { 0 - camx, 200 - camy };
			Gfx::printMultiline(text, boldPixels);
			break;
		}
	} // end wave1 tutorial text
    else if (currentWave == 2) {
        GfxText text{ "", 1.f, 255, 255, 255, 255, {0, 0} };
        switch (tutorialIndex) {
        case 2:
            text.text = "Welcome to\nthe Card Shop!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;
        case 3:
            text.text = "You get 1 buy\nper shop phase.\n\nSpend it wisely!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;
        case 4:
            text.text = "DRAG a card\nfrom the shop into\nACTIVE CARDS\nto equip it!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;
        case 5:
            text.text = "ACTIVE CARDS\nbuff your stats\nevery wave!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;
        case 6:
            text.text = "DRAG a card\ninto the BAG\nto store it\nfor passive\nupgrades!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;
        case 7:
            text.text = "DRAG a card\nto the TRASH\nto delete it.\n\nYou cannot\nget it back!\n\nLEFT CLICK\nto continue!";
            text.scale = 0.5f;
            text.pos = { 575, 250 };
            Gfx::printMultiline(text, boldPixels);
            if (AEInputCheckTriggered(AEVK_SPACE) || AEInputCheckTriggered(AEVK_LBUTTON))
                ++tutorialIndex;
            break;
        case 8:
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