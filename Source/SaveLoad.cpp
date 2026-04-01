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
    std::ofstream file(filepath);  // no std::ios::binary
    if (!file.is_open()) {
        OutputDebugStringA("SAVE FAILED: could not open file\n");
        return false;
    }

    file << data.currentWave << "\n";
    file << data.current_hp << "\n";
    file << data.current_xp << "\n";
    file << data.player_level << "\n";
    file << data.lastGameState << "\n";

    // Write card pools — count first then each ID
    file << data.shopCardIDs.size() << "\n";
    for (const auto& id : data.shopCardIDs)
        file << id << "\n";

    file << data.activeCardIDs.size() << "\n";
    for (const auto& id : data.activeCardIDs)
        file << id << "\n";

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
    std::ifstream file(filepath); // no std::ios::binary
    if (!file.is_open()) return false;

    file >> data.currentWave;
    file >> data.current_hp;
    file >> data.current_xp;
    file >> data.player_level;
    file >> data.lastGameState;

    // Read card pools
    auto readStringVector = [&](std::vector<std::string>& vec) {
        int count = 0;
        file >> count;
        file.ignore(); // consume newline after count
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
    data.lastGameState = gameState;

    // Save each card pool as IDs
    for (const auto& card : allCards[0])      data.shopCardIDs.push_back(card.info.ID);
    for (const auto& card : allCards[1])    data.activeCardIDs.push_back(card.info.ID);
    for (const auto& card : allCards[2]) data.inventoryCardIDs.push_back(card.info.ID);

    SaveGame(data);
}