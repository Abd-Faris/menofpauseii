// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    SaveLoad.h
// Authors: [Men of Pause II]
// Brief:   Declares the SaveData structure and the file I/O functions used 
//          for persisting player progress and card inventories.
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h"

// =============================================================================
// EXTERN GLOBALS
// =============================================================================

// ~ Brief: Boolean flag indicating if a current game session is in progress.
extern bool gamecurrrun;

// =============================================================================
// STRUCTS
// =============================================================================

// ~ Brief: Container for all data required to reconstruct a player's session.
struct SaveData {
    int   currentWave = 1;
    float current_hp = 0.f;
    float current_xp = 0.f;
    int   player_level = 1;
    int   skill_point = 0;
    int   upgradeLevels[5] = { 0, 0, 0, 0, 0 };

    // ~ Brief: Lists of unique card identifiers for different inventory pools.
    std::vector<std::string> shopCardIDs;
    std::vector<std::string> activeCardIDs;
    std::vector<std::string> inventoryCardIDs;

    int   lastGameState = GS_current;
};

// =============================================================================
// FUNCTIONS
// =============================================================================

// ~ Brief: Serialize the SaveData struct into a text file.
bool SaveGame(const SaveData& data, const char* filepath = "save.txt");

// ~ Brief: Deserialize a text file into a SaveData struct.
bool LoadGame(SaveData& data, const char* filepath = "save.txt");

// ~ Brief: Verify if a save file currently exists on the disk.
bool SaveExists(const char* filepath = "save.txt");

// ~ Brief: Permanently remove the save file from the disk.
void DeleteSave(const char* filepath = "save.txt");

// ~ Brief: Helper to pack current global variables into a SaveData struct and save.
void SaveCurrentProgress(int gameState);