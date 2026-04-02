// -----------------------------Gloomy's Revenge---------------------------- //
// File:	WaveSystem.h
// Authors:	[Men of Pause II]
// Brief:	Header for the wave spawning logic and enemy population management.
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h"

// =============================================================================
// EXTERNS
// =============================================================================

extern float spawnTimer;      // Cooldown between individual enemy spawns
extern float pendingBudget;    // Remaining "points" to spend on enemies this wave
extern float totalWaveBudget;  // The total difficulty value assigned to the wave

// =============================================================================
// WAVE FUNCTIONS
// =============================================================================

// ~ Brief: Calculates the difficulty budget and prepares the next wave of enemies.
void GenerateWave(int waveNumber, shape& player);

// ~ Brief: Handles the timing and logic of spawning enemies until the budget is empty.
void UpdateWaveSpawning(float dt, shape& player);

// ~ Brief: Checks if all enemies in the current wave have been defeated.
bool IsWaveCleared();

// ~ Brief: Debug/Cheat function to instantly clear the current wave and move forward.
void skipWave(shape& player);

// ~ Brief: Utility function to output the current number of active enemies to the console.
void printEnemyCount();