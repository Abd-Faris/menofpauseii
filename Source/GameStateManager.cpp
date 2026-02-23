/*****************************************************************************/
/*!
\file	GameStateManager.cpp
\author Men of Pause II
\brief	This file defines the GSM of the program and controls its flow

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/*****************************************************************************/

#include <iostream>

#include "MasterHeader.h"

int GS_current = 0, GS_previous = 0, GS_next = 0;

// function pointer array of CURRENT gamestate
GSFunctions GS_Functions;

// Array of Gamestate Function Pointers
// ENSURE THE ORDER MATCHES THE ENUM IN GameStateManager.h!!!
std::vector<GSFunctions> gamestates{
	{LoadMainMenu, InitializeMainMenu, UpdateMainMenu, DrawMainMenu, nullptr, UnloadMainMenu},
	{LoadGame, nullptr, UpdateGame, DrawGame, FreeGame, nullptr},
	{LoadDebug1, nullptr, UpdateDebug1, DrawDebug1, FreeDebug1, nullptr},
	{LoadDebug2, nullptr, nullptr, DrawDebug2, FreeDebug2, nullptr},
	{LoadDebug3, nullptr, nullptr, DrawDebug3, FreeDebug3, nullptr},
	{LoadDebug4, InitializeDebug4, UpdateDebug4, DrawDebug4, FreeDebug4, nullptr},
	{LoadDebug5, nullptr, nullptr, DrawDebug5, FreeDebug5, nullptr}
};
// Load, Initialize, Update, Draw, Free, Unload
// if no function of that type exists, enter nullptr

//FP fpLoad = nullptr, fpInitialize = nullptr, fpUpdate = nullptr, fpDraw = nullptr, fpFree = nullptr, fpUnload = nullptr;

//-----------------------------------------------------------//
// This function initialises the game state manager with a
// default start game state
//-----------------------------------------------------------//
void GSM_Initialize(int GS_startingState)
{
	GS_current = GS_previous = GS_next = GS_startingState;

	//some additional code
	printf("GSM:Initialize\n");
}

//-----------------------------------------------------------//
// This function updates the game state manager should the
// game state be different
//-----------------------------------------------------------//
void GSM_Update()
{
	// Switch Game States
	GS_Functions = gamestates[GS_current];
}