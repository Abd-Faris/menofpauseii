#pragma once
#include "MasterHeader.h"

namespace Graphics {
	AEGfxVertexList* createRectMesh(std::string alignment = "center", u32 colour = 0x00000000);
	void printMesh(Card &card);
	void printText(gfxtext &text, s8 font);
}
namespace Gfx = Graphics;
