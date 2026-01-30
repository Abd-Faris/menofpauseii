#pragma once
#include "MasterHeader.h"

namespace Graphics {
	AEGfxVertexList* createRectMesh(std::string alignment = "center", u32 colour = 0x00000000);
	void printMesh(AEGfxVertexList* mesh, f32 posx, f32 posy, f32 sizex, f32 sizey);
	void printMesh(Card &card);
	void printText(gfxtext &text, s8 font);
	void printButton(gfxbutton &button);
}
namespace Gfx = Graphics;
