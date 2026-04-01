#pragma once
#include "MasterHeader.h"


struct SaveData {
    int   currentWave = 1;
    float current_hp = 0.f;
    float current_xp = 0.f;
    int   player_level = 1;
    std::vector<std::string> shopCardIDs;
    std::vector<std::string> activeCardIDs;
    std::vector<std::string> inventoryCardIDs;
    int   lastGameState = GS_current;
};

bool SaveGame(const SaveData& data, const char* filepath = "save.dat");
bool LoadGame(SaveData& data, const char* filepath = "save.dat");
bool SaveExists(const char* filepath = "save.dat");
void DeleteSave(const char* filepath = "save.dat");
void SaveCurrentProgress(int gameState);