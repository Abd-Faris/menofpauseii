#pragma once
#include "MasterHeader.h"

struct gfxtext {
	const char* text;
	f32 x, y, scale, r, g, b, a;
	AEVec2 pos;
};

struct gfxbutton {
	AEVec2 pos;
	AEVec2 size;
	gfxtext text{};			 // optional
	AEGfxVertexList* mesh{}; // only assigned in initialize
};

namespace Graphics {
	AEGfxVertexList* createRectMesh(std::string alignment = "center", u32 colour = 0x00000000);
	void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size);
	void printMesh(Card &card);
	void printText(gfxtext &text, s8 font);
	void printButton(gfxbutton &button);
}
namespace Gfx = Graphics;
