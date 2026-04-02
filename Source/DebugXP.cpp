// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Debug XP.cpp
// Authors: [Men of Pause II]
// Brief:   Player HUD, XP/level-up logic, upgrade menu, and debug cheats.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// EXTERNS
// =============================================================================

extern std::array<Enemies, 50> enemyPool;  // Object pool for enemies
extern int   currentWave;                  // Current wave number
extern float bulletFireTimer;              // Cooldown timer for the shooting

// =============================================================================
// INITIAL PLAYER STATS
// =============================================================================

PlayerStats player_init = {
    // HP    DMG   SPEED  FIRERATE  XP
    300.0f, 15.0f, 300.0f,   0.5f,  1.0f,

    { 0, 0, 0, 0, 0 },  // initial upgrade levels

    0.0f, 0.0f, 0,      // initial XP stats (current_xp, xp_needed, player_level)

    0, false,           // initial skill_point, menu_open

    300.0f              // current_hp
};

// =============================================================================
// ANONYMOUS NAMESPACE — file-private data and helpers
// =============================================================================

namespace {

    // -------------------------------------------------------------------------
    // CONSTANTS
    // -------------------------------------------------------------------------

    // Stats
    constexpr int STAT_HP = 0;
    constexpr int STAT_DMG = 1;
    constexpr int STAT_SPEED = 2;
    constexpr int STAT_FIRERATE = 3;
    constexpr int STAT_XP = 4;
    constexpr int NUM_STATS = 5;

    // Pool / upgrade limits
    constexpr int   ENEMY_POOL_SIZE = 50;
    constexpr int   MAX_UPGRADE_LEVEL = 5;
    constexpr int   MENU_LEVEL_CAP = 26;  // menu stops opening at this level

    // Gameplay
    constexpr float MIN_FIRE_RATE = 0.1f;
    constexpr float MIN_CURRENT_HP = 1.0f;
    constexpr float LEVEL_UP_HEAL_PERCENT = 0.15f;

    // XP formula:  xp_needed = XP_BASE + level^XP_EXPONENT * XP_MULTIPLIER
    constexpr float XP_BASE = 100.0f;
    constexpr float XP_MULTIPLIER = 25.0f;
    constexpr float XP_EXPONENT = 1.5f;

    // UV mesh vertex colour (white, full alpha)
    constexpr uint32_t UV_MESH_COLOR = 0xFFFFFFFF;

    // ---- HUD bars ----
    constexpr float HUD_OFFSET_Y = -410.0f;
    constexpr float HUD_MAX_WIDTH = 300.0f;
    constexpr float HUD_BORDER = 10.0f;
    constexpr float HUD_OUTER_HEIGHT = 52.0f;
    constexpr float HUD_HP_BAR_OFFSET_Y = 7.0f;
    constexpr float HUD_HP_BAR_HEIGHT = 25.0f;
    constexpr float HUD_XP_BAR_OFFSET_Y = -15.0f;
    constexpr float HUD_XP_BAR_HEIGHT = 10.0f;

    // ---- Shoot-cooldown bar ----
    constexpr float SHOOT_BAR_WIDTH = 100.0f;
    constexpr float SHOOT_BAR_HEIGHT = 15.0f;
    constexpr float SHOOT_BAR_OFFSET_X = -250.0f;
    constexpr float SHOOT_BAR_OFFSET_Y = -410.0f;
    constexpr float SHOOT_BAR_BORDER_X = 6.0f;
    constexpr float SHOOT_BAR_BORDER_Y = 4.0f;

    // ---- Enemy health bar ----
    constexpr float ENEMY_BAR_HEIGHT = 6.0f;
    constexpr float ENEMY_BAR_BORDER = 2.0f;   // outline padding
    constexpr float ENEMY_BAR_Y_SCALE = 0.9f;   // vertical offset relative to enemy scale

    // ---- Upgrade menu layout ----
    constexpr float MENU_BG_WIDTH = 900.0f;
    constexpr float MENU_BG_HEIGHT = 650.0f;
    constexpr float MENU_ROW_START_Y = 200.0f;
    constexpr float MENU_ROW_SPACING_Y = 100.0f;
    constexpr float MENU_OFFSET_MIDDLE = 260.0f;  // X distance from centre to icon columns
    constexpr float MENU_ROW_BG_WIDTH = 400.0f;
    constexpr float MENU_ROW_BG_HEIGHT = 60.0f;
    constexpr float MENU_ICON_SIZE = 55.0f;
    constexpr float MENU_CLICK_HALF = 30.0f;   // half-size of icon hit region

    // ---- Upgrade segment boxes ----
    constexpr float SEGMENT_WIDTH = 70.0f;
    constexpr float SEGMENT_GAP = 8.0f;
    constexpr float SEGMENT_HEIGHT = 50.0f;

    // ---- HUD text ----
    constexpr float TEXT_SCALE_HUD = 0.35f;
    constexpr float TEXT_SCALE_LABEL = 0.4f;
    constexpr float TEXT_HP_Y = -0.91f;
    constexpr float TEXT_LEVEL_X = -0.18f;
    constexpr float TEXT_LEVEL_Y = -0.84f;
    constexpr float TEXT_WAVE_X = 0.06f;
    constexpr float TEXT_WAVE_Y = -0.84f;
    constexpr float TEXT_PROMPT_Y = -0.63f;

    // ---- Upgrade menu stat text ----
    constexpr float STATS_TEXT_X = -0.55f;
    constexpr float STATS_TEXT_START_Y = 0.43f;
    constexpr float STATS_TEXT_SPACING_Y = 0.22f;
    constexpr float STATS_TEXT_SCALE = 0.28f;

    // ---- Floating XP popup ----
    constexpr float XP_POPUP_DURATION = 0.7f;
    constexpr float XP_POPUP_DRIFT_SCALE = 0.07f;
    constexpr float XP_POPUP_X = 0.22f;
    constexpr float XP_POPUP_BASE_Y = -0.93f;
    constexpr float XP_POPUP_SCALE = 0.35f;

    // ---- Debug cheat overlay ----
    constexpr int   DEBUG_CHEAT_COUNT = 8;
    constexpr float DEBUG_TEXT_START_X = 0.73f;
    constexpr float DEBUG_TEXT_START_Y = -0.95f;
    constexpr float DEBUG_TEXT_SCALE = 0.3f;
    constexpr float DEBUG_LINE_GAP = 0.045f;
    constexpr float DEBUG_HEADER_SCALE = 0.28f;

    // -------------------------------------------------------------------------
    // MESHES & TEXTURES
    // -------------------------------------------------------------------------

    // Solid-colour meshes
    AEGfxVertexList* pMeshBlack = nullptr;
    AEGfxVertexList* pMeshWhite = nullptr;
    AEGfxVertexList* pMeshGreen = nullptr;
    AEGfxVertexList* pMeshRed = nullptr;
    AEGfxVertexList* pMeshYellow = nullptr;

    // UV-mapped mesh shared by all icons
    AEGfxVertexList* pMeshIcon = nullptr;

    // Per-stat upgrade icons
    AEGfxTexture* upgradeIcons[NUM_STATS] = { nullptr, nullptr, nullptr, nullptr, nullptr };
    const char* kIconPaths[NUM_STATS] = {
        "Assets/hp.png",
        "Assets/dmg.png",
        "Assets/speed.png",
        "Assets/firerate.png",
        "Assets/xp.png"
    };

    // Confirm ( + ) icon
    AEGfxTexture* pIconConfirm = nullptr;

    // -------------------------------------------------------------------------
    // STAT TABLES  (*** CHANGE MULTIPLIERS HERE ***)
    // -------------------------------------------------------------------------

    float kStatMultiplier[NUM_STATS] = { 25.0f, 4.0f, 30.0f,  0.033f,  0.5f };

    const char* kStatNames[NUM_STATS] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP MULT" };

	// Base stats (for display and upgrade calculations)
    float kBaseStats[NUM_STATS] = {
        player_init.baseHp,
        player_init.baseDmg,
        player_init.baseSpeed,
        player_init.baseFireRate,
        player_init.baseXpGain
    };

    // -------------------------------------------------------------------------
    // FLOATING XP POPUP STATE
    // -------------------------------------------------------------------------

    float xpPopupTimer = 0.0f;
    float xpPopupDuration = XP_POPUP_DURATION;
    float xpPopupValue = 0.0f;

    // -------------------------------------------------------------------------
    // MESH HELPERS
    // -------------------------------------------------------------------------

    // Build a solid-colour unit quad
    AEGfxVertexList* CreateColorMesh(uint32_t color) {
        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, color, 0.0f, 0.0f,
            0.5f, -0.5f, color, 0.0f, 0.0f,
            -0.5f, 0.5f, color, 0.0f, 0.0f);
        AEGfxTriAdd(0.5f, -0.5f, color, 0.0f, 0.0f,
            0.5f, 0.5f, color, 0.0f, 0.0f,
            -0.5f, 0.5f, color, 0.0f, 0.0f);
        return AEGfxMeshEnd();
    }

    // Draw a solid-colour mesh at world position (x, y)
    void DrawColorMesh(AEGfxVertexList* mesh, float x, float y, float scaleX, float scaleY,
        float r = 1.0f, float g = 1.0f, float b = 1.0f) {
        if (!mesh) return;
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEMtx33 scale, trans, final;
        AEMtx33Scale(&scale, scaleX, scaleY);
        AEMtx33Trans(&trans, x, y);
        AEMtx33Concat(&final, &trans, &scale);
        AEGfxSetTransform(final.m);
        AEGfxSetColorToMultiply(r, g, b, 1.0f);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
    }

    // Draw a textured icon at world position (x, y)
    void DrawIconMesh(AEGfxTexture* tex, float x, float y, float size) {
        if (!tex || !pMeshIcon) return;
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
        AEGfxMeshDraw(pMeshIcon, AE_GFX_MDM_TRIANGLES);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);  // restore colour mode
    }

    // -------------------------------------------------------------------------
    // DRAW HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Draw a health bar above an enemy (only when damaged)
    void DrawEnemyHealthBar(Enemies& enemy, float camX, float camY) {
        if (!enemy.alive || enemy.hp >= enemy.maxhp || enemy.hp <= 0) return;

		// Bar width scales with enemy size, but height is constant
        float barWidth = enemy.scale;
        float yOffset = -(enemy.scale * ENEMY_BAR_Y_SCALE);

		// Calculate health percentage (clamped between 0 and 1)
        float perc = (float)enemy.hp / (float)enemy.maxhp;
        if (perc < 0.0f) perc = 0.0f;

        // Black outline
        DrawColorMesh(pMeshBlack,
            enemy.pos.x, enemy.pos.y + yOffset,
            barWidth + ENEMY_BAR_BORDER + 2.0f, ENEMY_BAR_HEIGHT + ENEMY_BAR_BORDER);

        // Red fill
        float fillW = barWidth * perc;
        float shiftX = (barWidth - fillW) / 2.0f;
        DrawColorMesh(pMeshRed,
            enemy.pos.x - shiftX, enemy.pos.y + yOffset,
            fillW, ENEMY_BAR_HEIGHT,
            1.0f, 0.0f, 0.0f);
    }

    // ~ Brief: Draw the shoot-cooldown bar (static HUD position)
    void DrawShootCooldownBar(float camX, float camY, float timeSinceLast, float fireRate) {
		// Calculate cooldown progress (clamped between 0 and 1)
        float progress = timeSinceLast / fireRate;
        if (progress > 1.0f) progress = 1.0f;
        if (progress < 0.0f) progress = 0.0f;

        float finalX = camX + SHOOT_BAR_OFFSET_X;
        float finalY = camY + SHOOT_BAR_OFFSET_Y;

        // Black outline
        DrawColorMesh(pMeshBlack, finalX, finalY,
            SHOOT_BAR_WIDTH + SHOOT_BAR_BORDER_X,
            SHOOT_BAR_HEIGHT + SHOOT_BAR_BORDER_Y);

        float fillW = SHOOT_BAR_WIDTH * progress;
        float shiftX = (SHOOT_BAR_WIDTH - fillW) / 2.0f;

		// If fully cooled down, show a bright yellow bar. Otherwise, show the red cooldown fill
        if (progress >= 1.0f)
            DrawColorMesh(pMeshYellow, finalX, finalY, SHOOT_BAR_WIDTH, SHOOT_BAR_HEIGHT);
        else
            DrawColorMesh(pMeshRed, finalX - shiftX, finalY, fillW, SHOOT_BAR_HEIGHT);
    }

    // ~ Brief: Draw a single proportional bar (HP or XP) anchored to the HUD
    void DrawHudBar(AEGfxVertexList* mesh, float current, float maxVal,
        float anchorX, float anchorY, float relOffsetY,
        float barHeight, float maxWidth) {
        float perc = current / maxVal;
        if (perc > 1.0f) perc = 1.0f;
        if (perc < 0.0f) perc = 0.0f;

        float fillW = maxWidth * perc;
        float shiftRight = (maxWidth - fillW) / 2.0f;

        DrawColorMesh(mesh,
            anchorX - shiftRight, anchorY + relOffsetY,
            fillW, barHeight);
    }

    // ~ Brief: Draw the five upgrade rows inside the upgrade menu
    void DrawUpgradeRows(float camX, float camY) {
		// Pre-calculate horizontal positions for the upgrade segments to simplify the loop
        constexpr float kSegmentStep = SEGMENT_WIDTH + SEGMENT_GAP;
        constexpr float kRowStartX = -(kSegmentStep * 2.0f);

        for (int i = 0; i < NUM_STATS; ++i) {
            float rowY = MENU_ROW_START_Y - (i * MENU_ROW_SPACING_Y);

            // Row background
            DrawColorMesh(pMeshWhite, camX, rowY + camY, MENU_ROW_BG_WIDTH, MENU_ROW_BG_HEIGHT);

            // Left stat icon
            DrawIconMesh(upgradeIcons[i], -MENU_OFFSET_MIDDLE + camX, rowY + camY, MENU_ICON_SIZE);

            // Right confirm ( + ) icon
            DrawIconMesh(pIconConfirm, MENU_OFFSET_MIDDLE + camX, rowY + camY, MENU_ICON_SIZE);

            // Upgrade segment boxes
            for (int j = 0; j < MAX_UPGRADE_LEVEL; ++j) {
                float segX = kRowStartX + (j * kSegmentStep);
                if (j < player_init.upgradeLevels[i])
					// Draw filled green segments for purchased upgrades, grey for locked segments
                    DrawColorMesh(pMeshWhite, segX + camX, rowY + camY, SEGMENT_WIDTH, SEGMENT_HEIGHT, 0.0f, 1.0f, 0.0f);  // green = upgraded
                else
                    DrawColorMesh(pMeshWhite, segX + camX, rowY + camY, SEGMENT_WIDTH, SEGMENT_HEIGHT, 0.8f, 0.8f, 0.8f);  // grey = locked
            }
        }
    }

}  // namespace

// =============================================================================
// STAT CALCULATION
// =============================================================================

// Returns the current effective value for stat index i, accounting for
// base stats, card modifiers, and skill-point upgrades.
//
// FORMULA:  (base + cardBaseMod + upgradeLevel * multiplier) * cardMultMod
// HP is special: upgrade bonus is applied after card scaling (flat increase).

// ~ Brief: Calculate the player's current effective stat value for a given stat index (HP, DMG, SPEED, FIRERATE, XP)
float calculate_max_stats(int i) {
    switch (i) {
    case STAT_HP: return ((player_init.baseHp + cardBaseMod.hp) * cardMultMod.hp) + (player_init.upgradeLevels[0] * kStatMultiplier[STAT_HP]);
    case STAT_DMG:
        return (player_init.baseDmg + cardBaseMod.dmg
            + (player_init.upgradeLevels[STAT_DMG] * kStatMultiplier[STAT_DMG])) * cardMultMod.dmg;
    case STAT_SPEED:
        return (player_init.baseSpeed + cardBaseMod.moveSpeed
            + (player_init.upgradeLevels[STAT_SPEED] * kStatMultiplier[STAT_SPEED])) * cardMultMod.moveSpeed;
    case STAT_FIRERATE: {
        float result = (player_init.baseFireRate - cardBaseMod.fireRate
            - (player_init.upgradeLevels[STAT_FIRERATE] * kStatMultiplier[STAT_FIRERATE])) * cardMultMod.fireRate;
        if (result < MIN_FIRE_RATE) result = MIN_FIRE_RATE;
        return result;
    }
    case STAT_XP:
        return (player_init.baseXpGain + cardBaseMod.xp
            + (player_init.upgradeLevels[STAT_XP] * kStatMultiplier[STAT_XP])) * cardMultMod.xp;
    default:
        return 0.0f;
    }
}

// ~ Brief: After HP upgrades, adjust current HP to account for the new max HP. If max HP increased, 
//          give the player the difference. Always clamp current HP to the new max, and never let it drop below 1.
void UpdateCurrentHpAfterCards(float oldMaxHp) {
    float newMaxHp = calculate_max_stats(STAT_HP);

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

float get_max_hp() {
    return calculate_max_stats(STAT_HP);
}

// =============================================================================
// GAME STATE
// =============================================================================

// ~ Brief: Trigger a floating XP popup with the given XP amount. 
//          The popup will automatically drift upwards and fade out over time.
void TriggerXpPopup(float xpAmount) {
    xpPopupValue = xpAmount;
    xpPopupTimer = xpPopupDuration;
}

// ~ Brief: Reset all game state to initial values for a new game or after player death.
void reset_game() {
    // Tutorial
    tutorialOn = true;

    // Wave
    currentWave = 1;

    // Boss
    currentboss.alive = false;

    // Minions
    for (auto& minion : minionPool) {
        minion.alive = false;
        minion.hp = 0;
    }

    // Wave spawning
    pendingBudget = 0.0f;
    spawnTimer = 0.0f;
    totalWaveBudget = 0.0f;

    // Enemies
    for (auto& enemy : enemyPool) {
        enemy.alive = false;
        enemy.hp = 0;
    }

    // Bullets
    for (auto& bullet : bulletList)      bullet.isActive = false;
    for (auto& enBullet : enemyBulletList) enBullet.isActive = false;

    // Player
    player_init.player_level = 0;
    player_init.current_xp = 0;
    player_init.skill_point = 0;
    player_init.menu_open = false;
    for (int i = 0; i < NUM_STATS; ++i)
        player_init.upgradeLevels[i] = 0;
    player_init.current_hp = player_init.baseHp;
}

// =============================================================================
// LEVEL-UP LOGIC
// =============================================================================

// ~ Brief: Check if the player has enough XP to level up. If so, consume the XP, 
//          increase the player level and skill points, and partially heal the player.
void level_up(float xpNeeded) {
    if (player_init.current_xp < xpNeeded) return;

	// Consume XP and increase level/skill points
    player_init.current_xp -= xpNeeded;
    player_init.player_level++;
    player_init.skill_point++;

    // Open upgrade menu on level up
    if (player_init.player_level < MENU_LEVEL_CAP)
        player_init.menu_open = true;

    // Partial heal on level-up
    float maxHp = calculate_max_stats(STAT_HP);
    player_init.current_hp += maxHp * LEVEL_UP_HEAL_PERCENT;
    if (player_init.current_hp > maxHp)
        player_init.current_hp = maxHp;
}

// =============================================================================
// INPUT HANDLING
// =============================================================================

// ~ Brief: Handle mouse clicks on the upgrade menu
void handle_menu_input(float camX, float camY) {
    if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

    // No skill points left — close menu on any click
    if (player_init.skill_point <= 0) {
        player_init.menu_open = false;
        return;
    }

	// Get mouse position in screen coordinates, then convert to world coordinates relative to the camera
    s32 screenX, screenY;
    AEInputGetCursorPosition(&screenX, &screenY);

    float mouseX = (float)screenX - (AEGfxGetWindowWidth() / 2.0f);
    float mouseY = (AEGfxGetWindowHeight() / 2.0f) - (float)screenY;

	// Check each upgrade row for a click on the confirm ( + ) icon, and if valid, 
    // apply the upgrade and consume a skill point
    for (int i = 0; i < NUM_STATS; ++i) {
        float btnX = MENU_OFFSET_MIDDLE;
        float btnY = MENU_ROW_START_Y - (i * MENU_ROW_SPACING_Y);

        bool hovered = (mouseX >= btnX - MENU_CLICK_HALF && mouseX <= btnX + MENU_CLICK_HALF &&
            mouseY >= btnY - MENU_CLICK_HALF && mouseY <= btnY + MENU_CLICK_HALF);

		// If they clicked the confirm icon for this stat, and it's not already maxed out, apply the upgrade
        if (hovered && player_init.upgradeLevels[i] < MAX_UPGRADE_LEVEL) {
            float oldMax = calculate_max_stats(STAT_HP);
            player_init.upgradeLevels[i]++;
            player_init.skill_point--;

            if (i == STAT_HP) UpdateCurrentHpAfterCards(oldMax);

            return;
        }
    }
}

// ~ Brief: Debug / cheat key inputs
void debug_inputs(float xpNeeded) {
    if (!cheatsOn) return;

    float maxHp = calculate_max_stats(STAT_HP);

    if (AEInputCheckTriggered(AEVK_1)) player_init.current_hp = maxHp;    // full heal
    if (AEInputCheckTriggered(AEVK_2)) player_init.current_hp = 0.0f;     // instant death
    if (AEInputCheckTriggered(AEVK_3)) player_init.current_xp = xpNeeded; // instant level-up
}

// =============================================================================
// LOAD / FREE
// =============================================================================

/// ~ Brief: Load meshes and textures needed for the HUD, upgrade menu, and debug overlay.
void LoadDebug1() {
    // Solid-colour meshes
    pMeshBlack = CreateColorMesh(0xFF000000);
    pMeshWhite = CreateColorMesh(0xFFFFFFFF);
    pMeshGreen = CreateColorMesh(0xFF00FF00);
    pMeshRed = CreateColorMesh(0xFFFF0000);
    pMeshYellow = CreateColorMesh(0xFFFFFF00);

    // UV-mapped mesh for icons
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, UV_MESH_COLOR, 0.0f, 1.0f,
        0.5f, -0.5f, UV_MESH_COLOR, 1.0f, 1.0f,
        -0.5f, 0.5f, UV_MESH_COLOR, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, UV_MESH_COLOR, 1.0f, 1.0f,
        0.5f, 0.5f, UV_MESH_COLOR, 1.0f, 0.0f,
        -0.5f, 0.5f, UV_MESH_COLOR, 0.0f, 0.0f);
    pMeshIcon = AEGfxMeshEnd();

    // Stat icons
    for (int i = 0; i < NUM_STATS; ++i)
        upgradeIcons[i] = AEGfxTextureLoad(kIconPaths[i]);

    // Confirm icon
    pIconConfirm = AEGfxTextureLoad("Assets/plus.png");
}

// ~ Brief: Free meshes and textures used by the HUD, upgrade menu, and debug overlay to prevent memory leaks.
void FreeDebug1() {
    // Solid-colour meshes
    AEGfxVertexList* meshes[] = { pMeshBlack, pMeshWhite, pMeshGreen, pMeshRed, pMeshYellow };
    for (int i = 0; i < 5; ++i) {
        if (meshes[i]) AEGfxMeshFree(meshes[i]);
    }
    pMeshBlack = pMeshWhite = pMeshGreen = pMeshRed = pMeshYellow = nullptr;

    // Icon mesh
    if (pMeshIcon) { AEGfxMeshFree(pMeshIcon); pMeshIcon = nullptr; }

    // Stat textures
    for (int i = 0; i < NUM_STATS; ++i) {
        if (upgradeIcons[i]) { AEGfxTextureUnload(upgradeIcons[i]); upgradeIcons[i] = nullptr; }
    }

    // Confirm icon texture
    if (pIconConfirm) { AEGfxTextureUnload(pIconConfirm); pIconConfirm = nullptr; }
}

// =============================================================================
// UPDATE
// =============================================================================

// ~ Brief: Handle player input for the upgrade menu and debug cheats, and check for 
//          level-up conditions to update player stats accordingly.
void UpdateDebug1() {
	// Get camera position for converting screen coordinates to world coordinates in input handling
    float camX, camY;
    AEGfxGetCamPosition(&camX, &camY);

	// Calculate XP needed for next level using the formula: xp_needed = XP_BASE + level^XP_EXPONENT * XP_MULTIPLIER
    float xpNeeded = XP_BASE + (powf((float)player_init.player_level, XP_EXPONENT) * XP_MULTIPLIER);

    level_up(xpNeeded);

	// If the upgrade menu is open, handle menu input. Otherwise, check for debug cheat inputs.
    if (player_init.menu_open)
        handle_menu_input(camX, camY);
    else
        debug_inputs(xpNeeded);
}

// =============================================================================
// DRAW
// =============================================================================

// ~ Brief: Draw the player HUD (HP and XP bars), shoot-cooldown bar, enemy health bars, 
//          floating XP popups, upgrade menu, and debug cheat overlay.
void DrawDebug1() {
    float camX, camY;
    AEGfxGetCamPosition(&camX, &camY);
    float hudX = camX;
    float hudY = camY + HUD_OFFSET_Y;
    float maxHp = calculate_max_stats(STAT_HP);
    float xpNeeded = XP_BASE + (powf((float)player_init.player_level, XP_EXPONENT) * XP_MULTIPLIER);

    // -- Shoot-cooldown bar --
    DrawShootCooldownBar(camX, camY, bulletFireTimer, calculate_max_stats(STAT_FIRERATE));

    // -- Enemy health bars --
    for (int i = 0; i < ENEMY_POOL_SIZE; ++i) {
        if (enemyPool[i].alive)
            DrawEnemyHealthBar(enemyPool[i], camX, camY);
    }

    // -- Player HUD (HP + XP bars) -- 
    DrawColorMesh(pMeshBlack, hudX, hudY, HUD_MAX_WIDTH + HUD_BORDER, HUD_OUTER_HEIGHT);
    DrawHudBar(pMeshRed, player_init.current_hp, maxHp, hudX, hudY, HUD_HP_BAR_OFFSET_Y, HUD_HP_BAR_HEIGHT, HUD_MAX_WIDTH);
    DrawHudBar(pMeshYellow, player_init.current_xp, xpNeeded, hudX, hudY, HUD_XP_BAR_OFFSET_Y, HUD_XP_BAR_HEIGHT, HUD_MAX_WIDTH);

    // -- Floating XP popup --
    if (xpPopupTimer > 0.0f) {
        float dt = static_cast<f32>(AEFrameRateControllerGetFrameTime());
        char  buf[32];
        sprintf_s(buf, "+%.0fxp", xpPopupValue);

        float alpha = xpPopupTimer / xpPopupDuration;
        float upwardDrift = (xpPopupDuration - xpPopupTimer) * XP_POPUP_DRIFT_SCALE;

        AEGfxPrint(boldPixels, buf,
            XP_POPUP_X, XP_POPUP_BASE_Y + upwardDrift,
            XP_POPUP_SCALE, 1.0f, 1.0f, 1.0f, alpha);
        xpPopupTimer -= dt;
    }

    // -- HUD text (HP / Level / Wave) --
    char hudHpText[64], levelText[32], waveText[32];
    float displayHp = (player_init.current_hp < 0.0f) ? 0.0f : player_init.current_hp;
    sprintf_s(hudHpText, "%.0f / %.0f", displayHp, maxHp);
    sprintf_s(levelText, "LEVEL %d", player_init.player_level);
    sprintf_s(waveText, "WAVE %d", currentWave);

    float textW, textH;
    AEGfxGetPrintSize(boldPixels, hudHpText, TEXT_SCALE_HUD, &textW, &textH);

    AEGfxPrint(boldPixels, hudHpText, -(textW / 2.0f), TEXT_HP_Y, TEXT_SCALE_HUD, 1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxPrint(boldPixels, levelText, TEXT_LEVEL_X, TEXT_LEVEL_Y, TEXT_SCALE_LABEL, 0.0f, 0.0f, 0.0f, 1.0f);
    AEGfxPrint(boldPixels, waveText, TEXT_WAVE_X, TEXT_WAVE_Y, TEXT_SCALE_LABEL, 0.0f, 0.0f, 0.0f, 1.0f);

    // -- Upgrade menu --
    if (player_init.menu_open) {
        DrawColorMesh(pMeshBlack, camX, camY, MENU_BG_WIDTH, MENU_BG_HEIGHT);
        DrawUpgradeRows(camX, camY);

        // Stat values (left side of menu)
        const char* kMenuStatNames[NUM_STATS] = { "HP", "DMG", "SPEED", "FIRE RATE", "XP MULT" };
        for (int i = 0; i < NUM_STATS; ++i) {
            char statBuf[64];
            if (i == STAT_FIRERATE)
                sprintf_s(statBuf, "F-RATE: %.1f/s", 1.0f / calculate_max_stats(STAT_FIRERATE));
            else
                sprintf_s(statBuf, "%s: %.1f", kMenuStatNames[i], calculate_max_stats(i));

            AEGfxPrint(boldPixels, statBuf,
                STATS_TEXT_X, STATS_TEXT_START_Y - (i * STATS_TEXT_SPACING_Y),
                STATS_TEXT_SCALE, 1.0f, 1.0f, 1.0f, 1.0f);
        }

		// Prompt text
        const char* prompt = (player_init.skill_point > 0) ? "CLICK + TO SPEND SKILL POINT!" : "CLICK ANYWHERE TO CLOSE!";
        float       promptX = (player_init.skill_point > 0) ? -0.28f : -0.23f;
        AEGfxPrint(boldPixels, prompt, promptX, TEXT_PROMPT_Y, TEXT_SCALE_LABEL, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    // -- Debug cheat overlay --
    if (cheatsOn) {
        const char* kCheatLabels[DEBUG_CHEAT_COUNT] = {
            "1: FULL HEAL",
            "2: DIE",
            "3: LEVEL UP",
            "4: DUAL CANNON",
            "5: BIG CANNON",
            "6: 180 SHOT",
            "7: ORBIT SHIELD",
            "8: SKIP WAVE"
        };

        AEGfxPrint(boldPixels, "[ CHEATS MENU ]",
            DEBUG_TEXT_START_X, DEBUG_TEXT_START_Y + (DEBUG_LINE_GAP * 8),
            DEBUG_HEADER_SCALE, 0.0f, 0.0f, 0.0f, 1.0f);

        for (int i = 0; i < DEBUG_CHEAT_COUNT; ++i) {
            AEGfxPrint(boldPixels, kCheatLabels[i],
                DEBUG_TEXT_START_X, DEBUG_TEXT_START_Y + (DEBUG_LINE_GAP * (7 - i)),
                DEBUG_TEXT_SCALE, 0.0f, 0.0f, 0.0f, 1.0f);
        }
    }
}