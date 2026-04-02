#include "MasterHeader.h"

// Handles game state changing
void Debug_States() {
	if (AEInputCheckCurr(AEVK_0)) GS_next = GS_MAIN_MENU;
	else if (AEInputCheckCurr(AEVK_1)) GS_next = DEBUG1;
	else if (AEInputCheckCurr(AEVK_4)) GS_next = GS_CARD_SHOP;
	else if (AEInputCheckCurr(AEVK_6)) GS_next = GS_GAME;
}