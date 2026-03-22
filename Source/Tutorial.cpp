#include "Tutorial.h"

void printtutorial() {

	std::vector<GfxText> TutorialTexts{
	{"WASD to move!",    1.f, 0, 0, 0, 255, {0, -200}},
	{"Space or Mouse1 to Shoot!", 1.f, 0, 0, 0, 255, {0,    0}},
	{"Kill all enemies to proceed to next wave!",    1.f, 0, 0, 0, 255, {0,  200}},
	};
	for (GfxText s : TutorialTexts)
	Gfx::printMultiline(s, boldPixels);

}