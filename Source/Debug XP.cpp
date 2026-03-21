#include "MasterHeader.h"

// --- GLOBALS FROM GAME.CPP ---
extern std::array<Enemies, 50> enemyPool;
extern int currentWave;

// --- INITIAL PLAYER STATS ---
PlayerStats	player_init = {
	// -- HP DMG SPEED FIRERATE XP --
	200.0f, 10.0f, 280.0f, 0.5f, 1.0f,

	{ 0, 0, 0, 0, 0 },   //initial upgrade amount

	0.0f, 0.0f, 0,    //initial xp stats

	0, false,  //initial skill point, and menu state

	200.0f // current hp
};

namespace {

	//color meshes
	AEGfxVertexList* pBlackRectMesh = nullptr;
	AEGfxVertexList* pWhiteRectMesh = nullptr;
	AEGfxVertexList* pGreenRectMesh = nullptr;
	AEGfxVertexList* pRedRectMesh = nullptr;
	AEGfxVertexList* pYellowRectMesh = nullptr;

	//mesh for icons
	AEGfxVertexList* pIconMesh = nullptr;

	//stat icons
	AEGfxTexture* upgradeIcons[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	const char* iconPaths[5] = {
		"Assets/hp.png",
		"Assets/dmg.png",
		"Assets/speed.png",
		"Assets/firerate.png",
		"Assets/xp.png"
	};

	//+ icon
	AEGfxTexture* upgradeConfirmIcon = nullptr;

	//helper to create color mesh
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

	//helper to draw color mesh
	void drawmesh(AEGfxVertexList* mesh, float x, float y, float scale_x, float scale_y, float r = 1, float g = 1, float b = 1) {
		if (mesh == nullptr) return;
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEMtx33 scale, trans, final;
		AEMtx33Scale(&scale, scale_x, scale_y);
		AEMtx33Trans(&trans, x, y);
		AEMtx33Concat(&final, &trans, &scale);
		AEGfxSetTransform(final.m);
		AEGfxSetColorToMultiply(r, g, b, 1.0f);
		AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
	}

	//helper to draw textured icon
	void drawicon(AEGfxTexture* tex, float x, float y, float size) {
		if (tex == nullptr || pIconMesh == nullptr) return;
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxTextureSet(tex, 0, 0);
		AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
		AEGfxSetTransparency(1.0f);
		AEMtx33 scale, trans, final;
		AEMtx33Scale(&scale, size, size);
		AEMtx33Trans(&trans, x, y);
		AEMtx33Concat(&final, &trans, &scale);
		AEGfxSetTransform(final.m);
		AEGfxMeshDraw(pIconMesh, AE_GFX_MDM_TRIANGLES);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR); // reset back to color mode
	}

	//helper to draw enemy health bar
	void draw_enemy_health_bar(Enemies& enemy, float camX, float camY) {
		if (!enemy.alive || enemy.hp >= enemy.maxhp || enemy.hp <= 0) return;

		float barWidth = enemy.scale;
		float barHeight = 6.0f;
		float yOffset = -(enemy.scale * 0.9f);

		float perc = (float)enemy.hp / (float)enemy.maxhp;
		if (perc < 0.0f) perc = 0.0f;

		drawmesh(pBlackRectMesh, enemy.pos.x, enemy.pos.y + yOffset, barWidth + 4.0f, barHeight + 2.0f);

		float actualW = barWidth * perc;
		float shiftX = (barWidth - actualW) / 2.0f;
		drawmesh(pRedRectMesh, enemy.pos.x - shiftX, enemy.pos.y + yOffset, actualW, barHeight, 1.0f, 0.0f, 0.0f);
	}

	//***** IMPORTANT: CHANGE MULTIPLIERS HERE *****
	// HP , DMG , MV SPEED , FIRE RATE , XP GAIN //
	float multiplier[] = { 25.0f, 4.0f, 30.0f, 0.033f, 0.5f };

	//storing into arrays (mainly for printing)
	const char* stats[] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP GAIN" };

	//to put into update loop for implementation of cards
	float base_stats[] = { player_init.baseHp, player_init.baseDmg, player_init.baseSpeed, player_init.baseFireRate, player_init.baseXpGain };

	//floating xp text pop up variables
	float xpPopuptimer = 0.0f;
	float xpPopupduration = 0.7f;
	float xpPopupvalue = 0.0f;
}


void LoadDebug1() {
	// load font
	//boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);

	// create color meshes
	pBlackRectMesh = createmesh(0xFF000000);
	pWhiteRectMesh = createmesh(0xFFFFFFFF);
	pGreenRectMesh = createmesh(0xFF00FF00);
	pRedRectMesh = createmesh(0xFFFF0000);
	pYellowRectMesh = createmesh(0xFFFFFF00);

	// create UV-mapped mesh for icons
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pIconMesh = AEGfxMeshEnd();

	// load per-stat icons (left squares)
	for (int i = 0; i < 5; ++i) {
		upgradeIcons[i] = AEGfxTextureLoad(iconPaths[i]);
	}

	// load confirm icon (right squares)
	upgradeConfirmIcon = AEGfxTextureLoad("Assets/plus.png");
}


//  --- FUNCTION TO TRIGGER FLOATING XP TEXT ---
void TriggerXpPopup(float xpAmount) {
	xpPopupvalue = xpAmount;
	xpPopuptimer = xpPopupduration;
}

// --- FUNCTION TO RESET GAME  ---
void reset_game() {
	// reset wave
	currentWave = 1;

	// clear boss
	boss.alive = false;

	// clear minions
	for (auto& minion : minionPool) {
		minion.alive = false;
		minion.hp = 0;
	}

	// reset wave spawning
	pendingBudget = 0.0f;
	spawnTimer = 0.0f;
	totalWaveBudget = 0.0f;

	// reset enemy
	for (auto& enemy : enemyPool) {
		enemy.alive = false;
		enemy.hp = 0;
	}
	 
	// reset bullets
	for (auto& bullet : bulletList) {
		bullet.isActive = false;
	}
	for (auto& enBullet : enemyBulletList) {
		enBullet.isActive = false;
	}

	player_init.player_level = 0;
	player_init.current_xp = 0;
	player_init.skill_point = 0;
	player_init.menu_open = false;
	for (int i = 0; i < 5; ++i) {
		player_init.upgradeLevels[i] = 0;
	}
	player_init.current_hp = player_init.baseHp;
}



float calculate_max_stats(int i) {
	// FORMULA:
	// (base + cardBaseMod + (upgradeLevel * multiplier)) * cardMultMod
	switch (i) {
		// for hp: (base + cardBaseMod) * cardMultMod + (upgrade level * mutiplier) (to make sure upgrading for skill points are flat)
	case 0: return ((player_init.baseHp + cardBaseMod.hp) * cardMultMod.hp) + (player_init.upgradeLevels[0] * multiplier[0]);
	case 1: return (player_init.baseDmg + cardBaseMod.dmg + (player_init.upgradeLevels[1] * multiplier[1])) * cardMultMod.dmg;
	case 2: return (player_init.baseSpeed + cardBaseMod.moveSpeed + (player_init.upgradeLevels[2] * multiplier[2])) * cardMultMod.moveSpeed;
	case 3: {
		float result = (player_init.baseFireRate - cardBaseMod.fireRate - (player_init.upgradeLevels[3] * multiplier[3])) * cardMultMod.fireRate;
		if (result < 0.1f) result = 0.1f;
		return result;
	}
	case 4: return (player_init.baseXpGain + cardBaseMod.xp + (player_init.upgradeLevels[4] * multiplier[4])) * cardMultMod.xp;
	default: return 0.0f;
	}
}

float get_max_hp() {
	return calculate_max_stats(0);
}

void UpdateCurrentHpAfterCards(float oldMaxHp) {
	float newMaxHp = calculate_max_stats(0);

	// if max hp increased, give the player the difference
	if (newMaxHp > oldMaxHp) {
		player_init.current_hp += (newMaxHp - oldMaxHp);
	}

	// always clamp current hp to new max
	if (player_init.current_hp > newMaxHp) {
		player_init.current_hp = newMaxHp;
	}

	// never go below 1
	if (player_init.current_hp < 1.0f) {
		player_init.current_hp = 1.0f;
	}
}

//---- LEVEL UP LOGIC ----
void level_up(float xp_needed) {
	if (player_init.current_xp >= xp_needed) {
		player_init.current_xp -= xp_needed;
		player_init.player_level++;
		player_init.skill_point++;

		if (player_init.player_level < 26) {
			player_init.menu_open = true;
		}

		float max_hp = calculate_max_stats(0);
		player_init.current_hp += max_hp * 0.2f;
		if (player_init.current_hp > max_hp) {
			player_init.current_hp = max_hp;
		}
	}
}

// --- CURSOR COORDS CHECKING AND CLICKING ---
void handle_menu_input(float camX, float camY) {
	if (player_init.skill_point <= 0 && AEInputCheckTriggered(AEVK_LBUTTON)) {
		player_init.menu_open = false;
		// print player stats to console
	// calculates player stats
		f32 hp = calculate_max_stats(0);
		f32 dmg = calculate_max_stats(1);
		f32 speed = calculate_max_stats(2);
		f32 fire_rate = calculate_max_stats(3);
		f32 xp_mult = calculate_max_stats(4);

		std::cout
			<< "\n====================================\n"
			<< "HP: " << hp
			<< "\nDamage: " << dmg
			<< "\nSpeed: " << speed
			<< "\nFire Rate: " << fire_rate
			<< "\nXP Mult: " << xp_mult
			<< "\n====================================\n";
		return;
	}

	if (AEInputCheckTriggered(AEVK_LBUTTON)) {
		s32 screenX, screenY;
		AEInputGetCursorPosition(&screenX, &screenY);

		s32 winWidth = AEGfxGetWindowWidth();
		s32 winHeight = AEGfxGetWindowHeight();

		float mouseX = (float)screenX - (winWidth / 2.0f);
		float mouseY = (winHeight / 2.0f) - (float)screenY;

		float start_y = 200.0f;
		float spacing_y = 100.0f;
		float offset_middle = 260.0f;
		float squaresize = 60.0f;
		float middle_to_edge = squaresize / 2.0f;

		for (int i = 0; i < 5; ++i) {
			float buttonX = offset_middle;
			float buttonY = start_y - (i * spacing_y);

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

// --- CHEATS ---
void debug_inputs(float max_hp) {
	if (AEInputCheckTriggered(AEVK_R)) {
		player_init.current_hp += 100.0f;
		if (player_init.current_hp > max_hp)
			player_init.current_hp = max_hp;
	}

	if (AEInputCheckTriggered(AEVK_T)) {
		player_init.current_hp -= 50.0f;
		if (player_init.current_hp < 0.0f)
			player_init.current_hp = 0.0f;
	}

	if (AEInputCheckCurr(AEVK_E)) {
		float xp_multiplier = 1.0f + (player_init.upgradeLevels[4] * 5.0f);
		player_init.current_xp += 5.0f * xp_multiplier;
	}
}

// --- DRAW PLAYER HUD ---
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

// --- DRAWS THE UPGRADE ROWS ---
void draw_upgrade_rows(float camX, float camY) {
	float start_y = 200.0f, spacing_y = 100.0f, offset_middle = 260.0f;
	float segment_w = 70.0f, gap = 8.0f;
	float total_segment_dist = segment_w + gap;
	float start_x = -(total_segment_dist * 2.0f);

	for (int i = 0; i < 5; ++i) {
		float actual_y = start_y - (i * spacing_y);

		//row background
		drawmesh(pWhiteRectMesh, 0.0f + camX, actual_y + camY, 400.0f, 60.0f);

		//left squares
		drawicon(upgradeIcons[i], -offset_middle + camX, actual_y + camY, 55.0f);

		//right squares
		drawicon(upgradeConfirmIcon, offset_middle + camX, actual_y + camY, 55.0f);

		//upgrade boxes
		for (int j = 0; j < 5; ++j) {
			float current_x = start_x + (j * total_segment_dist);

			if (j < player_init.upgradeLevels[i]) {
				drawmesh(pWhiteRectMesh, current_x + camX, actual_y + camY, segment_w, 50.0f, 0.0f, 1.0f, 0.0f); // green
			}
			else {
				drawmesh(pWhiteRectMesh, current_x + camX, actual_y + camY, segment_w, 50.0f, 0.8f, 0.8f, 0.8f); // grey
			}
		}
	}
}


void UpdateDebug1() {
	float camX, camY;
	AEGfxGetCamPosition(&camX, &camY);

	float xp_needed = 100.0f + (powf((float)player_init.player_level, 1.5f) * 25.0f);
	float max_hp = calculate_max_stats(0);
	float max_dmg = calculate_max_stats(1);
	float max_speed = calculate_max_stats(2);
	float max_fire_rate = calculate_max_stats(3);

	level_up(xp_needed);

	if (player_init.menu_open) {
		handle_menu_input(camX, camY);
	}
	else {
		debug_inputs(max_hp);
	}
}


void DrawDebug1() {

	// --- GET CAM POS FOR UPGRADE MENU ---
	float camX, camY;
	AEGfxGetCamPosition(&camX, &camY);

	float hudX = camX + 0.0f;
	float hudY = camY - 410.0f;
	float max_width = 300.0f;
	float max_hp = calculate_max_stats(0);
	float xp_needed = 100.0f + (powf((float)player_init.player_level, 1.5f) * 25.0f);

	// --- DRAW ENEMY HEALTH BARS ---
	for (int i = 0; i < 50; i++) {
		if (enemyPool[i].alive) {
			draw_enemy_health_bar(enemyPool[i], camX, camY);
		}
	}

	// --- DRAW PLAYER HUD ---
	drawmesh(pBlackRectMesh, hudX, hudY, max_width + 10.0f, 52.0f);
	draw_hud_bar(pRedRectMesh, player_init.current_hp, max_hp, hudX, hudY, 7.0f, 25.0f, max_width);
	draw_hud_bar(pYellowRectMesh, player_init.current_xp, xp_needed, hudX, hudY, -15.0f, 10.0f, max_width);

	// --- DRAW FLOATING XP TEXT ---
	if (xpPopuptimer > 0.0f) {
		float dt = AEFrameRateControllerGetFrameTime();
		char xppopup[32];
		sprintf_s(xppopup, "+%.0fxp", xpPopupvalue);

		float alpha = xpPopuptimer / xpPopupduration;
		float upward_drift = (xpPopupduration - xpPopuptimer) * 0.07f;

		AEGfxPrint(boldPixels, xppopup, 0.22f, -0.93f + upward_drift, 0.35f, 1.0f, 1.0f, 1.0f, alpha);
		xpPopuptimer -= dt;
	}

	// --- PRINT HP, LEVEL, WAVE TEXT ---
	char hudHP[64], level[32], wave[32];

	sprintf_s(hudHP, "%.0f / %.0f", player_init.current_hp, max_hp);
	sprintf_s(level, "LEVEL %d", player_init.player_level);
	sprintf_s(wave, "WAVE %d", currentWave);

	float textScale = 0.35f;
	float textWidth, textHeight;
	AEGfxGetPrintSize(boldPixels, hudHP, textScale, &textWidth, &textHeight);

	// center on screen using fixed normalized coords
	AEGfxPrint(boldPixels, hudHP, -(textWidth / 2.0f), -0.91f, textScale, 1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxPrint(boldPixels, level, -0.18f, -0.84f, 0.4f, 0.0f, 0.0f, 0.0f, 1.0f);
	AEGfxPrint(boldPixels, wave, 0.06f, -0.84f, 0.4f, 0.0f, 0.0f, 0.0f, 1.0f);

	// --- DRAW UPGRADE MENU ---
	if (player_init.menu_open) {
		drawmesh(pBlackRectMesh, 0.0f + camX, 0.0f + camY, 900.0f, 650.0f);
		draw_upgrade_rows(camX, camY);

		// prints current player stats on upgrade menu
		const char* stats[] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP GAIN" };
		float rowStartY = 0.43f;
		float rowSpacing = 0.22f;
		for (int i = 0; i < 5; ++i) {
			char statText[64];
			if (i == 3) {
				float shotsPerSec = 1.0f / calculate_max_stats(3);
				sprintf_s(statText, "F-RATE: %.1f/s", shotsPerSec);
			}
			else {
				sprintf_s(statText, "%s: %.1f", stats[i], calculate_max_stats(i));
			}
			AEGfxPrint(boldPixels, statText, -0.55f, rowStartY - (i * rowSpacing), 0.28f, 1.0f, 1.0f, 1.0f, 1.0f);
		}

		if (player_init.skill_point > 0) {
			AEGfxPrint(boldPixels, "SPEND POINT TO CONTINUE!", -0.23f, -0.63f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			AEGfxPrint(boldPixels, "CLICK ANYWHERE TO CLOSE!", -0.23f, -0.63f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}


void FreeDebug1() {
	//AEGfxDestroyFont(boldPixels);

	// free color meshes
	AEGfxVertexList* meshes[] = { pBlackRectMesh, pWhiteRectMesh, pGreenRectMesh, pRedRectMesh, pYellowRectMesh };
	for (int i = 0; i < 5; ++i) {
		if (meshes[i]) AEGfxMeshFree(meshes[i]);
	}
	pBlackRectMesh = pWhiteRectMesh = pGreenRectMesh = pRedRectMesh = pYellowRectMesh = nullptr;

	// free icon mesh
	if (pIconMesh) { AEGfxMeshFree(pIconMesh); pIconMesh = nullptr; }

	// free stat textures
	for (int i = 0; i < 5; ++i) {
		if (upgradeIcons[i]) { AEGfxTextureUnload(upgradeIcons[i]); upgradeIcons[i] = nullptr; }
	}

	// free + icon
	if (upgradeConfirmIcon) { AEGfxTextureUnload(upgradeConfirmIcon); upgradeConfirmIcon = nullptr; }
}