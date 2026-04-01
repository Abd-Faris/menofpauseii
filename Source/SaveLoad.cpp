#include "SaveLoad.h"
#include <fstream>
#include <cstdio>

// Magic number to verify file isn't corrupt
static constexpr int SAVE_MAGIC = 0x5441424B; // "TABK"
static constexpr int SAVE_VERSION = 1;           // bump this if SaveData changes

bool SaveGame(const SaveData& data, const char* filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(&SAVE_MAGIC), sizeof(SAVE_MAGIC));
    file.write(reinterpret_cast<const char*>(&SAVE_VERSION), sizeof(SAVE_VERSION));
    file.write(reinterpret_cast<const char*>(&data), sizeof(SaveData));

    return file.good();
}

bool LoadGame(SaveData& data, const char* filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    int magic = 0, version = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    // Reject corrupt or outdated saves
    if (magic != SAVE_MAGIC || version != SAVE_VERSION) return false;

    file.read(reinterpret_cast<char*>(&data), sizeof(SaveData));
    return file.good();
}

bool SaveExists(const char* filepath) {
    std::ifstream file(filepath, std::ios::binary);
    return file.is_open();
}

void DeleteSave(const char* filepath) {
    std::remove(filepath);
}