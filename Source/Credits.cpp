// ---------------------------- - Gloomy's Escape----------------------------- //
// File:	credits.cpp
// Authors:	[Men of Pause II]
// Brief:	This file contains and displays the credits
// 
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"
#include "credits.h"


std::vector<GfxText> CreditTexts{
		{"Men of Pause Production",    1.f, 0, 0, 0, 255, {0, -200}},
		{"Digipen Executives\nPresident Claude Comair\nPrasanna Ghali", 1.f, 0, 0, 0, 255, {0, 0}},
		{"Instructors\nGerald\nTommy\nDr. Sooroor\n",    1.f, 0, 0, 0, 255, {0, 200}},
		{"Our Team\nFaris\nIzzat the goat\nShao Wei\nZi Hao\n",    1.f, 0, 0, 0, 255, {0, 400}},
		{"Assets\nGraphics\nPiskel\nFontMeme\nKenney\n",    1.f, 0, 0, 0, 255, {0, 600}},
		{"Audio\nBosca Ceoil\nSoundly\n",    1.f, 0, 0, 0, 255, {0, 800}},
};

void LoadCredits() {
	struct AEVec2 init[] = {
		{ 0, -200 },  // Men of Pause
		{ 0, 0 }, // Executives
		{ 0, 200 },
		{ 0, 400 },
		{ 0, 600 }, // Instructors
		{ 0, 800 },

	};
	for (int i = 0; i < CreditTexts.size(); i++) {
		CreditTexts[i].pos = init[i];
	}

}
void InitializeCredits() {

}
void UpdateCredits() {
	while (CreditTexts[5].pos.y > -1000) {
		for (auto c : CreditTexts) c.pos.y -= 10;
	}
}
void DrawCredits() {
	for (auto c : CreditTexts)
	Gfx::printMultiline(c, boldPixels);

}
void FreeCredits() {

}
void UnloadCredits() {

}