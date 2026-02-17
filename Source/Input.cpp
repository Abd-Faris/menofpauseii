#include "MasterHeader.h"

// Handles game state changing
void Debug_States() {
	if (AEInputCheckCurr(AEVK_0)) GS_next = GS_MAIN_MENU;
	else if (AEInputCheckCurr(AEVK_1)) GS_next = DEBUG1;
	else if (AEInputCheckCurr(AEVK_2)) GS_next = DEBUG2;
	else if (AEInputCheckCurr(AEVK_3)) GS_next = DEBUG3;
	else if (AEInputCheckCurr(AEVK_4)) GS_next = DEBUG4;
	else if (AEInputCheckCurr(AEVK_5)) GS_next = DEBUG5;
	else if (AEInputCheckCurr(AEVK_6)) GS_next = GS_GAME;
}