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

	void printText(gfxtext &text, s8 font) {
		// Normalise Coordinates
		f32 x = text.x / 800.f;
		f32 y = text.y / 800.f;
		// Normalise RGBA
		f32 r = text.r / 255.f;
		f32 g = text.g / 255.f;
		f32 b = text.b / 255.f;
		f32 a = text.a / 255.f;
		AEGfxPrint(font, text.text, x, y, text.scale, r, g, b, a);
	}
}

