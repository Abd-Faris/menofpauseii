#include "DebugMenus.h"
#include "MasterHeader.h"
#include "AEEngine.h"
#include "AEGraphics.h"

//player base stats
struct PlayerStats {
	float baseHp;
	float baseDmg;
	float baseSpeed;
	float baseFireRate;
	float baseXpGain;
	//number of times it can upgrade
	int upgradeLevels[5];
};

namespace {
	s8 boldPixels;
	//to create the mesh
	AEGfxVertexList* pBlackRectMesh = nullptr;
	AEGfxVertexList* pWhiteRectMesh = nullptr; 
	AEGfxVertexList* pGreenRectMesh = nullptr;
	//initialise the player stats
	PlayerStats player = {
		//base values
		100.0f, 10.0f, 5.0f, 1.0f, 1.0f,
		//initial upgrade value
		{ 0, 0, 0, 0, 0 }                
	};
}

void LoadDebug1() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);

	//create black mesh
	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
	pBlackRectMesh = AEGfxMeshEnd();

	//create white mesh
	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pWhiteRectMesh = AEGfxMeshEnd();

	//create green mesh
	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF00FF00, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF00FF00, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF00FF00, 0.0f, 0.0f);
	AEGfxTriAdd(
		0.5f, -0.5f, 0xFF00FF00, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF00FF00, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF00FF00, 0.0f, 0.0f);
	pGreenRectMesh = AEGfxMeshEnd();
}

void UpdateDebug1() {
	//typedef
	s32 screenX, screenY;
	//get cursor pos.
	AEInputGetCursorPosition(&screenX, &screenY);
	//window width and height
	s32 winWidth = AEGfxGetWindowWidth();
	s32 winHeight = AEGfxGetWindowHeight();

	//convert screen coords to world coords (0,0 in the middle)
	float mouseX = (float)screenX - (winWidth / 2.0f);
	float mouseY = (winHeight / 2.0f) - (float)screenY;

	if (AEInputCheckTriggered(AEVK_LBUTTON)) {
		//initialise the geometry for the white buttons (on the right)
		float startY = 200.0f; //vertical anchor
		float spacingY = 100.0f; //vertical gap between rows
		float sideOffset = 260.0f; //horizontal distance from the centre
		float squareSize = 60.0f; //scale
		float halfSq = squareSize / 2.0f;

		//for loop for vertical offset
		for (int i = 0; i < 5; ++i) {
			float currentY = startY - (i * spacingY);
			//the AABB collision logic (to detect mouse click)
			if (mouseX >= (sideOffset - halfSq) && mouseX <= (sideOffset + halfSq) &&
				mouseY >= (currentY - halfSq) && mouseY <= (currentY + halfSq))
			{
				//increment the array by 1 in player struct
				if (player.upgradeLevels[i] < 5) {
					player.upgradeLevels[i]++;
				}
			}
		}
	}
}

void DrawDebug1() {
	//set bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	//set render mode
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	//define the matrix
	AEMtx33 scaleMtx, transMtx, finalMtx;

	//draw the menubg (black)
	AEMtx33Scale(&scaleMtx, 800.0f, 600.0f);
	AEMtx33Trans(&transMtx, 0.0f, 0.0f);
	AEMtx33Concat(&finalMtx, &transMtx, &scaleMtx);
	AEGfxSetTransform(finalMtx.m);
	AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
	AEGfxMeshDraw(pBlackRectMesh, AE_GFX_MDM_TRIANGLES);

	// IMPORTANT: needs to be the same as the ones initialised in update function
	float startY = 200.0f; //vertical anchor
	float spacingY = 100.0f; //vertical gap between rows
	float sideOffset = 260.0f; //horizontal distance from the centre
	float squareSize = 60.0f; //scale

	//main rectangle loop for each stat
	for (int i = 0; i < 5; ++i) {
		//calculates the vertical pos for each row
		float currentY = startY - (i * spacingY);

		//draw white container
		AEMtx33Scale(&scaleMtx, 400.0f, 60.0f);
		AEMtx33Trans(&transMtx, 0.0f, currentY);
		AEMtx33Concat(&finalMtx, &transMtx, &scaleMtx);
		AEGfxSetTransform(finalMtx.m);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		AEGfxMeshDraw(pWhiteRectMesh, AE_GFX_MDM_TRIANGLES);

		//draw progress segments
		float segmentWidth = 70.0f; //horizontal width of a segment
		float segmentGap = 78.0f; //distance between the centres of each segment
		float leftStart = -156.0f; //the x of the first segment

		//loop for green and grey segments
		for (int s = 0; s < 5; ++s) {
			AEMtx33Scale(&scaleMtx, segmentWidth, 50.0f);
			//multiply the gaps based on index s
			AEMtx33Trans(&transMtx, leftStart + (s * segmentGap), currentY);
			AEMtx33Concat(&finalMtx, &transMtx, &scaleMtx);
			AEGfxSetTransform(finalMtx.m);

			//to fill with green by comparing index s with upgrade level array in player struct
			//if less than, then fill it with green
			if (s < player.upgradeLevels[i]) {
				AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
				AEGfxMeshDraw(pGreenRectMesh, AE_GFX_MDM_TRIANGLES);
			}
			// else fill with grey
			else {
				AEGfxSetColorToMultiply(0.8f, 0.8f, 0.8f, 1.0f);
				AEGfxMeshDraw(pWhiteRectMesh, AE_GFX_MDM_TRIANGLES);
			}
		}

		//draw side squares
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		//left squares (to put textures of different stats on the mesh)
		AEMtx33Scale(&scaleMtx, squareSize, squareSize);
		AEMtx33Trans(&transMtx, -sideOffset, currentY);
		AEMtx33Concat(&finalMtx, &transMtx, &scaleMtx);
		AEGfxSetTransform(finalMtx.m);
		AEGfxMeshDraw(pWhiteRectMesh, AE_GFX_MDM_TRIANGLES);

		//right squares (to upgrade the stats)
		AEMtx33Scale(&scaleMtx, squareSize, squareSize);
		AEMtx33Trans(&transMtx, sideOffset, currentY);
		AEMtx33Concat(&finalMtx, &transMtx, &scaleMtx);
		AEGfxSetTransform(finalMtx.m);
		AEGfxMeshDraw(pWhiteRectMesh, AE_GFX_MDM_TRIANGLES);
	}

	//to show the stats on the top left
	const char* statNames[5] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP GAIN" };

	//calculating upgraded stats (can be modified)
	for (int i = 0; i < 5; ++i) {
		char buffer[64];
		float finalStat = 0.0f;

		//switch for all the stats
		switch (i) {
		case 0: //hp
			finalStat = player.baseHp + (player.upgradeLevels[i] * 20.0f);
			break;
		case 1: //dmg
			finalStat = player.baseDmg + (player.upgradeLevels[i] * 5.0f);
			break;
		case 2: //speed
			finalStat = player.baseSpeed + (player.upgradeLevels[i] * 0.5f);
			break;
		case 3: //fire rate
			finalStat = player.baseFireRate + (player.upgradeLevels[i] * 0.2f);
			break;
		case 4: //xp gain
			finalStat = player.baseXpGain + (player.upgradeLevels[i] * 0.1f);
			break;
		}

		//print text (to be removed later just added for clarity)
		sprintf_s(buffer, "%s: %.1f", statNames[i], finalStat);

		float textX = -0.95f;
		float textY = 0.90f - (i * 0.1f);

		AEGfxPrint(boldPixels, buffer, textX, textY, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f);
	}
}

void FreeDebug1() {
	// free font
	AEGfxDestroyFont(boldPixels);
	if (pBlackRectMesh) {
		AEGfxMeshFree(pBlackRectMesh);
		pBlackRectMesh = nullptr;
	}
	if (pWhiteRectMesh) {
		AEGfxMeshFree(pWhiteRectMesh);
		pWhiteRectMesh = nullptr;
	}
	if (pGreenRectMesh) { 
		AEGfxMeshFree(pGreenRectMesh); 
		pGreenRectMesh = nullptr;
	}
}