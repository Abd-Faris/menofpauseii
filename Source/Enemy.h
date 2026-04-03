// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Enemy.h
// Authors: [Men of Pause II]
// Brief:   Declares the enemy object pool, bullet list, textures, and all
//          functions for spawning, updating, and drawing enemies.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// EXTERN GLOBALS
// =============================================================================

// ~ Brief: Object pool for all active enemies. Fixed size set by MAX_ENEMIES_COUNT.
extern std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;

// ~ Brief: Accumulates elapsed time between enemy spawns during wave management.
extern f64 enemySpawnTimer;

// ~ Brief: Pool of bullets fired by all enemy types. Shared with the boss system.
extern BulletObj enemyBulletList[GameConfig::MAX_BULLETS_COUNT];

// ~ Brief: Texture and mesh used to render all enemy-fired projectiles.
extern AEGfxTexture* pEnemyBulletTex;
extern AEGfxVertexList* pEnemyBulletMesh;

// =============================================================================
// FUNCTIONS
// =============================================================================

// ~ Brief: Load all enemy and bullet textures and build their shared UV-mapped meshes.
void LoadEnemies();

// ~ Brief: Reset an enemy slot to its default inactive state and move it off-screen.
void ResetEnemy(Enemies* enemyToReset);

// ~ Brief: Spawn a passive box enemy (small or big) at a safe world position.
//          HP scales with the current wave multiplier.
void SpawnOneEnemy(bool isBigEnemy, shape player);

// ~ Brief: Spawn a kamikaze (ATTACK) enemy that charges directly at the player
//          once within detection range. HP scales with the current wave multiplier.
void SpawnAttackEnemy(shape player);

// ~ Brief: Spawn a shooter enemy that maintains engagement distance and fires
//          projectiles at the player on a cooldown. HP scales with wave multiplier.
void SpawnShooterEnemy(shape player);

// ~ Brief: Update movement, rotation, separation, and shooting for all active enemies.
//          Awards XP and triggers explosions when an enemy's HP reaches zero.
void updateEnemyPhysics(shape& player, float deltaTime);

// ~ Brief: Move all active enemy bullets along their direction each frame.
//          Deactivates bullets that collide with world geometry.
void updateEnemyBullets(float deltaTime);

// ~ Brief: Draw a colored arrow at the screen edge pointing toward each off-screen
//          enemy, minion, and boss. Color-coded by type: purple, blue, red, orange.
void DrawEnemyIndicators(shape& player, AEGfxVertexList* MeshTriangle);

// ~ Brief: Unload all enemy and bullet textures and free their meshes.
void FreeEnemies();