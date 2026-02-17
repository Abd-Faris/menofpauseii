#pragma once
/*****************************************************************************/
/*!
\file	GameStateManager.h
\author Men of Pause II
\brief	This file declares the GSM of the program and controls its flow
*/
/*****************************************************************************/

#include "MasterHeader.h" // for function access

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
// GS Function Pointer Storage of CURRENT gamestate
extern GSFunctions GS_Functions;

// Game States Enum
enum GS_STATES
{
	// Main Game States
	GS_MAIN_MENU = 0,
	GS_GAME,

	// Debugging Game States
	DEBUG1,
	DEBUG2,
	DEBUG3,
	DEBUG4,
	DEBUG5,

	// Game Management States
	GS_QUIT,
	GS_RESTART
};

// GameState Manager Functions
void GSM_Initialize(int GS_startingState);
void GSM_Update();

