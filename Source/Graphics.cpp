#include "MasterHeader.h"

namespace {
	AEGfxVertexList* rectMeshCenter(u32 colour) {
		AEGfxMeshStart();
		AEGfxTriAdd(
			-0.5f, -0.5f, colour, 0.0f, 0.0f,
			0.5f, -0.5f, colour, 0.0f, 0.0f,
			-0.5f, 0.5f, colour, 0.0f, 0.0f);
		AEGfxTriAdd(
			0.5f, -0.5f, colour, 0.0f, 0.0f,
			0.5f, 0.5f, colour, 0.0f, 0.0f,
			-0.5f, 0.5f, colour, 0.0f, 0.0f);
		return AEGfxMeshEnd();
	}
	AEGfxVertexList* rectMeshLeft(u32 colour) {
		// quadrilateral mesh, left align
		AEGfxMeshStart();
		AEGfxTriAdd(
			0.f, -0.5f, colour, 0.0f, 1.0f,
			1.f, -0.5f, colour, 1.0f, 1.0f,
			0.f, 0.5f, colour, 0.0f, 0.0f);
		AEGfxTriAdd(
			1.f, -0.5f, colour, 1.0f, 1.0f,
			0.f, 0.5f, colour, 0.0f, 0.0f,
			1.f, 0.5f, colour, 1.0f, 0.0f);
		return AEGfxMeshEnd();
	}
	AEGfxVertexList* rectMeshRight(u32 colour) {
		// quadrilateral mesh, right align
		AEGfxMeshStart();
		AEGfxTriAdd(
			0.f, -0.5f, colour, 0.0f, 1.0f,
			-1.f, -0.5f, colour, 1.0f, 1.0f,
			0.f, 0.5f, colour, 0.0f, 0.0f);
		AEGfxTriAdd(
			-1.f, -0.5f, colour, 1.0f, 1.0f,
			0.f, 0.5f, colour, 0.0f, 0.0f,
			-1.f, 0.5f, colour, 1.0f, 0.0f);
		return AEGfxMeshEnd();
	}
}

namespace Graphics {

	AEGfxVertexList* createRectMesh(std::string alignment, u32 colour) {
		if (alignment == "center") return ::rectMeshCenter(colour);
		if (alignment == "left")   return ::rectMeshLeft(colour);
		if (alignment == "right")  return ::rectMeshRight(colour);
		return nullptr;
	}

	AEGfxVertexList* createCircleMesh(u32 colour) {
		constexpr int edges{ 40 };
		AEGfxMeshStart(); // let the pain begin

		// for loop to create meshes at diff rotations to form a circle
		for (int i{ 0 }; i < edges; ++i) {
			float angle1 = (i / float(edges)) * TWO_PI;
			float angle2 = ((i + 1) / float(edges)) * TWO_PI;

			// define triangle mesh with centre pts (0,0)
			AEGfxTriAdd(
				0.f, 0.f, 0xFFFFFFFF, 0.5f, 0.5f, // Centre point
				AECos(angle1) * 0.5f, AESin(angle1) * 0.5f, colour, 0.0f, 0.0f,
				AECos(angle2) * 0.5f, AESin(angle2) * 0.5f, colour, 0.0f, 0.0f);
		}
		// return mesh and marvel at a round object
		// such teknologi much wow
		return AEGfxMeshEnd();
	}

	// Prints a mesh using TRS
	void printMesh(AEGfxVertexList *mesh, f32 posx, f32 posy, f32 sizex, f32 sizey) {
		// if no mesh, return
		if (!mesh) return;
		// object drawing settings
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
		AEGfxSetTransparency(1.f);
		// Translate Matrix
		AEMtx33 translate{ 0 };
		AEMtx33Trans(&translate, posx, posy);
		// Scale Matrix
		AEMtx33 scale{ 0 };
		AEMtx33Scale(&scale, sizex, sizey);
		// Resultant Transformation
		AEMtx33 transform{ 0 };
		AEMtx33Concat(&transform, &translate, &scale);	
		// Apply transformation to mesh
		AEGfxSetTransform(transform.m);
		// Printing
		AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
	}

	// overload for card printing
	void printMesh(Card& card) {
		printMesh(card.mesh, card.xpos, card.ypos, card.sizex, card.sizey);
	}

	// prints text
	void printText(gfxtext &text, s8 font) {
		// Normalise Coordinates
		f32 x{ text.x }, y{ text.y };
		Comp::normalizePoint(x, y);
		// Normalise RGBA
		f32 r{ text.r }, g{ text.g }, b{ text.b }, a{ text.a };
		Comp::normalizeRGBA(r, g, b, a);
		// Calculates offset to "center align" text
		f32 width, height;
		AEGfxGetPrintSize(font, text.text, text.scale, &width, &height);
		// Subtract normalised (width/height) / 2 from normalised pos
		f32 drawX = x - (width / 2.f);
		f32 drawY = y - (height / 2.f);
		// Prints text
		AEGfxPrint(font, text.text, drawX, drawY, text.scale, r, g, b, a);
	}

	void printButton(gfxbutton &button) {
		gfxtext& text = button.text;
		printMesh(button.mesh, text.x, text.y, button.sizex, button.sizey);
	}
}

