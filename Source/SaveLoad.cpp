// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    SaveLoad.cpp
// Authors: [Men of Pause II]
// Brief:   Handles persistent data by saving and loading player stats, 
//          wave progress, upgrade levels, and card inventory to external files.
// ------------------------------------------------------------------------- //

#include "SaveLoad.h"
#include <fstream>
#include <cstdio>

// =============================================================================
// GLOBALS
// =============================================================================

// ~ Brief: Tracks whether a valid game session is currently active.
bool gamecurrrun = false;

// =============================================================================
// FUNCTIONS
// =============================================================================

// ~ Brief: Writes the provided SaveData struct to a text file at the specified path.
//          Returns true if the write operation was successful.
bool SaveGame(const SaveData& data, const char* filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    file << data.currentWave << "\n";
    file << data.current_hp << "\n";
    file << data.current_xp << "\n";
    file << data.player_level << "\n";
    file << data.skill_point << "\n";
    for (int i = 0; i < 5; i++)
        file << data.upgradeLevels[i] << "\n";
    file << data.lastGameState << "\n";

    file << "[SHOP]\n";
    file << data.shopCardIDs.size() << "\n";
    for (const auto& id : data.shopCardIDs)
        file << id << "\n";

    file << "[ACTIVE]\n";
    file << data.activeCardIDs.size() << "\n";
    for (const auto& id : data.activeCardIDs)
        file << id << "\n";

    file << "[INVENTORY]\n";
    file << data.inventoryCardIDs.size() << "\n";
    for (const auto& id : data.inventoryCardIDs)
        file << id << "\n";

    if (!file.good()) {
        return false;
    }

    return true;
}

// ~ Brief: Reads data from a text file and populates the provided SaveData struct.
//          Uses a lambda to parse categorized string vectors (Cards).
bool LoadGame(SaveData& data, const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    file >> data.currentWave;
    file >> data.current_hp;
    file >> data.current_xp;
    file >> data.player_level;
    file >> data.skill_point;
    for (int i = 0; i < 5; i++)
        file >> data.upgradeLevels[i];
    file >> data.lastGameState;

    auto readStringVector = [&](std::vector<std::string>& vec) {
        std::string label;
        file >> label;      // consume [SHOP] / [ACTIVE] / [INVENTORY]
        int count = 0;
        file >> count;
        file.ignore();      // consume newline after count
        vec.resize(count);
        for (auto& id : vec)
            std::getline(file, id);
        };

    readStringVector(data.shopCardIDs);
    readStringVector(data.activeCardIDs);
    readStringVector(data.inventoryCardIDs);

    return file.good();
}

// ~ Brief: Checks if a save file exists and can be opened for reading.
bool SaveExists(const char* filepath) {
    std::ifstream file(filepath);
    return file.is_open();
}

// ~ Brief: Deletes the save file from the local file system.
void DeleteSave(const char* filepath) {
    std::remove(filepath);
}

// ~ Brief: Captures current global player/game instances, converts them to 
//          ID-based SaveData, and triggers the file write.
void SaveCurrentProgress(int gameState) {
    SaveData data;
    data.currentWave = currentWave;
    data.current_hp = player_init.current_hp;
    data.current_xp = player_init.current_xp;
    data.player_level = player_init.player_level;
    data.skill_point = player_init.skill_point;
    for (int i = 0; i < 5; i++)
        data.upgradeLevels[i] = player_init.upgradeLevels[i];
    data.lastGameState = gameState;

    // Save each card pool as IDs
    for (const auto& card : allCards[0])      data.shopCardIDs.push_back(card.info.ID);
    for (const auto& card : allCards[1])    data.activeCardIDs.push_back(card.info.ID);
    for (const auto& card : allCards[2]) data.inventoryCardIDs.push_back(card.info.ID);

    SaveGame(data);
}