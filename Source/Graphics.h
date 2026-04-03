// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Graphics.h
// Authors: [Men of Pause II]
// Brief:   Graphics utility library — mesh factories (rect, circle, triangle),
//          world-space and camera-locked mesh drawing, and text rendering
//          helpers (single-line, multiline, button wrappers).
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h"

// =============================================================================
// GRAPHICS STRUCTS
// =============================================================================

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
	bool hovered{ false }; // for SFX triggers
};

// =============================================================================
// GRAPHICS UTILITY LIBRARY
// =============================================================================

namespace Graphics {

	// =========================================================================
	// MESH FACTORIES
	// =========================================================================

	// ~ Brief: Build and return a UV-mapped rectangle mesh with a configurable
	//          colour and UV region. Defaults produce a full white unit quad.
	AEGfxVertexList* createRectMesh(u32 colour = 0x00000000, f32 uMin = 0.0f, f32 vMin = 0.0f, f32 uMax = 1.0f, f32 vMax = 1.0f);
	
	// ~ Brief: Build and return a circle mesh approximated by a fan of 40 triangles
	//          radiating from the origin.
	AEGfxVertexList* createCircleMesh(u32 colour = 0x00000000);

	// ~ Brief: Build and return a right-pointing triangle mesh.
	AEGfxVertexList* createTriangleMesh(u32 colour = 0x00000000);
	
	// =========================================================================
	// WORLD-SPACE MESH DRAWING
	// =========================================================================

	// ~ Brief: Draw a mesh in world space using a full RTS transform.
	//          An optional local offset is applied before rotation and translation,
	//          allowing pivot-point control without moving the mesh origin.
	//          Does nothing if mesh is null.
	void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size, f32 angleRad = 0.f, AEVec2 offset = { 0,0 });
	
	// ~ Brief: Card-specific overload — draws a mesh scaled by an additional scalar
	//          applied on top of the card's own size. Assumes texture mode is already set.
	void printMesh(AEGfxVertexList* mesh, Card& card, f32 scalar = 1);
	
	// ~ Brief: Overload of printMesh that optionally skips render-mode setup,
	//          allowing the caller to pre-configure texture mode before the draw call.
	//          Does nothing if mesh is null.
	void printMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size, f32 angleRad, AEVec2 offset, bool useTexture);
	
	// ~ Brief: Card-specific overload — draws a mesh scaled by an additional scalar
	//          applied on top of the card's own size. Assumes texture mode is already set.
	void printText(GfxText& text, s8 const& font);

	// =========================================================================
	// UI MESH DRAWING (CAMERA-LOCKED)
	// =========================================================================

	// ~ Brief: Convenience wrapper that draws a GfxButton's mesh in world space
	//          at the button's own position and size.
	void printButton(GfxButton& button);

	// ~ Brief: Draw a multi-line GfxText by splitting on newline characters and
	//          rendering each line with a vertical offset proportional to the font scale.
	void printMultiline(GfxText& text, s8 const& font);

	// ~ Brief: Draw a mesh glued to screen space by offsetting its position by
	//          the current camera world position each frame.
	//          This guarantees the element moves perfectly with the screen glass.
	//          Does nothing if mesh is null.
	void printUIMesh(AEGfxVertexList* mesh, AEVec2 pos, AEVec2 size);

	// ~ Brief: Convenience wrapper that draws a GfxButton's mesh camera-locked
	//          at the button's own position and size.
	void printUIButton(GfxButton& button);
} // namespace Graphics
namespace Gfx = Graphics;
