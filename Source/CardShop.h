#pragma once
#include "MasterHeader.h"

// Externs
extern std::array<std::vector<CardStats>, NUM_OF_RARITIES> cardPool;
extern PlayerStatsModifier cardBaseMod, cardMultMod;
extern u32 upgradeFlag;

void LoadCardShop();

void InitializeCardShop();

void UpdateCardShop();

void DrawCardShop();

void FreeCardShop();

// Functions

namespace Cards {
	// Load cards from JSON file
	void Load_Cards(std::string const& filename);

	// Randomly selects a card from the cardPool
	void generateCard(Card& card);

	// Resets card vectors
	void resetCards();

	// compute card effects
	void computeCardEffects();
}