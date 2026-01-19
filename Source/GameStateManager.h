/*****************************************************************************/
/*!
\file	GameStateManager.h
\author Abdul Faris, abdulfaris.b, 2502386
\par	abdulfaris.b\@digipen.edu
\date	18/1/2026
\brief	This file declares the GSM of the program and controls its flow

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/*****************************************************************************/

#pragma once

typedef void(*FP)(void);

extern int current, previous, next;

extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

void GSM_Initialize(int startingState);

void GSM_Update();