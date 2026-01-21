#pragma once
/*****************************************************************************/
/*!
\file	GameStateManager.h
\author Men of Pause II
\brief	This file declares the GSM of the program and controls its flow
*/
/*****************************************************************************/

// Function Pointer Type Declaration
typedef void(*FP)(void);
// Game State Storage
extern int current, previous, next;

// Function Pointer Struct
struct GSFunctions {
	FP Load;
	FP Initialize;
	FP Update;
	FP Draw;
	FP Free;
	FP Upload;
};

extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

void GSM_Initialize(int startingState);

void GSM_Update();

