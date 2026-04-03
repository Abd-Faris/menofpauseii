// -----------------------------Gloomy's Revenge---------------------------- //
// File:    GameStateManager.h
// Authors: [Men of Pause II]
// Brief:   Defines the Game State Manager (GSM) — maintains current, previous,
//          and next game states, maps each state to its function pointers, and
//          drives state transitions each frame.
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h" // for function access

// =============================================================================
// GAME STATE TABLE
// =============================================================================

// Function Pointer Type Declaration
typedef void(*FP)(void);
// Game State Storage
extern int GS_current, GS_previous, GS_next;

// Function Pointer Struct
struct GSFunctions {
	FP Load;
	FP Initialize;
	FP Update;
	FP Draw;
	FP Free;
	FP Unload;
};

// Game State Enum
enum GS_STATES
{
	// Main Game States
	GS_LOGO = 0,
	GS_MAIN_MENU,
	GS_GAME,
	GS_RESULTS,
	GS_LEVEL_SELECT,
	GS_CREDITS,
	GS_SETTINGS,
	GS_CONTROLS,

	// Debugging Game States
	DEBUG1,
	GS_CARD_SHOP,
	

	// Game Management States
	GS_QUIT,
	GS_RESTART
};

// =============================================================================
// EXTERNS
// =============================================================================

// GS Function Pointer Storage of CURRENT gamestate
extern GSFunctions GS_Functions;

// =============================================================================
// GSM FUNCTIONS
// =============================================================================

// ~ Brief: Initialise the GSM with a starting game state.
//          Sets current, previous, and next to the same state so the first
//          frame loads and enters cleanly without a spurious transition.
void GSM_Initialize(int GS_startingState);

// ~ Brief: Resolve the function pointer set for the current game state.
//          Called once per frame before the lifecycle functions are dispatched.
void GSM_Update();

