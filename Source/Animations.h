// -----------------------------Gloomy's Revenge---------------------------- //
// File:	Animations.h
// Authors:	[Men of Pause II]
// Brief:	Header for global particle effects, explosions, and impact visuals.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// LIFECYCLE FUNCTIONS
// =============================================================================

// ~ Brief: Loads animation-related textures (explosion sheets, impact sprites, etc.).
void Animations_Load();

// ~ Brief: Updates the lifecycle and frame state of all active global animations.
void Animations_Update(float dt);

// ~ Brief: Renders all active animations/particles to the screen.
void Animations_Draw();

// ~ Brief: Cleans up animation resources and resets the particle pools.
void Animations_Free();

// =============================================================================
// TRIGGER FUNCTIONS
// =============================================================================

// ~ Brief: Spawns an explosion effect at the specified coordinates with custom scaling.
void TriggerExplosion(float x, float y, float size);

// ~ Brief: Creates a bullet impact effect oriented based on the projectile's direction.
void TriggerBulletImpact(float x, float y, float bulletDirX, float bulletDirY);