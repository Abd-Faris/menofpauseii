#include "DebugMenus.h"
#include "MasterHeader.h"
#include "AEEngine.h"
#include "AEGraphics.h"
#include "Structs.h"

//EVERYTHING USES WORLD COORDINATES
//TEXTS USE NORMALISED
	
//initialise player struct
PlayerStats	player_init = {
	100.0f, 10.0f, 300.0f, 0.25f, 1.0f, //base stats

	{ 0, 0, 0, 0, 0 },   //initial upgrade amount

	0.0f, 0.0f, 0,    //initial xp stats

	0, false,  //initial skill point, and menu state

};

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
		if (mesh == nullptr) {
			return;
		}
		AEMtx33 scale, trans, final;
		AEMtx33Scale(&scale, scale_x, scale_y);
		AEMtx33Trans(&trans, x, y);
		AEMtx33Concat(&final, &trans, &scale);
		AEGfxSetTransform(final.m);
		AEGfxSetColorToMultiply(r, g, b, 1.0f);
		AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
	}


	//***** IMPORTANT: CHANGE MULTIPLIERS HERE *****
	// HP , DMG , MV SPEED , FIRE RATE , XP GAIN //
	float multiplier[] = { 25.0f, 4.0f, 30.0f, 0.04f, 0.5f }; 

	//storing into arrays (mainly for printing)
	const char* stats[] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP GAIN" };

	//to put into update loop for implementation of cards
	float base_stats[] = { player_init.baseHp, player_init.baseDmg, player_init.baseSpeed, player_init.baseFireRate, player_init.baseXpGain };
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

	//resets player hp to current hp
	player_init.current_hp = player_init.baseHp;
}


void reset_game() {
	//reset player stats
	player_init.player_level = 0;
	player_init.current_xp = 0;
	player_init.skill_point = 0;
	player_init.menu_open = false;

	//fill upgrade with 0
	for (int i = 0; i < 5; ++i) {
		player_init.upgradeLevels[i] = 0;
	}
	player_init.current_hp = player_init.baseHp; //set base hp to current hp
}


float calculate_max_stats(int i) {
	//base stats + upgrade amount * multiplier
	switch (i) {
	case 0: return player_init.baseHp + (player_init.upgradeLevels[0] * multiplier[0]);
	case 1: return player_init.baseDmg + (player_init.upgradeLevels[1] * multiplier[1]);
	case 2: return player_init.baseSpeed + (player_init.upgradeLevels[2] * multiplier[2]);
	case 3: return player_init.baseFireRate - (player_init.upgradeLevels[3] * multiplier[3]);
	case 4: return player_init.baseXpGain + (player_init.upgradeLevels[4] * multiplier[4]);
	default: return 0.0f;
	}
}


//---- LEVEL UP LOGIC ----
void level_up(float xp_needed) {
	if (player_init.current_xp >= xp_needed) { //if xp threshold reached
		player_init.current_xp -= xp_needed; //ensures the excess xp gets carried over
		player_init.player_level++; //add level
		player_init.skill_point++; //add 1 sp
		player_init.menu_open = true; //opens menu

		//heal player hp every level up
		float max_hp = calculate_max_stats(0);
		player_init.current_hp += max_hp * 0.2f;

		if (player_init.current_hp > max_hp) {
			player_init.current_hp = max_hp;
		}
	}
}


void handle_menu_input(float camX, float camY) {
	if (player_init.skill_point <= 0 && AEInputCheckTriggered(AEVK_ESCAPE)) {
		player_init.menu_open = false;
		return;
	}

	if (AEInputCheckTriggered(AEVK_LBUTTON)) {
		s32 screenX, screenY;
		AEInputGetCursorPosition(&screenX, &screenY);

		s32 winWidth = AEGfxGetWindowWidth();
		s32 winHeight = AEGfxGetWindowHeight();

		// This is your mouse relative to the center of the screen (e.g., 0,0 is center)
		float mouseX = (float)screenX - (winWidth / 2.0f);
		float mouseY = (winHeight / 2.0f) - (float)screenY;

		float start_y = 200.0f;
		float spacing_y = 100.0f;
		float offset_middle = 260.0f;
		float squaresize = 60.0f;
		float middle_to_edge = squaresize / 2.0f;

		for (int i = 0; i < 5; ++i) {
			// DO NOT add camX/camY here. 
			// We want the STATIC screen position of the buttons.
			float buttonX = offset_middle;
			float buttonY = start_y - (i * spacing_y);

			// Compare screen-relative mouse to screen-relative button
			if (mouseX >= (buttonX - middle_to_edge) && mouseX <= (buttonX + middle_to_edge) &&
				mouseY >= (buttonY - middle_to_edge) && mouseY <= (buttonY + middle_to_edge))
			{
				if (player_init.skill_point > 0 && player_init.upgradeLevels[i] < 5) {
					player_init.upgradeLevels[i]++;
					player_init.skill_point--;
					if (i == 0) player_init.current_hp += multiplier[0];
				}
			}
		}
	}
}


void debug_inputs(float max_hp) {
	if (AEInputCheckTriggered(AEVK_R)) {
		player_init.current_hp += 10.0f;
		if (player_init.current_hp > max_hp)
			player_init.current_hp = max_hp;
	}

	//lose hp with T
	if (AEInputCheckTriggered(AEVK_T)) {
		player_init.current_hp -= 10.0f;
		if (player_init.current_hp < 0.0f)
			player_init.current_hp = 0.0f;
	}

	//press E for xp
	if (AEInputCheckCurr(AEVK_E)) {
		float xp_multiplier = 1.0f + (player_init.upgradeLevels[4] * 0.5f); //get total multiplier for xp
		player_init.current_xp += 5.0f * xp_multiplier; //calculate player xp
	}
}


void draw_hud_bar(AEGfxVertexList* mesh, float current, float max, float anchorX, float anchorY, float relativeY, float bar_height, float max_width) {
	float perc = current / max;
	if (perc > 1.0f) perc = 1.0f;
	if (perc < 0.0f) perc = 0.0f;

	float actual_w = max_width * perc;

	float shiftRight = (max_width - actual_w) / 2.0f;

	float finalX = anchorX - shiftRight;
	float finalY = anchorY + relativeY;

	drawmesh(mesh, finalX, finalY, actual_w, bar_height);
}


void draw_upgrade_rows(float camX, float camY) {
	//initalise white container variables
	float start_y = 200.0f, spacing_y = 100.0f, offset_middle = 260.0f;
	//initialise segment inside container variables
	float segment_w = 70.0f, gap = 8.0f;
	float total_segment_dist = segment_w + gap;
	float start_x = -(total_segment_dist * 2.0f);

	//draws each row 5 times
	for (int i = 0; i < 5; ++i) {
		float actual_y = start_y - (i * spacing_y); //formula to calculate the centre point
		drawmesh(pWhiteRectMesh, 0.0f + camX, actual_y + camY, 400.0f, 60.0f);
		drawmesh(pWhiteRectMesh, -offset_middle + camX, actual_y + camY, 60.0f, 60.0f);
		drawmesh(pWhiteRectMesh, offset_middle + camX, actual_y + camY, 60.0f, 60.0f);

		for (int j = 0; j < 5; ++j) {
			//draw a box at start_x, then move right by total segment distance, repeat 4 times
			float current_x = start_x + (j * total_segment_dist);

			//compares segment index with upgraded levels
			if (j < player_init.upgradeLevels[i]) {
				//draw green
				drawmesh(pWhiteRectMesh, current_x + camX, actual_y + camY, segment_w, 50.0f, 0.0f, 1.0f, 0.0f);
			}
			else {
				//draw grey
				drawmesh(pWhiteRectMesh, current_x + camX, actual_y + camY, segment_w, 50.0f, 0.8f, 0.8f, 0.8f);
			}
		}
	}
}


void UpdateDebug1() {
	if (player_init.current_hp <= 0) {
		reset_game();
	}

	float camX, camY;
	AEGfxGetCamPosition(&camX, &camY);

	//updates levels, and maxhp 
	float xp_needed = 100.0f + (player_init.player_level * 50.0f);
	float max_hp = calculate_max_stats(0);
	float max_dmg = calculate_max_stats(1);
	float max_speed = calculate_max_stats(2);
	float max_fire_rate = calculate_max_stats(3);

	//calls level up for logic
	level_up(xp_needed);

	//resume
	if (player_init.menu_open) {
		handle_menu_input(camX, camY);
	}
	//pause
	else {
		debug_inputs(max_hp);
	}
}
			

void DrawDebug1() {

	float camX, camY;
	AEGfxGetCamPosition(&camX, &camY);

	float hudX = camX + 0.0f;
	float hudY = camY - 410.0f;

	float max_width = 300.0f;
	float max_hp = calculate_max_stats(0);
	float xp_needed = 100.0f + (player_init.player_level * 50.0f);

	// --- DRAW HUD ---
	drawmesh(pBlackRectMesh, hudX, hudY, max_width + 10.0f, 52.0f);
	draw_hud_bar(pRedRectMesh, player_init.current_hp, max_hp, hudX, hudY, 7.0f, 25.0f, max_width);
	draw_hud_bar(pYellowRectMesh, player_init.current_xp, xp_needed, hudX, hudY, -15.0f, 10.0f, max_width);

	//print hp and xp stats
	char hud[64];
	char hud2[64];
	sprintf_s(hud, "LEVEL: %d (press E for XP): %.1f/%.0f", player_init.player_level, player_init.current_xp, xp_needed);
	sprintf_s(hud2, " %.0f / %.0f (R to heal, T to lose hp)", player_init.current_hp, max_hp);
	AEGfxPrint(boldPixels, hud, 0.35f, -0.85f, 0.4f, 0.0f, 0.0f, 0.0f, 1.0f);
	AEGfxPrint(boldPixels, hud2, 0.35f, -0.9f, 0.4f, 0.0f, 0.0f, 0.0f, 1.0f);

	//print the 5 main stats
	for (int i = 0; i < 5; ++i) {
		char buff[64];
		float finalStat = calculate_max_stats(i);
		sprintf_s(buff, "%s: %.1f", stats[i], finalStat);
		AEGfxPrint(boldPixels, buff, -0.95f, 0.90f - (i * 0.08f), 0.4f, 0, 0, 0, 1);
	}

	//draw upgrade menu
	if (player_init.menu_open) {
		//menu bg
		drawmesh(pBlackRectMesh, 0.0f + camX, 0.0f + camY, 900.0f, 650.0f);
		draw_upgrade_rows(camX, camY);

		if (player_init.skill_point > 0) {
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
	AEGfxVertexList* meshes[] = { pBlackRectMesh, pWhiteRectMesh, pGreenRectMesh, pRedRectMesh, pYellowRectMesh };

	for (int i = 0; i < 5; ++i) {
		if (meshes[i]) {
			AEGfxMeshFree(meshes[i]);
		}
	}

	// reset all pointers to nullptrs
	pBlackRectMesh = pWhiteRectMesh = pGreenRectMesh = pRedRectMesh = pYellowRectMesh = nullptr;
}