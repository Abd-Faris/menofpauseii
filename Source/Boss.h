// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Boss.h
// Authors: [Men of Pause II]
// Brief:   Declares the boss and minion systems, including state management,
//          attack patterns (rings, spirals, lasers), and rendering logic.
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h"

// =============================================================================
// EXTERN GLOBALS
// =============================================================================

// ~ Brief: Textures used for the boss entity and its summoned minions.
extern AEGfxTexture* pBossTex;
extern AEGfxTexture* pMinionTex;
extern AEGfxTexture* pBossArmTex;
extern AEGfxTexture* pLaserTex;

// ~ Brief: Shared mesh for rendering the boss sprite.
extern AEGfxVertexList* pBossMesh;

// ~ Brief: The active boss instance currently present in the game world.
extern Boss currentboss;

// ~ Brief: Maximum number of minions allowed on screen at once.
constexpr int MAX_MINIONS_COUNT = 30;

// ~ Brief: Object pool for managing active minions summoned by bosses.
extern std::array<Enemies, MAX_MINIONS_COUNT> minionPool;

// =============================================================================
// FUNCTIONS
// =============================================================================

// ~ Brief: Load boss-related textures and initialize required meshes.
void LoadBoss();

// ~ Brief: Initialize a boss of a specific type at a spawn location.
void SpawnBoss(BossType type, shape& player);

// ~ Brief: Update boss movement, AI states, and internal timers.
void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime);

// ~ Brief: Render the boss entity and its associated components.
void DrawBoss(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle);

// ~ Brief: Execute an attack pattern that fires a ring of bullets from the boss.
void BossShootRing(Boss& boss);

// ~ Brief: Handle collision detection between the boss, the player, and active shields.
void BossCollision(Boss& boss, shape& player, bool orbitActive, float orbitPosX, float orbitPosY);

// ~ Brief: Render the boss's health bar and name UI relative to the player's view.
void DrawBossHP(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle, shape& player);

// ~ Brief: Logic for the boss to spawn a minion from the minionPool.
void BossSpawnMinion(Boss& boss);

// ~ Brief: Update movement and behaviors for all active minions in the pool.
void updateMinionPhysics(shape& player, float deltaTime);

// ~ Brief: Pattern for Boss 3: Fires bullets in a continuous spiral formation.
void Boss3Spiral(Boss& boss, float deltaTime);

// ~ Brief: Pattern for Boss 3: Fires a targeted shot directly at the player.
void Boss3AimedShot(Boss& boss, shape& player);

// ~ Brief: Release all boss-related textures and meshes from memory.
void FreeBoss();

// ~ Brief: Calculate the world position of the boss's left or right gun barrels.
AEVec2 GetGunPosition(Boss& boss, bool leftGun);

// ~ Brief: Pattern for Boss 4: Manages dual-gun firing logic and cooldowns.
void Boss4ShootGuns(Boss& boss, shape& player, float deltaTime);

// ~ Brief: Render the telegraph and hitbox for the boss's laser attack.
void DrawBossLaser(Boss& boss, AEGfxVertexList* MeshRect);