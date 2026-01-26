#include "MasterHeader.h"

f64 dt{};

Enemies maxenemy[20]{};

namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 1};
	gfxtext texts[NUM_OF_TEXTS] = {
		{"Debug 3", 0, 0, 1, 255, 255, 255, 255}
	};
	AEGfxVertexList* MeshRect{};
	
}

void DrawEnemyMesh() {
	
	

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
}

void InitEnemies(Enemies *enemy) {
	enemy->pos_x = 67676767; // spawn way outside player boundary
	enemy->pos_y = 67676767; // spawn way outside player boundary
	enemy->scale = 0;
	enemy->xp = 0;
	enemy->rotation = 0;//static_cast<float>(AERandFloat() * 360);
	enemy->alive = false;
	enemy->hp = 0;
}

void SpawnEnemies(Enemies* enemy) {
	enemy->pos_x = static_cast<float>((AERandFloat() * AEGfxGetWindowWidth()) - AEGfxGetWindowWidth()/2);
	enemy->pos_y = static_cast<float>((AERandFloat() * AEGfxGetWindowHeight()) - AEGfxGetWindowHeight() / 2);
	enemy->scale = 100;
	enemy->xp = 5;
	enemy->rotation = static_cast<float>(AERandFloat() * 360);
	enemy->alive = true;
	enemy->hp = 5;
}

void LoadDebug3() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
	DrawEnemyMesh();
	
	for (int i = 0; i < 20; i++) {
		InitEnemies(&maxenemy[i]);
	}

}

void DrawDebug3() {

	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
		Graphics::printText(texts[i], boldPixels);
	}

	dt += AEFrameRateControllerGetFrameTime();

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1.0f);


	if (dt >= 1) {
		if (AERandFloat() * 10 >= 2) {
			for (int i = 0; i < 20; i++) {

				if (!maxenemy[i].alive) {
					SpawnEnemies(&maxenemy[i]);
					break;
				}
			}
		}

		dt = 0;
	}
	for (int i = 0; i < 20; i++) {

		if (maxenemy[i].alive) {
		AEMtx33 transformSquare, scaleSquare, translateSquare, rotateSquare;
		AEMtx33Identity(&transformSquare);



		AEMtx33Scale(&scaleSquare, maxenemy[i].scale, maxenemy[i].scale);

		AEMtx33RotDeg(&rotateSquare, maxenemy[i].rotation);

		AEMtx33Trans(&translateSquare, maxenemy[i].pos_x, maxenemy[i].pos_y);



		AEMtx33Concat(&transformSquare, &translateSquare, &rotateSquare);

		AEMtx33Concat(&transformSquare, &transformSquare, &scaleSquare);

		AEGfxSetTransform(transformSquare.m);
		AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
		}
}



	
}

void FreeDebug3() {
	// free font
	AEGfxDestroyFont(boldPixels);
	AEGfxMeshFree(MeshRect);
}