#include "MasterHeader.h"
#include "DebugMenus.h"
#include "AEEngine.h"
#include "AEGraphics.h"
#include <crtdbg.h> 
#include "AEMath.h"
#include <cmath>

namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 1};
	gfxtext texts[NUM_OF_TEXTS] = {
		{"Player + Shooting", 0, 0, 1, 255, 255, 255, 255}
	};
	AEGfxVertexList* MeshCircle{};
	AEGfxVertexList* MeshRect{};
	shape circle = { 0 };

}

int gGameRunning = 1;

void LoadDebug2() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
	

	AEGfxMeshStart();

	int slices = 40;
	float angle = 2.0f * 3.1415f / slices;

	for (int i = 0; i < slices; i++) {
		AEGfxTriAdd(
			0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,
			cosf(i * angle), sinf(i * angle), 0xFFFFFFFF, 1.0f, 1.0f,
			cosf((i + 1) * angle), sinf((i + 1) * angle), 0xFFFFFFFF, 0.0f, 0.0f);
	}

	// Saving the mesh (list of triangles) in MeshCircle
	MeshCircle = AEGfxMeshEnd();

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	AEGfxTriAdd(
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	// Saving the mesh (list of triangles) in MeshRect
	MeshRect = AEGfxMeshEnd();
	// Initialize circles
	

	//middle circle
	circle.scale = 50.f;
	circle.pos_x = 0.f;
	circle.pos_y = 0.f;
	float midradius = circle.scale;

	
}

void DrawDebug2() {

	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
		Graphics::printText(texts[i], boldPixels);
	}


		AESysFrameStart();
		// Set the background to black.
		AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

		// Tell the engine to get ready to draw something coloured
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);


		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		//movement controls
		f32 dt = (f32)AEFrameRateControllerGetFrameTime();
		f32 move = 500.f;
		if (AEInputCheckCurr(AEVK_W)) {
			circle.pos_y += dt * move;
		}
		if (AEInputCheckCurr(AEVK_S)) {
			circle.pos_y -= dt * move;
		}
		if (AEInputCheckCurr(AEVK_D)) {
			circle.pos_x += dt * move;
		}
		if (AEInputCheckCurr(AEVK_A)) {
			circle.pos_x -= dt * move;
		}
		//Draw square
		AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1.0f);

		AEMtx33 transformSquare, scaleSquare, translateSquare;
		AEMtx33Identity(&transformSquare);

		
		float squareSize = 100.0f;
		AEMtx33Scale(&scaleSquare, squareSize, squareSize);

		
		AEMtx33Trans(&translateSquare, circle.pos_x, circle.pos_y);

		
		AEMtx33Concat(&transformSquare, &translateSquare, &scaleSquare);

		AEGfxSetTransform(transformSquare.m);
		AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);

		// draw protrusion
		AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1.0f);
		static float rotateangle{};
		float rotatespeed = 5.0f;
		rotateangle = dt * rotatespeed;
		
		//draw circle
		AEGfxSetColorToMultiply(0.0f, 0.5f, 1.0f, 1.0f); //Blue Circle

		
		AEMtx33 transform;
		AEMtx33Identity(&transform);



		AEMtx33 scale;
		AEMtx33Scale(&scale, circle.scale, circle.scale);



		AEMtx33 translate;
		AEMtx33Trans(&translate, circle.pos_x, circle.pos_y);
		AEMtx33Concat(&transform, &translate, &scale);

		// Choose the transform to use
		AEGfxSetTransform(transform.m);
		// Actually drawing the mesh 
		AEGfxMeshDraw(MeshCircle, AE_GFX_MDM_TRIANGLES);
	
}

void FreeDebug2() {
	// free font
	AEGfxDestroyFont(boldPixels);
}