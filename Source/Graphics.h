#pragma once
#include "MasterHeader.h"

namespace Graphics {
	AEGfxVertexList* createRectMesh(u32 colour = 0xFFFFFF00, std::string alignment = "center");
	void printText(gfxtext &text, s8 font);
}
namespace Gfx = Graphics;
