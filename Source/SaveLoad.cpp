#include "SaveLoad.h"
#include <fstream>
#include <cstdio>

bool gamecurrrun = false;

bool SaveGame(const SaveData& data, const char* filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        OutputDebugStringA("SAVE FAILED: could not open file\n");
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
        OutputDebugStringA("SAVE FAILED: write error\n");
        return false;
    }

    OutputDebugStringA("SAVE SUCCESS\n");
    return true;
}

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

bool SaveExists(const char* filepath) {
    std::ifstream file(filepath);
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