#include "DebugMenus.h"
#include "MasterHeader.h"
#include "AEEngine.h"
#include "AEGraphics.h"


namespace {
	//for font
	s8 boldPixels;
	//to create the mesh
	AEGfxVertexList* pBlackRectMesh = nullptr;
	AEGfxVertexList* pWhiteRectMesh = nullptr; 
	AEGfxVertexList* pGreenRectMesh = nullptr;
	AEGfxVertexList* pRedRectMesh = nullptr;
	AEGfxVertexList* pYellowRectMesh = nullptr;

	//helper to create mesh typedef unsigned int 
	AEGfxVertexList* createmesh(uint32_t color) {
		AEGfxMeshStart();
		AEGfxTriAdd(
			-0.5f, -0.5f, color, 0.0f, 0.0f,
			0.5f, -0.5f, color, 0.0f, 0.0f,
			-0.5f, 0.5f, color, 0.0f, 0.0f);
		AEGfxTriAdd(
			0.5f, -0.5f, color, 0.0f, 0.0f,
			0.5f, 0.5f, color, 0.0f, 0.0f,
			-0.5f, 0.5f, color, 0.0f, 0.0f);
		return AEGfxMeshEnd();
	}

	//helper to draw mesh
	void drawmesh(AEGfxVertexList* mesh, float x, float y, float scale_x, float scale_y, float r = 1, float g = 1, float b = 1) {
		AEMtx33 scale, trans, final;
		AEMtx33Scale(&scale, scale_x, scale_y);
		AEMtx33Trans(&trans, x, y);
		AEMtx33Concat(&final, &trans, &scale);
		AEGfxSetTransform(final.m);
		AEGfxSetColorToMultiply(r, g, b, 1.0f);
		AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
	}

	//initialise player struct
	PlayerStats player = {
		//base stats
		100.0f, 10.0f, 5.0f, 1.0f, 1.0f,
		//initial upgrade amount
		{ 0, 0, 0, 0, 0 },
		//initial xp stats
		0.0f, 0.0f, 0,
		//initial skill point, and menu state
		0, false
	};
}

void LoadDebug1() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);

	//call helper to load mesh
	pBlackRectMesh = createmesh(0xFF000000);
	pWhiteRectMesh = createmesh(0xFFFFFFFF);
	pGreenRectMesh = createmesh(0xFF00FF00);
	pRedRectMesh = createmesh(0xFFFF0000);
	pYellowRectMesh = createmesh(0xFFFFFF00);
}

void UpdateDebug1() {
	//xp needed for each level
	float xp_needed = 100.0f + (player.player_level * 50.0f);

	//logic for levelling up
	if (player.current_xp >= xp_needed) {
		player.current_xp -= xp_needed; //ensures the excess xp gets carried over
		player.player_level++; //add level
		player.skill_point++; //add 1 sp
		player.menu_open = true; //opens menu --> then draws the menu
	}

	if (player.menu_open) {
		//condition to close menu
		if (player.skill_point <= 0 && AEInputCheckTriggered(AEVK_ESCAPE)) {
			player.menu_open = false;
		}

		if (AEInputCheckTriggered(AEVK_LBUTTON)) {
			s32 screenX, screenY; //typedef
			AEInputGetCursorPosition(&screenX, &screenY); //get cursor pos.

			//get window width & height
			s32 winWidth = AEGfxGetWindowWidth(); 
			s32 winHeight = AEGfxGetWindowHeight();

			//convert screen coords to world coords (0,0 in the middle)
			float mouseX = (float)screenX - (winWidth / 2.0f);
			float mouseY = (winHeight / 2.0f) - (float)screenY;

			//initialise the geometry for the white buttons (on the right)
			float start_y = 200.0f; //y coordinate for first button
			float spacing_y = 100.0f; //vertical gap between buttons
			float offset_middle = 260.0f; //horizontal distance from the centre
			float squaresize = 60.0f; //size of sq
			float middle_to_edge = squaresize / 2.0f; //distance from centre to edge of the sq

			//loop for 5 buttons
			for (int i = 0; i < 5; ++i) {
				float currentY = start_y - (i * spacing_y); //calculates centre point of each button
				//the AABB collision logic (to detect mouse click)
				if (mouseX >= (offset_middle - middle_to_edge) && mouseX <= (offset_middle + middle_to_edge) &&
					mouseY >= (currentY - middle_to_edge) && mouseY <= (currentY + middle_to_edge))
				{
					//skill point check
					if (player.skill_point > 0 && player.upgradeLevels[i] < 5) {
						//add one upgrade, then - 1 skill point
						player.upgradeLevels[i]++;
						player.skill_point--;
					}
				}
			}
		}
	}		
	//resume state
	else {
		//gain exp while pressing E
		if (AEInputCheckCurr(AEVK_E)) {
		float xp_multiplier = 1.0f + (player.upgradeLevels[4] * 0.5f); //get total multiplier for xp
		player.current_xp += 2.0f * xp_multiplier; //calculate player xp
		}
	}
}
			
void DrawDebug1() {
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f); //bg
	AEGfxSetRenderMode(AE_GFX_RM_COLOR); //render mode
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f); //reset colors

	//draws hud at the bottom
	drawmesh(pBlackRectMesh, 0.0f, -410.0f, 310.0f, 52.0f); 
	drawmesh(pRedRectMesh, 0.0f, -403.0f, 300.0f, 25.0f);

	//xp bar logic
	float xp_needed = 100.0f + (player.player_level * 50.0f); //base xp amount + level
	float xp_perc = player.current_xp / xp_needed; // percentage
	//anchors
	if (xp_perc > 1.0f) {
		xp_perc = 1.0f;
	}
	if (xp_perc < 0.0f) {
		xp_perc = 0.0f;
	}
	
	float current_w = 300.0f * xp_perc; //current width of bar
	drawmesh(pYellowRectMesh, (current_w - 300.0f) / 2.0f, -425.0f, current_w, 10.0f); //draw bar

	//storing different data into arrays
	const char* stats[] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP GAIN" };

	float multiplier[] = { 20.0f, 5.0f, 0.5f, 0.2f, 3.0f }; //***** IMPORTANT: CHANGE MULTIPLIERS HERE *****

	float base_stats[] = { player.baseHp, player.baseDmg, player.baseSpeed, player.baseFireRate, player.baseXpGain };

	//print level and xp progress
	char hud[64];
	sprintf_s(hud, "LEVEL: %d (press E for XP): %.1f/%.0f", player.player_level, player.current_xp, xp_needed);
	AEGfxPrint(boldPixels, hud, 0.35f, -0.85f, 0.4f, 0.0f, 0.0f, 0.0f, 1.0f);

	//prints the 5 main stats (can be removed)
	for (int i = 0; i < 5; ++i) {
		char buff[64];
		float finalStat = base_stats[i] + (player.upgradeLevels[i] * multiplier[i]);
		sprintf_s(buff, "%s: %.1f", stats[i], finalStat);
		AEGfxPrint(boldPixels, buff, -0.95f, 0.90f - (i * 0.08f), 0.4f, 0, 0, 0, 1);
	}

	//condition: open menu when leveled up
	if (player.menu_open) {
		drawmesh(pBlackRectMesh, 0.0f, 0.0f, 900.0f, 650.0f); //menu bg

		//initialise geometry for the rows (same as in update loop)
		float start_y = 200.0f, spacing_y = 100.0f, offset_middle = 260.0f; 

		for (int i = 0; i < 5; ++i) {
			float actual_y = start_y - (i * spacing_y); //calculate centre point of the rows

			//draws rows
			drawmesh(pWhiteRectMesh, 0.0f, actual_y, 400.0f, 60.0f); //long white rectangle
			drawmesh(pWhiteRectMesh, -offset_middle, actual_y, 60.0f, 60.0f); //right sq
			drawmesh(pWhiteRectMesh, offset_middle, actual_y, 60.0f, 60.0f); //left sq

			//logic for filling up grey boxes 
			for (int s = 0; s < 5; ++s) {
				//if less then current level, turn green otherwise its grey
				if (s < player.upgradeLevels[i]) {
					//uses whitemesh and turns it into green
					drawmesh(pWhiteRectMesh, -156.0f + (s * 78.0f), actual_y, 70.0f, 50.0f, 0.0f, 1.0f, 0.0f); 
				}
				else {
					//uses whitemesh and turns it to grey
					drawmesh(pWhiteRectMesh, -156.0f + (s * 78.0f), actual_y, 70.0f, 50.0f, 0.8f, 0.8f, 0.8f);
				}
			}
		}

		//condition texts on menu
		if (player.skill_point > 0) {
			AEGfxPrint(boldPixels, "SPEND POINT TO CONTINUE!", -0.23f, -0.63f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			AEGfxPrint(boldPixels, "PRESS ESC TO CLOSE!", -0.18f, -0.63f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}

void FreeDebug1() {
	//free font
	AEGfxDestroyFont(boldPixels);

	//free mesh
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
	if (pRedRectMesh) {
		AEGfxMeshFree(pRedRectMesh);
		pRedRectMesh = nullptr;
	}
	if (pYellowRectMesh) {
		AEGfxMeshFree(pYellowRectMesh);
		pYellowRectMesh = nullptr;
	}
}