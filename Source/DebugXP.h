// -----------------------------Gloomy's Revenge---------------------------- //
// File:	DebugXP.h
// Authors:	[Men of Pause II]
// Brief:	Header file declaring HUD, XP logic, and player stat functions.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

// ~ Brief: Loads all visual assets (meshes/textures) for the HUD and menus.
void LoadDebug1();

// ~ Brief: Handles logic for level-ups, menu interaction, and debug inputs.
void UpdateDebug1();

// ~ Brief: Renders the player HUD, enemy health bars, and upgrade menus.
void DrawDebug1();

// ~ Brief: Cleans up memory by freeing meshes and unloading textures.
void FreeDebug1();

// ~ Brief: Helper to retrieve the current maximum HP including all modifiers.
float get_max_hp();

// ~ Brief: Adjusts the player's current health when max HP changes via cards.
void UpdateCurrentHpAfterCards(float oldMaxHp);

// ~ Brief: Calculates the effective value of a specific stat (HP, DMG, etc.).
float calculate_max_stats(int i);

// ~ Brief: Initializes the floating XP text animation above the HUD.
void TriggerXpPopup(float xpAmount);

// ~ Brief: Resets player levels, XP, and world state for a fresh start.
void reset_game();