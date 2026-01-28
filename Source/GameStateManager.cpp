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

int current = 0, previous = 0, next = 0;

FP fpLoad = nullptr, fpInitialize = nullptr, fpUpdate = nullptr, fpDraw = nullptr, fpFree = nullptr, fpUnload = nullptr;

//-----------------------------------------------------------//
// This function initialises the game state manager with a
// default start game state
//-----------------------------------------------------------//
void GSM_Initialize(int startingState)
{
	current = previous = next = startingState;

	//some additional code
	printf("GSM:Initialize\n");
}

//-----------------------------------------------------------//
// This function updates the game state manager should the
// game state be different
//-----------------------------------------------------------//
void GSM_Update()
{
	//some unfinished code here
	//printf("GSM:Update\n");
	
	// reset function pointers
	fpLoad = nullptr, fpInitialize = nullptr, fpUpdate = nullptr, fpDraw = nullptr, fpFree = nullptr, fpUnload = nullptr;

	// Game State Switch Statements
	switch (current)
	{
	case GS_MAIN_MENU:
		fpLoad = LoadMainMenu;
		fpDraw = DrawMainMenu;
		fpFree = FreeMainMenu;
		break;
	case DEBUG1:
		fpLoad = LoadDebug1;
		fpUpdate = UpdateDebug1;
		fpDraw = DrawDebug1;
		fpFree = FreeDebug1;
		break;
	case DEBUG2:
		fpLoad = LoadDebug2;
		fpDraw = DrawDebug2;
		fpFree = FreeDebug2;
		break;
	case DEBUG3:
		fpLoad = LoadDebug3;
		fpDraw = DrawDebug3;
		fpFree = FreeDebug3;
		break;
	case DEBUG4:
		fpLoad = LoadDebug4;
		fpInitialize = InitializeDebug4;
		fpDraw = DrawDebug4;
		fpFree = FreeDebug4;
		break;
	case DEBUG5:
		fpLoad = LoadDebug5;
		fpDraw = DrawDebug5;
		fpFree = FreeDebug5;
		break;
	case DEBUG6:
		fpLoad = LoadDebug6;
		fpDraw = DrawDebug6;
		fpFree = FreeDebug6;
		break;
	case GS_RESTART:
		break;
	case GS_QUIT:
		break;
	default:
		break;
	}
}