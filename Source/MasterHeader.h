// -----------------------------Gloomy's Revenge---------------------------- //
// File:	MasterHeader.h
// Authors:	[Men of Pause II]
// Brief:	The central inclusion hub connecting engine, systems, and states.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// THIRD-PARTY & ENGINE HEADERS
// =============================================================================

// Alpha Engine Core
#include "AEEngine.h"
#include "AEGraphics.h"
#include "AEMath.h"

// Standard Library
#include <crtdbg.h>
#include <cmath>
#include <string>
#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>

// =============================================================================
// SYSTEM & ARCHITECTURE
// =============================================================================

#include "System.h"           // Engine initialization and window management
#include "GameStateManager.h" // Logic for switching between menus and levels
#include "SaveLoad.h"         // JSON/File I/O for player progress and settings

// =============================================================================
// UTILITIES & DATA STRUCTURES
// =============================================================================

#include "Structs.h"          // Global POD types (Vectors, Entity data)
#include "Graphics.h"         // Wrapper functions for AE rendering
#include "Computation.h"      // Math helpers and collision formulas

// =============================================================================
// GAME STATES (SCENES)
// =============================================================================

#include "logo.h"             // Logo screen
#include "MainMenu.h"         // Primary navigation hub
#include "LevelSelect.h"      // Level select screen
#include "Settings.h"         // Audio and display configuration
#include "Controls.h"		  // Controls description
#include "Tutorial.h"         // Instructional overlay/state
#include "Game.h"             // Main gameplay loop
#include "Pause.h"            // Pause menu
#include "CardShop.h"         // Card upgrade system between waves
#include "Results.h"          // Win or lose state
#include "Credits.h"          // Credits screen
#include "DebugXP.h"          // Player stats

// =============================================================================
// GAMEPLAY SYSTEMS & ENTITIES
// =============================================================================

// Visuals
#include "Animations.h"       // Particle systems and sprite animations

// Entities
#include "Player.h"           // Tank movement and weapon logic
#include "Enemy.h"            // Basic AI behavior and spawning
#include "Boss.h"             // Specialized boss patterns and phases

// Environment
#include "World.h"            // Tilemap and static collisions
#include "Wave.h"             // Wave difficulty scaling and progression