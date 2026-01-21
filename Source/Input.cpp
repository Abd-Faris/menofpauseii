#include "MasterHeader.h"

// Handles game state changing
void Debug_States() {
	if (AEInputCheckCurr(AEVK_0)) next = GS_MAIN_MENU;
	else if (AEInputCheckCurr(AEVK_1)) next = DEBUG1;
	else if (AEInputCheckCurr(AEVK_2)) next = DEBUG2;
	else if (AEInputCheckCurr(AEVK_3)) next = DEBUG3;
	else if (AEInputCheckCurr(AEVK_4)) next = DEBUG4;
	else if (AEInputCheckCurr(AEVK_5)) next = DEBUG5;
	else if (AEInputCheckCurr(AEVK_6)) next = DEBUG6;
}