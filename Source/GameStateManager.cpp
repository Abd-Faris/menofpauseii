/*****************************************************************************/
/*!
\file	GameStateManager.cpp
\author Abdul Faris, abdulfaris.b, 2502386
\par	abdulfaris.b\@digipen.edu
\date	18/1/2026
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

	switch (current)
	{
	case GS_MAIN_MENU:
		fpLoad = LoadMainMenu;
		fpDraw = DrawMainMenu;
		fpFree = FreeMainMenu;
		break;
	case GS_LEVEL2:
		break;
	case GS_RESTART:
		break;
	case GS_QUIT:
		break;
	default:
		break;
	}
}