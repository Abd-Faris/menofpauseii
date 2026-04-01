#pragma once
#include "MasterHeader.h"


struct SaveData {
    int   currentWave = 1;
    float current_hp = 0.f;
    float current_xp = 0.f;
    int   player_level = 1;
    int   barrelCount = 1;
    bool  bigcannon = false;
    std::vector<std::string> activecards{};
    std::vector<std::string> passivecards{};
};

bool SaveGame(const SaveData& data, const char* filepath = "save.dat");
bool LoadGame(SaveData& data, const char* filepath = "save.dat");
bool SaveExists(const char* filepath = "save.dat");
void DeleteSave(const char* filepath = "save.dat");