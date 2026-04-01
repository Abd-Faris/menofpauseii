#include "SaveLoad.h"
#include <fstream>
#include <cstdio>

// Magic number to verify file isn't corrupt
static constexpr int SAVE_MAGIC = 0x5441424B; // "TABK"
static constexpr int SAVE_VERSION = 1;           // bump this if SaveData changes

static void WriteStringVector(std::ofstream& file, const std::vector<std::string>& vec) {
    int count = (int)vec.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& str : vec) {
        int len = (int)str.size();
        file.write(reinterpret_cast<const char*>(&len), sizeof(len));
        file.write(str.c_str(), len);
    }
}

static void ReadStringVector(std::ifstream& file, std::vector<std::string>& vec) {
    int count = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));
    vec.resize(count);
    for (auto& str : vec) {
        int len = 0;
        file.read(reinterpret_cast<char*>(&len), sizeof(len));
        str.resize(len);
        file.read(&str[0], len);
    }
}

bool SaveGame(const SaveData& data, const char* filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(&SAVE_MAGIC), sizeof(SAVE_MAGIC));
    file.write(reinterpret_cast<const char*>(&SAVE_VERSION), sizeof(SAVE_VERSION));

    // Write plain data
    file.write(reinterpret_cast<const char*>(&data.currentWave), sizeof(data.currentWave));
    file.write(reinterpret_cast<const char*>(&data.current_hp), sizeof(data.current_hp));
    file.write(reinterpret_cast<const char*>(&data.current_xp), sizeof(data.current_xp));
    file.write(reinterpret_cast<const char*>(&data.player_level), sizeof(data.player_level));


    // Write card pools
    WriteStringVector(file, data.shopCardIDs);
    WriteStringVector(file, data.activeCardIDs);
    WriteStringVector(file, data.inventoryCardIDs);

    return file.good();
}

bool LoadGame(SaveData& data, const char* filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    int magic = 0, version = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (magic != SAVE_MAGIC || version != SAVE_VERSION) return false;

    file.read(reinterpret_cast<char*>(&data.currentWave), sizeof(data.currentWave));
    file.read(reinterpret_cast<char*>(&data.current_hp), sizeof(data.current_hp));
    file.read(reinterpret_cast<char*>(&data.current_xp), sizeof(data.current_xp));
    file.read(reinterpret_cast<char*>(&data.player_level), sizeof(data.player_level));


    ReadStringVector(file, data.shopCardIDs);
    ReadStringVector(file, data.activeCardIDs);
    ReadStringVector(file, data.inventoryCardIDs);

    return file.good();
}

bool SaveExists(const char* filepath) {
    std::ifstream file(filepath, std::ios::binary);
    return file.is_open();
}

void DeleteSave(const char* filepath) {
    std::remove(filepath);
}

void SaveCurrentProgress(int gameState) {
    SaveData data;
    data.currentWave = currentWave;
    data.current_hp = player_init.current_hp;
    data.current_xp = player_init.current_xp;
    data.player_level = player_init.player_level;
    data.lastGameState = gameState;

    // Save each card pool as IDs
    for (const auto& card : allCards[0])      data.shopCardIDs.push_back(card.info.ID);
    for (const auto& card : allCards[1])    data.activeCardIDs.push_back(card.info.ID);
    for (const auto& card : allCards[2]) data.inventoryCardIDs.push_back(card.info.ID);

    SaveGame(data);
}