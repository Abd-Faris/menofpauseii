// -----------------------------Gloomy's Revenge---------------------------- //
// File:    CardShop.h
// Authors: [Men of Pause II]
// Brief:   Card shop screen — card loading, generation, deck management,
//          drag-and-drop interaction, stat modifier computation, and drawing.
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h"

// =============================================================================
// EXTERNS
// =============================================================================
extern std::array<std::vector<Card>, 3>    allCards;
extern std::array<std::vector<CardStats>, NUM_OF_RARITIES> cardPool;
extern PlayerStatsModifier cardBaseMod, cardMultMod;
extern u32 upgradeFlag;

// ~ Brief: Load all textures and build meshes needed for the card shop screen
void LoadCardShop();

// ~ Brief: Build all panel meshes, reserve deck memory, populate the shop,
//          compute card home positions, and reset per-round budgets.
void InitializeCardShop();

// ~ Brief: Handle pause, Next Round button clicks, card drag-and-drop, and reroll input.
void UpdateCardShop();

// ~ Brief: Render the full card shop screen in layer order:
//          UI panels, cards, texts, card description, tutorial,
//          shop blocker (if no purchases remain), selected card and prompts (always on top).
void DrawCardShop();

// ~ Brief: Clear shop cards, reset selection pointers, and turn off the tutorial.
//          Stat modifier console dump is intentionally commented out; remove if no longer needed.
void FreeCardShop();

// ~ Brief: Free all textures and meshes owned by the card shop screen to prevent memory leaks.
void UnloadCardShop();


// =============================================================================
// CARDS NAMESPACE — JSON loading, card generation, and modifier computation
// =============================================================================
namespace Cards {
	// ~ Brief: Parse cards.json and populate cardPool, bucketed by rarity.
	//          Logs errors to stdout if the file cannot be opened or the JSON is malformed.
	//          Reference: https://rapidjson.org/md_doc_stream.html#FileStreams
	void Load_Cards(std::string const& filename);

	// ~ Brief: Assign a randomly generated card of a randomly selected rarity to card.
	//          Rerolls the rarity if the chosen bucket is empty. Clamps rarity to valid range.
	void generateCard(Card& card);

	// ~ Brief: Clear all card decks and reset all modifiers for a fresh game session.
	void resetCards();

	// ~ Brief: Recompute all stat modifiers and passive upgrade flags from the current decks.
	//          Active cards contribute active effects; inventory cards contribute passive effects.
	//          Called whenever the deck composition changes (card placed, bought, or trashed).
	void computeCardEffects();

	// ~ Brief: Search the full card pool for a card whose ID matches search,
	//          and assign its stats to card. Falls back to the first common card if not found.
	void GetCardByID(std::string& search, Card& card);
}

