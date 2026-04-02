#pragma once

// debugxp.cpp
void LoadDebug1();
void UpdateDebug1();
void DrawDebug1();
void FreeDebug1();
float get_max_hp();
void UpdateCurrentHpAfterCards(float oldMaxHp);
float calculate_max_stats(int i);
void TriggerXpPopup(float xpAmount);
void reset_game();
