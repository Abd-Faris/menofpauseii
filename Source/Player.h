// -----------------------------Gloomy's Revenge---------------------------- //
// File:	Player.h
// Authors:	[Men of Pause II]
// Brief:	Header for player movement, tank rendering, and combat systems.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// EXTERNS & PROJECTILE POOLS
// =============================================================================

extern BulletObj bulletList[GameConfig::MAX_BULLETS_COUNT];
extern float bulletFireTimer;
extern bool bigcannon;              // Toggle for the heavy weapon upgrade
extern AEGfxTexture* pBulletTex;
extern AEGfxVertexList* pBulletMesh;

// =============================================================================
// LIFECYCLE & COMBAT FUNCTIONS
// =============================================================================

// ~ Brief: Loads bullet textures and initializes the projectile vertex mesh.
void LoadBullets();

// ~ Brief: Fires a projectile from the player's position based on current orientation.
void ShootBullet(shape& player, float deltaTime);

// ~ Brief: Updates the physics and collision checks for all active bullets in the pool.
void updateBullets(shape& player, float deltaTime);

// ~ Brief: Cleans up bullet-related resources and active effects.
void FreeBullets();

// =============================================================================
// PLAYER MOVEMENT & ROTATION
// =============================================================================

// ~ Brief: Handles keyboard input to translate the player's world position.
void movePlayer(shape& player, float deltaTime);

// ~ Brief: Logic for rotating the tank body and turret toward the mouse cursor.
void rotatePlayer(shape& player);

// =============================================================================
// RENDERING & SPECIAL WEAPONS
// =============================================================================

// ~ Brief: Main draw call for the standard player tank.
void drawBigTank(shape& player);

// ~ Brief: Specialized draw call for the "Big Cannon" upgrade variant.
void drawBigCannon(shape& player);

// ~ Brief: Renders a specialized dual-barrel setup for the back of the tank.
void DualBack(shape& player);

// ~ Brief: Helper to render multiple barrels with specific spacing and offsets.
void DrawMultiBarrels(int count, float gap, float pivotOffset, float tankRot, float tankX, float tankY, float barrelWidth, float barrelLength, AEGfxVertexList* MeshRect);

// =============================================================================
// ORBITALS & PARTICLE EFFECTS
// =============================================================================

// ~ Brief: Updates the position of orbital objects circling the player.
void updateOrbit(shape& player, float deltaTime);

// ~ Brief: Renders the orbital visual effects around the player scale.
void DrawOrbit(float playerScale);

// ~ Brief: Spawns a smoke particle at the given coordinates (e.g., for exhaust or firing).
void SpawnSmoke(float x, float y, float baseSize);

// ~ Brief: Updates the lifetime and fade-out of active smoke particles.
void updateSmoke(float deltaTime);