#include "Tutorial.h"

void printtutorial() {

	f32 camx, camy;

	AEGfxGetCamPosition(&camx, &camy);

	std::vector<GfxText> TutorialTexts{
	{"WASD to move!\n Space or Mouse1 to Shoot! \n Kill all enemies to proceed to next wave!",    0.5f, 0, 0, 0, 255, {0 - camx, 200 - camy}},
	};

	for (GfxText s : TutorialTexts)
	Gfx::printMultiline(s, boldPixels);



	if (AEInputCheckCurr(AEVK_SPACE)) {
		tutorialOn = FALSE;
	}

}