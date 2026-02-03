#pragma once
#include "MasterHeader.h"

// OLD STRUCT USE NEW ONE BELOW
struct gfxtext {
	const char* text;
	f32 x, y, scale, r, g, b, a;
	AEVec2 pos;
};

// NEW ONE, USE THIS INSTEAD
struct Gfxtext {
	AEVec2 pos;
	f32 scale;
	const char* text;
	f32 r{ 0.f }, g{ 0.f }, b{ 0.f }, a{ 255.f };

};

struct gfxbutton {
	AEVec2 pos;
	AEVec2 size;
	gfxtext text{};			 // optional
	AEGfxVertexList* mesh{}; // only assigned in initialize
};

namespace Graphics {
	AEGfxVertexList* createRectMesh(std::string alignment = "center", u32 colour = 0x00000000);
	void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size, f32 scalar=1);
	void printMesh(AEGfxVertexList* mesh, Card &card, f32 scalar = 1);
	void printText(Gfxtext& text, s8 font);
	void printText(gfxtext &text, s8 font);
	void printButton(gfxbutton &button);
}
namespace Gfx = Graphics;
