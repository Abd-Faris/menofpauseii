#pragma once

struct SaveData {
    int   currentWave = 1;
    float current_hp = 0.f;
    float current_xp = 0.f;
    int   player_level = 1;
    int   barrelCount = 1;
    bool  bigcannon = false;
};

bool SaveGame(const SaveData& data, const char* filepath = "save.dat");
bool LoadGame(SaveData& data, const char* filepath = "save.dat");
bool SaveExists(const char* filepath = "save.dat");
void DeleteSave(const char* filepath = "save.dat");