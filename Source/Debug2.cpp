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
	const int MAX_BULLETS = 1000; 
	bullets bulleting[MAX_BULLETS];
	float protrusionlen = 300.0f;
	float protrusionwid = 30.0f;
	float bullettimer = 0.0f;
	float bulletinterval = 0.05f;
	float bulletsize = 15.0f;


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


		//test enemy
		AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Red Color

		AEMtx33 enemyScale, enemyTrans, enemyFinal;
		// Make it a 60x60 square
		AEMtx33Scale(&enemyScale, 60.f, 60.f);
		// Place it at fixed world coordinates (400, 300)
		AEMtx33Trans(&enemyTrans, 400.f, 300.f);

		// Combine scale and translation
		AEMtx33Concat(&enemyFinal, &enemyTrans, &enemyScale);

		// Draw it using your existing Square mesh
		AEGfxSetTransform(enemyFinal.m);
		AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
		// end enemy

		AEGfxSetCamPosition(circle.pos_x, circle.pos_y);
		//Draw square


		AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1.0f);

		AEMtx33 transformSquare, scaleSquare, translateSquare;
		AEMtx33Identity(&transformSquare);

		
		float squareSize = 150.0f;
		AEMtx33Scale(&scaleSquare, squareSize, squareSize);

		
		AEMtx33Trans(&translateSquare, circle.pos_x, circle.pos_y);

		
		AEMtx33Concat(&transformSquare, &translateSquare, &scaleSquare);

		AEGfxSetTransform(transformSquare.m);
		AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);

		//draw bullets
		if (AEInputCheckCurr(AEVK_SPACE)) {
			bullettimer -= dt;
			if (bullettimer <= 0) {
				for (int i = 0; i < MAX_BULLETS; i++) {
					if (!bulleting[i].active) {
						bulleting[i].active = true;
						bulleting[i].directx = cosf(circle.currentAngle + 1.5708f);
						bulleting[i].directy = sinf(circle.currentAngle + 1.5708f);
						f32 dirx = bulleting[i].directx;
						f32 diry = bulleting[i].directy;

						bulleting[i].x = circle.pos_x + (dirx * protrusionlen);
						bulleting[i].y = circle.pos_y + (diry * protrusionlen);
						bulleting[i].speed = 500.0f;


						bullettimer = bulletinterval;
						break;
					}
				}
			}
		}
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (bulleting[i].active) {

				bulleting[i].x += bulleting[i].directx * bulleting[i].speed * dt;
				bulleting[i].y += bulleting[i].directy * bulleting[i].speed * dt;

				if ((abs(bulleting[i].x) > 1200) || (abs(bulleting[i].y) > 800)) {
						bulleting[i].active = false;
				}
			}
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (bulleting[i].active) {
				AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f); //yellow Circle
				AEMtx33 transformbullet;
				AEMtx33Identity(&transformbullet);
				AEMtx33 scalebullet;
				AEMtx33Scale(&scalebullet, bulletsize, bulletsize);
				AEMtx33 translatebullet;
				AEMtx33Trans(&translatebullet, bulleting[i].x, bulleting[i].y);
				AEMtx33Concat(&transformbullet, &translatebullet, &scalebullet);

				// Choose the transform to use
				AEGfxSetTransform(transformbullet.m);
				// Actually drawing the mesh 
				AEGfxMeshDraw(MeshCircle, AE_GFX_MDM_TRIANGLES);
			}
		}
		

		// draw protrusion

		//get mouse position
		s32 mousex, mousey;
		AEInputGetCursorPosition(&mousex, &mousey);

		float windowh = 900.0f;
		float windoww = 1600.0f;

		float mouseposx = (float)mousex - windoww / 2.0f;
		float mouseposy = windowh / 2.0f - (float)mousey;

		float finalmousex = mouseposx + circle.pos_x;
		float finalmousey = mouseposy + circle.pos_y;

		//distance between mouse and player
		float distxpm = finalmousex - circle.pos_x;
		float distypm = finalmousey - circle.pos_y;

		if ((distxpm * distxpm + distypm * distypm) > 1.0f)
		{
			float rotationSpeed = 0.1f;
			float targetangle = atan2f(distypm, distxpm) - 1.5708f; // arctangent find angle 1.5708f is half of pi
			float diffangle = targetangle - circle.currentAngle;

			while (diffangle > 3.1415f)
			{
				diffangle -= 6.2831f;
			}
			while (diffangle < -3.1415f)
			{
				diffangle += 6.2831f;
			}

			// Smoothly update the persistent angle
			circle.currentAngle += diffangle * rotationSpeed;
		}

		// Use circle.currentAngle in your AEMtx33Rot call
		


		AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1.0f);

		AEMtx33 transformProtrusion, scaleProtrusion, translateProtrusion, transProtrusionmove, rotateProtrusion;
		AEMtx33Identity(&transformProtrusion);
		AEMtx33 interval;

		
		AEMtx33Scale(&scaleProtrusion, protrusionwid, protrusionlen);

		AEMtx33Trans(&transProtrusionmove, 0.0f, protrusionlen / 2);// move the base of the protrusion to the base

		//AEMtx33Rot(&rotateProtrusion, angle); // rotate reference
		AEMtx33Rot(&rotateProtrusion, circle.currentAngle);
		AEMtx33Trans(&translateProtrusion, circle.pos_x, circle.pos_y);

		AEMtx33Concat(&interval	, &transProtrusionmove, &scaleProtrusion); // attachment
		AEMtx33Concat(&interval, &rotateProtrusion, &interval);
		AEMtx33Concat(&transformProtrusion, &translateProtrusion, &interval); //playerposition*(rotation*offset*scale)

		AEGfxSetTransform(transformProtrusion.m);
		AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);





		
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