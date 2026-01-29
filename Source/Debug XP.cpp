#include "DebugMenus.h"
#include "MasterHeader.h"
#include "AEEngine.h"
#include "AEGraphics.h"

//EVERYTHING USES WORLD COORDINATES
//TEXTS USE NORMALISED

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
		100.0f, 10.0f, 5.0f, 1.0f, 1.0f, //base stats
		
		{ 0, 0, 0, 0, 0 },   //initial upgrade amount
		
		0.0f, 0.0f, 0,    //initial xp stats
		
		0, false,  //initial skill point, and menu state
	
	};

	//***** IMPORTANT: CHANGE MULTIPLIERS HERE *****
	// HP , DMG , MV SPEED , FIRE RATE , XP GAIN //
	float multiplier[] = { 20.0f, 5.0f, 0.5f, 0.2f, 3.0f }; 

	//storing into arrays (mainly for printing)
	const char* stats[] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP GAIN" };

	//to put into update loop for implementation of cards
	float base_stats[] = { player.baseHp, player.baseDmg, player.baseSpeed, player.baseFireRate, player.baseXpGain };
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
	player.current_hp = player.baseHp;
}


float calculate_max_stats(int i) {
	//base stats + upgrade amount * multiplier
	return base_stats[i] + (player.upgradeLevels[i] * multiplier[i]);
}


//---- LEVEL UP LOGIC ----
void level_up(float xp_needed) {
	if (player.current_xp >= xp_needed) { //if xp threshold reached
		player.current_xp -= xp_needed; //ensures the excess xp gets carried over
		player.player_level++; //add level
		player.skill_point++; //add 1 sp
		player.menu_open = true; //opens menu

		//reset player hp for each level up
		float max_hp = calculate_max_stats(0);
		player.reset_player_hp(max_hp);

	}
}


void handle_menu_input() {
	//condition to close upgrade menu
	if (player.skill_point <= 0 && AEInputCheckTriggered(AEVK_ESCAPE)) {
		player.menu_open = false;
		return;
	}

	//logic to detect mouse click only at the right squares
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
			//point to rectangle collision (to detect mouse click)
			if (mouseX >= (offset_middle - middle_to_edge) && mouseX <= (offset_middle + middle_to_edge) &&
				mouseY >= (currentY - middle_to_edge) && mouseY <= (currentY + middle_to_edge))
			{
				//what happens after the click
				if (player.skill_point > 0 && player.upgradeLevels[i] < 5) {
					//add one upgrade, then - 1 skill point
					player.upgradeLevels[i]++;
					player.skill_point--;

					//if skill point is added to HP, add the multplier
					if (i == 0) {
						player.current_hp += multiplier[0];
					}
				}
			}
		}
	}
}


void debug_inputs(float max_hp) {
	if (AEInputCheckTriggered(AEVK_R)) {
		player.current_hp += 10.0f;
		if (player.current_hp > max_hp)
			player.current_hp = max_hp;
	}

	//lose hp with T
	if (AEInputCheckTriggered(AEVK_T)) {
		player.current_hp -= 10.0f;
		if (player.current_hp < 0.0f)
			player.current_hp = 0.0f;
	}

	//press E for xp
	if (AEInputCheckCurr(AEVK_E)) {
		float xp_multiplier = 1.0f + (player.upgradeLevels[4] * 0.5f); //get total multiplier for xp
		player.current_xp += 2.0f * xp_multiplier; //calculate player xp
	}
}


void draw_hud_bar(AEGfxVertexList* mesh, float current, float max, float y_pos, float bar_height, float max_width) {
	//get %
	float perc = current / max;
	//anchor
	if (perc > 1.0f) perc = 1.0f;
	if (perc < 0.0f) perc = 0.0f;

	//calculate actual w of the bars
	float actual_w = max_width * perc;
	//calculate offset to centre to align it to the left
	float offset = (max_width - actual_w) / 2.0f;
	drawmesh(mesh, 0.0f - offset, y_pos, actual_w, bar_height);
}


void draw_upgrade_rows() {
	//initalise white container variables
	float start_y = 200.0f, spacing_y = 100.0f, offset_middle = 260.0f;
	//initialise segment inside container variables
	float segment_w = 70.0f, gap = 8.0f;
	float total_segment_dist = segment_w + gap;
	float start_x = -(total_segment_dist * 2.0f);

	//draws each row 5 times
	for (int i = 0; i < 5; ++i) {
		float actual_y = start_y - (i * spacing_y); //formula to calculate the centre point
		drawmesh(pWhiteRectMesh, 0.0f, actual_y, 400.0f, 60.0f);
		drawmesh(pWhiteRectMesh, -offset_middle, actual_y, 60.0f, 60.0f);
		drawmesh(pWhiteRectMesh, offset_middle, actual_y, 60.0f, 60.0f);

		for (int j = 0; j < 5; ++j) {
			//draw a box at start_x, then move right by total segment distance, repeat 4 times
			float current_x = start_x + (j * total_segment_dist);

			//compares segment index with upgraded levels
			if (j < player.upgradeLevels[i]) {
				//draw green
				drawmesh(pWhiteRectMesh, current_x, actual_y, segment_w, 50.0f, 0.0f, 1.0f, 0.0f);
			}
			else {
				//draw grey
				drawmesh(pWhiteRectMesh, current_x, actual_y, segment_w, 50.0f, 0.8f, 0.8f, 0.8f);
			}
		}
	}
}


void UpdateDebug1() {
	//updates levels, and maxhp 
	float xp_needed = 100.0f + (player.player_level * 50.0f);
	float max_hp = calculate_max_stats(0);
	float max_dmg = calculate_max_stats(1);
	float max_speed = calculate_max_stats(2);
	float max_fire_rate = calculate_max_stats(3);

	//calls level up for logic
	level_up(xp_needed);

	//resume
	if (player.menu_open) {
		handle_menu_input();
	}
	//pause
	else {
		debug_inputs(max_hp);
	}
}
			

void DrawDebug1() {

	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

	//draw hub bg
	drawmesh(pBlackRectMesh, 0.0f, -410.0f, 310.0f, 52.0f);

	//draws hp and xp bar
	float max_hp = player.baseHp + (player.upgradeLevels[0] * multiplier[0]);
	float xp_needed = 100.0f + (player.player_level * 50.0f);

	draw_hud_bar(pRedRectMesh, player.current_hp, max_hp, -403.0f, 25.0f, 300.0f);
	draw_hud_bar(pYellowRectMesh, player.current_xp, xp_needed, -425.0f, 10.0f, 300.0f);

	//print hp and xp stats
	char hud[64];
	char hud2[64];
	sprintf_s(hud, "LEVEL: %d (press E for XP): %.1f/%.0f", player.player_level, player.current_xp, xp_needed);
	sprintf_s(hud2, " %.0f / %.0f (R to heal, T to lose hp)", player.current_hp, max_hp);
	AEGfxPrint(boldPixels, hud, 0.35f, -0.85f, 0.4f, 0.0f, 0.0f, 0.0f, 1.0f);
	AEGfxPrint(boldPixels, hud2, 0.35f, -0.9f, 0.4f, 0.0f, 0.0f, 0.0f, 1.0f);

	//prints the 5 main stats (can be removed)
	for (int i = 0; i < 5; ++i) {
		char buff[64];
		float finalStat = base_stats[i] + (player.upgradeLevels[i] * multiplier[i]);
		sprintf_s(buff, "%s: %.1f", stats[i], finalStat);
		AEGfxPrint(boldPixels, buff, -0.95f, 0.90f - (i * 0.08f), 0.4f, 0, 0, 0, 1);
	}

	if (player.menu_open) {
		drawmesh(pBlackRectMesh, 0.0f, 0.0f, 900.0f, 650.0f); //menu bg
		draw_upgrade_rows();
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
	AEGfxVertexList* meshes[] = { pBlackRectMesh, pWhiteRectMesh, pGreenRectMesh, pRedRectMesh, pYellowRectMesh };

	for (int i = 0; i < 5; ++i) {
		if (meshes[i]) {
			AEGfxMeshFree(meshes[i]);
		}
	}

	// reset all pointers to nullptrs
	pBlackRectMesh = pWhiteRectMesh = pGreenRectMesh = pRedRectMesh = pYellowRectMesh = nullptr;
}