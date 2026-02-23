#pragma once
#include "MasterHeader.h"

struct GfxText {
	std::string text;
	f32 scale{ 1 }; // default 1
	f32 r{ 0.f }, g{ 0.f }, b{ 0.f }, a{ 255.f }; // default black
	AEVec2 pos{ 0.f, 0.f }; // optional
};

struct GfxButton {
	AEVec2 pos, size;
	AEGfxVertexList* mesh;
	int nextGS{}; // if not set, defaults to main menu 
};

namespace Graphics {
	AEGfxVertexList* createRectMesh(std::string alignment = "center", u32 colour = 0x00000000);
	AEGfxVertexList* createCircleMesh(u32 colour = 0x00000000);
	AEGfxVertexList* createTriangleMesh(u32 colour = 0x00000000);
	//void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size, f32 scalar=1);
	void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size, f32 angleRad = 0.f, AEVec2 offset = { 0,0 });
	void printMesh(AEGfxVertexList* mesh, Card& card, f32 scalar = 1);
	void printText(GfxText& text, s8 font);
	void printButton(GfxButton& button);
}
namespace Gfx = Graphics;
