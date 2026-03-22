#pragma once
#include "MasterHeader.h"
#include "../Extern/rapidjson/document.h"
#include "../Extern/rapidjson/error/en.h"
#include <fstream>
#include <sstream>

// externs
// declare array for ALL loaded cards in the game
std::array<std::vector<CardStats>, NUM_OF_RARITIES> cardPool;
// structs to keep track of player modifiers
PlayerStatsModifier cardBaseMod{0.f, 0.f, 0.f, 0.f, 0.f}, cardMultMod{1.f, 1.f, 1.f, 1.f, 1.f};
u32 upgradeFlag{ UPGRADE_NONE }; // flag for upgrades

// externs from other files
extern int currentWave;

namespace { // functions for InitializeCardShop()
	
	// define meshes
	AEGfxVertexList* rectMesh{};
	AEAudio mainbgm{ nullptr };
	AEAudioGroup bgm{ nullptr };
	
	// graphic boxes
	AEGfxVertexList* bag, * shop, * desc, * cardSlots, * trash;
	u32 cbag{ 0xFFC77014 }, cshop{ 0xFFcccc00 }, cdesc{ 0xFF000000 }, ccardSlots{ 0xFFcccc00 }, ctrash{ 0xFFe62e00 };
	GfxText cardCount{ "", 0.5f };

	// panel textures
	AEGfxTexture* pBgTex = nullptr;
	AEGfxTexture* pShopTex = nullptr; // shoporange.png — bag panel
	AEGfxTexture* pSlotsTex = nullptr; // shopyellow.png — shop + active slots
	AEGfxTexture* pTrashTex = nullptr; // trash.png
	AEGfxVertexList* pPanelMesh = nullptr; // shared UV mesh for panels

	// spritesheet constants
	AEGfxTexture* cardSpriteSheet{ nullptr };
	const int NUM_COLS{ 4 };
	const int NUM_ROWS{ 7 };

	// card scales for diff vector arrays
	f32 CARD_SHOP_SCALE{ 7 };
	f32 ACTIVE_CARD_SCALE{ 3 };
	f32 INVENTORY_CARD_SCALE{ 3 };

	// base num of cards availbale to player (without card effects applied)
	const int base_shopCards     { 3 };
	const int base_activeCards   { 5 };
	const int base_inventoryCards{ 15 };
	// base num of cards from shop buy-able
	const int base_buyable	   { 1 };

	// MAX num of cards available to be displayed to player
	const int max_shopCards		 { 7 };
	const int max_activeCards	 { 10 };
	const int max_inventoryCards { 15 };
	const int buyable_max		 { 3 };

	// num of cards available to be displayed to player
	int num_shopCards      { 3 };
	int num_activeCards    { 5 };
	int num_inventoryCards { 15 };
	
	// num of cards from shop buy-able
	int num_buyable		   { 1 };
	int buyable_left	   { 0 };

	// rarity weights (out of 100)
	const float RARITY_WEIGHTS[] = {
		50.0f,  // COMMON
		30.0f,  // RARE
		15.0f,  // EPIC
		5.0f,   // UNIQUE
	};

	// declare array for cards
	std::array<std::vector<Card>, 3>    allCards;	// vector for all cards
	// reference for easy ref
	std::vector<Card>& shopCards      = allCards[0];// reference to shop cards
	std::vector<Card>& activeCards    = allCards[1];// ref to active cards
	std::vector<Card>& inventoryCards = allCards[2];// reference to cards in bag
	// pointer to selected card and hovered over card
	Card* pSelectedCard{nullptr};
	Card* pHoveredCard {nullptr};

	// init shop texts
	std::vector<GfxText> shopTexts{
		{"BAG", 0.5f, 0, 0, 0, 255, {575, 375}},
		{"SHOP", 0.8f, 0, 0, 0, 255, {-200, 195}},
		{"ACTIVE CARDS", 0.5f, 0, 0, 0, 255, {-125, -225}}
	};

	// initialises the card shop array
	void initCardShop(std::vector<Card>& cardShop) {
		// create 3 cards for shop
		Card card{};
		for (int i{ 0 }; i < num_shopCards; ++i) {
			// generate card FIRST
			Cards::generateCard(card);
			// assign other data members
			card.from = DECK::SHOP;
			cardShop.push_back(card); // push card into vector
		}
	}

	// Computes the home positions of an array of cards within a bounded range
	void computeXCardPositions(std::vector<Card>& arr, f32 start, f32 end, f32 y, f32 scale = 10, f32 maxGap = 100.f) {
		// EDGECASE: if array empty, return
		if (arr.empty()) return;
		
		// SET NUM OF DISPLAY CARDS
		int displayCards = static_cast<int>(arr.size());
		// set range for shop INCL PADDING ON EITHER SIDE
		start += (arr[0].size.x / 2) * scale;
		end   -= (arr[0].size.x / 2) * scale;
		f32 totalRange = end - start;

		f32 contentWidth = (displayCards - 1) * maxGap;

		// If cards are too spread out, shrink range and center
		if (contentWidth < totalRange && displayCards > 1) {
			f32 offset = (totalRange - contentWidth) / 2.0f;
			start += offset;
			end -= offset;
		}

		// finds range
		f32 range = end - start;

		// normalise card position within range
		for (int i{ 0 }; i < displayCards; ++i) {
			// EDGECASE: ONLY compute if more than 1 card
			if (arr.size() <= 1) {
				arr[i].pos.x = start + (0.5f*range); // centers one card
				arr[i].pos.y = y;
				// assigns homepos to computed pos
				arr[i].homepos = arr[i].pos;
				continue;
			}
			// normalise card into range
			f32 normalized = static_cast<f32>(i) / (displayCards - 1);
			// denormalise from range into world coords
			arr[i].pos.x = start + (normalized * range);
			arr[i].pos.y = y;
			// assigns homepos to computed pos
			arr[i].homepos = arr[i].pos;
		}
		return;
	}

	// Calculates home positions of cards in Inventory Deck
	void computeBagCardPositions() {
		// init hardcoded positions of the inventory cards
		std::vector <float> xpos{ 487.5f, 575.f, 662.5f };
		std::vector <float> ypos{ 287.5f, 145.f, 2.5f, -140.f, -282.5f };

		// assign positions of all cards in inventory deck
		for (int i = 0; i < inventoryCards.size(); ++i) {
			int row = i / 3; // computes row
			int col = i % 3; // computes column

			// if run out of defined grid positions, break
			if (row >= ypos.size()) break;

			// assign card home positions
			inventoryCards[i].homepos = { xpos[col], ypos[row] };
			inventoryCards[i].pos = inventoryCards[i].homepos;
		}

		return;
	}

	// Driver for Calulating Home Positions of ALL cards
	void computeCardHomePos() {
		// if vector is not empty, calc positions
		if (!activeCards.empty()) {
			computeXCardPositions(activeCards, -550.f, 300.f, -300.f, ACTIVE_CARD_SCALE, 150.f);
			for (Card& card : activeCards) {
				card.homepos = card.pos;
				Comp::computeBoundingBox(card.boundingBox, card.homepos, card.size, ACTIVE_CARD_SCALE * 10);
			}
		}
		if (!shopCards.empty()) {
			computeXCardPositions(shopCards, -700.f, 300.f, 20.f, CARD_SHOP_SCALE, 300.f);
			for (Card& card : shopCards) {
				card.homepos = card.pos;
				Comp::computeBoundingBox(card.boundingBox, card.homepos, card.size, CARD_SHOP_SCALE * 10);
			}
		}
		if (!inventoryCards.empty()) {
			computeBagCardPositions();
			for (Card& card : inventoryCards) {
				card.homepos = card.pos;
				Comp::computeBoundingBox(card.boundingBox, card.homepos, card.size, INVENTORY_CARD_SCALE * 10);
			}
		}
		return;
	}
}

void LoadCardShop() {
	// load audio assets
	mainbgm = AEAudioLoadMusic("Assets/audio/bgm/credits_bgm.mp3");
	bgm = AEAudioCreateGroup();
	AEAudioPlay(mainbgm, bgm, 2.f, 1.f, -1);

	// loads graphics
	cardSpriteSheet = AEGfxTextureLoad("Assets/cards.png");
	if (!cardSpriteSheet) std::cout << "[ ERROR ] cards.png failed to load!\n";

	// load panel textures
	pBgTex = AEGfxTextureLoad("./Assets/sandbg.png");
	pShopTex = AEGfxTextureLoad("./Assets/shoporange.png");
	pSlotsTex = AEGfxTextureLoad("./Assets/shopyellow.png");
	pTrashTex = AEGfxTextureLoad("./Assets/trash.png");

	// shared UV mesh for panels
	AEGfxMeshStart();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pPanelMesh = AEGfxMeshEnd();

	PauseScreen::LoadPause();
}

void InitializeCardShop() {
	// create container meshes
	// card spritesheet unit size
	f32 uSize = 1.0f / NUM_COLS;
	f32 vSize = 1.0f / NUM_ROWS;
	rectMesh = Gfx::createRectMesh(0xFFFFFFFF, 0.f, 0.f, uSize, vSize);

	bag = Gfx::createRectMesh(cbag);
	shop = Gfx::createRectMesh(cshop);
	desc = Gfx::createRectMesh(cdesc);
	cardSlots = Gfx::createRectMesh(ccardSlots);
	trash = Gfx::createRectMesh(ctrash);

	// [ DO NOT TOUCH ] Reserve space in memory for different card categories
	shopCards.reserve(5);       // max cards allowed
	activeCards.reserve(10);    // max cards allowed
	inventoryCards.reserve(15); // max cards allowed

	// initializes shop cards
	initCardShop(shopCards);
	computeCardHomePos();
	buyable_left = num_buyable;
}

namespace { // functions for UpdateCardShop()

	// removes card from its source, called when shifting cards between decks
	void removeFromSource(Card& card) {
		// determine and create pointer to sourceDeck
		std::vector<Card>* sourceDeck = &shopCards; // default shopCards
		switch (card.from) {
		case DECK::ACTIVE: sourceDeck = &activeCards; break;
		case DECK::BAG:    sourceDeck = &inventoryCards; break;
		}

		// if card from shop, decrement buyable_left
		if (card.from == DECK::SHOP) --buyable_left;

		// for loop to find index of card in sourceDeck
		for (size_t i = 0; i < (*sourceDeck).size(); ++i) {
			// if homepos matches (homepos is unique to each card)
			if (Comp::AEVec2Equal((*sourceDeck)[i].homepos, card.homepos)) {
				// erase card from old deck
				(*sourceDeck).erase((*sourceDeck).begin() + i);
				break; // break out of loop
			}
		}
	}

	// triggers when pSelectedCard != nullptr
	void handleCardDrop() {
		Card& card = *pSelectedCard;
		// if dropped in bag
		if (Comp::collisionPointRect(card.pos, { 575, 0 }, { 350,800 })) {
			// only trigger if not from bag AND have space
			if ((card.from == DECK::BAG) || (inventoryCards.size() >= num_inventoryCards)) return;

			// reset pSelectedCard
			pSelectedCard = nullptr;

			// add card to bag deck, delete from old deck
			Card newCard = card;
			newCard.from = DECK::BAG;
			inventoryCards.push_back(newCard);

			removeFromSource(card);
			computeCardHomePos();
		}
		// else if dropped in active
		else if (Comp::collisionPointRect(card.pos, { -125, -300 }, { 950, 200 })) {
			// only trigger if not from active AND have space
			if ((card.from == DECK::ACTIVE) || (activeCards.size() >= num_activeCards)) return;

			// reset pSelectedCard
			pSelectedCard = nullptr;

			// add card to active deck, delete from old deck
			Card newCard = card;
			newCard.from = DECK::ACTIVE;
			activeCards.push_back(newCard);

			removeFromSource(card);
			computeCardHomePos();
		}
		// trash
		else if (Comp::collisionPointRect(card.pos, { -700, -300 }, { 100, 100 })) {
			// only trigger if not from shop
			if (card.from == DECK::SHOP) return;

			// reset pSelectedCard
			pSelectedCard = nullptr;

			// trash card
			removeFromSource(card);
			computeCardHomePos();
		}
	}

	// selectedCard to follow cursor until left click released
	void updateCardPosition() {
		// get selected card
		Card& card = *pSelectedCard;

		// update card position
		AEVec2 deltacursorpos{};
		Comp::getDeltaCursorPos(deltacursorpos);
		card.pos.x += deltacursorpos.x;
		card.pos.y -= deltacursorpos.y;

		// if user NOT holding left click
		if (!AEInputCheckCurr(AEVK_LBUTTON)) {
			handleCardDrop();
			// reset to home pos
			card.pos = card.homepos;
			// set selected card ptr to null
			pSelectedCard = nullptr;
			// updates card effects
			Cards::computeCardEffects();
		}
		
	}

	// checks card collision with boxes of other decks
	void checkCardCollision() {
		// get cursor position
		AEVec2 cursorpos{};
		Comp::getCursorPos(cursorpos);

		// for every card in ALL arrays, check for a hit
		// if hit, assign card to ptr IF left button is triggered
		for (Card& card : activeCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				// set card to display description
				pHoveredCard = &card;
				if (AEInputCheckTriggered(AEVK_LBUTTON)) pSelectedCard = &card;
				return;
			}
		}
		for (Card& card : inventoryCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				// set card to display description
				pHoveredCard = &card;
				if (AEInputCheckTriggered(AEVK_LBUTTON)) pSelectedCard = &card;
				return;
			}
		}
		// if user cannot buy cards, return
		if (buyable_left <= 0) return;
		
		// print shop cards
		for (Card& card : shopCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				// set card to display description
				pHoveredCard = &card;
				if (AEInputCheckTriggered(AEVK_LBUTTON)) pSelectedCard = &card;
				return;
			}
		}
		// else return no hit at all
		pHoveredCard = nullptr;
		return;
	}

	void checkContinue() {
		// skip if left click not clicked
		if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

		// get cursor position
		AEVec2 mousepos{};
		Comp::getCursorPos(mousepos);

		// if user clicked within shop bounds
		if (Comp::collisionPointRect(mousepos, { -200, 40 }, { 1100, 380 })) {
			// set next gs back to GS_GAME
			GS_next = GS_GAME;
		}
	}
}

void UpdateCardShop() {
	PauseScreen::UpdatePause();
	// if theres a card selected, update selected card position
	if (pSelectedCard) {
		updateCardPosition();
		// set selected card to display description
		pHoveredCard = pSelectedCard;
	}
	// else check collision for ALL cards
	else {
		checkCardCollision();
	} // endif

	// if player cant buy any more cards, check if player selected to continue
	if (buyable_left <= 0) checkContinue();
	
}

namespace { // functions for DrawCardShop()

	// calculates UV coords of cards spritesheet
	AEGfxVertexList* createCardMesh(const Card& card) {
		f32 uMin = card.info.col * (1.0f / NUM_COLS);
		f32 vMin = card.info.row * (1.0f / NUM_ROWS);
		f32 uMax = uMin + (1.0f / NUM_COLS);
		f32 vMax = vMin + (1.0f / NUM_ROWS);

		std::cout << "Card: " << card.info.ID
			<< " row:" << card.info.row
			<< " col:" << card.info.col
			<< " UV: (" << uMin << "," << vMin << ") -> (" << uMax << "," << vMax << ")\n";
		return Gfx::createRectMesh(0xFFFFFFFF, uMin, vMin, uMax, vMax);
	}

	void drawCards() {
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

		for (Card& indivCard : shopCards) {
			f32 uOffset = indivCard.info.col * (1.0f / NUM_COLS);
			f32 vOffset = indivCard.info.row * (1.0f / NUM_ROWS);
			// UV offset
			AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
			// Draw using the single, pre-loaded mesh
			Gfx::printMesh(rectMesh, indivCard, CARD_SHOP_SCALE);
		}
		for (Card& indivCard : activeCards) {
			f32 uOffset = indivCard.info.col * (1.0f / NUM_COLS);
			f32 vOffset = indivCard.info.row * (1.0f / NUM_ROWS);
			// UV offset
			AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
			// Draw using the single, pre-loaded mesh
			Gfx::printMesh(rectMesh, indivCard, ACTIVE_CARD_SCALE);
		}
		for (Card& indivCard : inventoryCards) {
			f32 uOffset = indivCard.info.col * (1.0f / NUM_COLS);
			f32 vOffset = indivCard.info.row * (1.0f / NUM_ROWS);
			// UV offset
			AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
			// Draw using the single, pre-loaded mesh
			Gfx::printMesh(rectMesh, indivCard, INVENTORY_CARD_SCALE);
		}

		// set graphics settings back
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
		AEGfxSetTransparency(1.f);
	}

	//// draws cards
	//void drawCards() {
	//	for (Card& indivCard : shopCards) {
	//		Gfx::printMesh(rectMesh, indivCard, CARD_SHOP_SCALE);
	//	}
	//	for (Card& indivCard : activeCards) {
	//		Gfx::printMesh(rectMesh, indivCard, ACTIVE_CARD_SCALE);
	//	}
	//	for (Card& indivCard : inventoryCards) {
	//		Gfx::printMesh(rectMesh, indivCard, INVENTORY_CARD_SCALE);
	//	}
	//}

	void drawTexts() {
		// STATIC TEXTS
		for (GfxText& text : shopTexts) {
			Gfx::printText(text, boldPixels);
		}
		// DYNAMIC TEXTS
		// ACTIVE
		//GfxText cardCount{ "", 0.5f };
		cardCount.pos = { -125, -375 };
		cardCount.text = std::to_string(activeCards.size()) + "/" + std::to_string(num_activeCards);
		Gfx::printText(cardCount, boldPixels);
		// INVENTORY
		cardCount.pos = { 575, -375 };
		cardCount.text = std::to_string(inventoryCards.size()) + "/" + std::to_string(num_inventoryCards);
		Gfx::printText(cardCount, boldPixels);
	}

	// prompts to user to let go to drop into a diff inventory
	// prompts to user to let go to drop into a diff inventory
	void drawPrompts() {
		Card& card = *pSelectedCard;
		// check collision with non-shop inventories

		// bag deck prompt
		if (Comp::collisionPointRect(card.pos, { 575, 0 }, { 350,800 })) {
			// dont display if selectedCard was from the bag deck
			if ((card.from == DECK::BAG)) return;
			// display bag deck prompt with texture
			AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
			AEGfxTextureSet(pShopTex, 0, 0);
			AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
			AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
			AEGfxSetBlendMode(AE_GFX_BM_BLEND);
			AEGfxSetTransparency(1.f);
			Gfx::printMesh(pPanelMesh, { 575, 0 }, { 350, 800 }, 0.f, { 0.f, 0.f }, true);
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			GfxText text{ "Add to Bag", 0.8f };
			text.pos = { 575, 0 };
			Gfx::printText(text, boldPixels);
		}
		// active deck prompt
		else if (Comp::collisionPointRect(card.pos, { -125, -300 }, { 950, 200 })) {
			// dont display if selectedCard was from the active deck
			if ((card.from == DECK::ACTIVE)) return;
			// display active deck prompt with texture
			AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
			AEGfxTextureSet(pSlotsTex, 0, 0);
			AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
			AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
			AEGfxSetBlendMode(AE_GFX_BM_BLEND);
			AEGfxSetTransparency(1.f);
			Gfx::printMesh(pPanelMesh, { -125, -300 }, { 950, 200 }, 0.f, { 0.f, 0.f }, true);
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			GfxText text{ "Add to Active Cards" };
			text.pos = { -125, -300 };
			Gfx::printText(text, boldPixels);
		}
	}

	// draws the updated selected card's location based on where the cursor is
	void drawSelectedCard() {

		// set graphics settings
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

		// determine card size based on card type
		Card& card = *pSelectedCard;
		f32 scale{};
		// determine scale
		switch (card.from) {
		case DECK::SHOP:   scale = CARD_SHOP_SCALE; break;
		case DECK::ACTIVE: scale = ACTIVE_CARD_SCALE; break;
		case DECK::BAG:    scale = INVENTORY_CARD_SCALE; break;
		}
		//// create mesh to draw
		//AEGfxVertexList* cardMesh = createCardMesh(*pSelectedCard);
		//Gfx::printMesh(cardMesh, *pSelectedCard, scale);
		//AEGfxMeshFree(cardMesh);
		////Gfx::printMesh(rectMesh, *pSelectedCard, scale);

		f32 uOffset = card.info.col * (1.0f / NUM_COLS);
		f32 vOffset = card.info.row * (1.0f / NUM_ROWS);
		// UV offset
		AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
		// Draw using the single, pre-loaded mesh
		Gfx::printMesh(rectMesh, card, scale);

		// set graphics settings back
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
		AEGfxSetTransparency(1.f);
	}

	// displays description of card
	void drawCardDescription(bool printDefault = false) {

		if (printDefault) {
			// print default description text
			GfxText text{ "", 1.f , 255, 255, 255, 255 };
			text.pos = { -200, 325 };
			text.text = "Select a card to continue!";
			Gfx::printText(text, boldPixels);
			return;
		}


		// Defines GfxText object
		GfxText cardDesc{ "", 0.5f, 255, 255, 255, 255 }; // set font colour to white
		cardDesc.pos = { -200, 360 };

		Card& card = *pHoveredCard;
		// if active card effect array isnt empty, concatenate desc
		if (!card.info.active.empty()) {
			for (CardEffect& effect : card.info.active) {
				cardDesc.text += effect.desc + '\n';
			}
		}

		// if passive card array isnt empty, concatenate desc
		if (!card.info.passive.empty()) {
			cardDesc.text += "[ PASSIVE ABILITY ]\n";
			for (CardEffect& effect : card.info.passive) {
				cardDesc.text += effect.desc + '\n';
			}
		}

		// print cardDescription
		Gfx::printMultiline(cardDesc, boldPixels);
	}

	void blockShop() {
		// block user from viewing the other cards with texture
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxTextureSet(pSlotsTex, 0, 0);
		AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.f);
		Gfx::printMesh(pPanelMesh, { -200, 40 }, { 1100, 380 }, 0.f, { 0.f, 0.f }, true);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

		// prints stats
		GfxText text{ "", 0.5f };
		text.pos = { -200, 180 };

		text.text += "Current Wave: " + std::to_string(currentWave) + "\n\n\n\n\n";
		text.text += "CLICK ANYWHERE HERE TO CONTINUE\n\n\n\n\n";

		int nextBoss{ 6 - (currentWave % 5) };
		// if boss spawns next wave, wish them luck
		if (nextBoss == 6) {
			text.text += "BOSS SPAWNING THIS WAVE\nGood Luck :)";
		}
		else { // normal text
			text.text += std::to_string(nextBoss)
				+ " MORE WAVE" + (nextBoss > 1 ? "S" : "")
				+ " TO BOSS SPAWN";
		}
		Gfx::printMultiline(text, boldPixels);
	}
}

// -600 -200 200
void DrawCardShop() {
	
	// -- draw background texture fullscreen --
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxTextureSet(pBgTex, 0, 0);
	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.f);
	Gfx::printMesh(pPanelMesh, { 0.f, 0.f },
		{ (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight() }, 0.f, { 0.f, 0.f }, true);

	// -- draw textured panels --
	AEGfxTextureSet(pShopTex, 0, 0);
	Gfx::printMesh(pPanelMesh, { 575, 0 }, { 350, 800 }, 0.f, { 0.f, 0.f }, true);        // bag

	AEGfxTextureSet(pSlotsTex, 0, 0);
	Gfx::printMesh(pPanelMesh, { -200, 40 }, { 1100, 380 }, 0.f, { 0.f, 0.f }, true);     // shop

	AEGfxTextureSet(pSlotsTex, 0, 0);
	Gfx::printMesh(pPanelMesh, { -125, -300 }, { 950, 200 }, 0.f, { 0.f, 0.f }, true);    // active slots

	AEGfxTextureSet(pTrashTex, 0, 0);
	Gfx::printMesh(pPanelMesh, { -700, -300 }, { 100, 100 }, 0.f, { 0.f, 0.f }, true);    // trash

	// -- desc panel --
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);
	Gfx::printMesh(desc, { -200, 325 }, { 1100, 150 });

	// print card shop UI to screen
	drawCards();
	drawTexts();
	PauseScreen::DrawPauseButton();

	// print card description
	// if a card is hovered over, display its description
	if (pHoveredCard) {
		drawCardDescription(false);
	}
	else {
		drawCardDescription(true);
	}

	// if user cant buy any more cards, block access to shop
	if (buyable_left <= 0) blockShop();

	// prints selected card [ ALWAYS ON TOP ]
	// if theres a card selected, draw prompts
	if (pSelectedCard) {
		drawPrompts();		// prompts to user to let go within deck bounds
		drawSelectedCard(); // ensure selected card always draws on top
	}
	PauseScreen::DrawPause();
}

void FreeCardShop() {
	// free meshes
	AEGfxMeshFree(rectMesh);
	AEGfxMeshFree(bag);
	AEGfxMeshFree(shop);
	AEGfxMeshFree(desc);
	AEGfxMeshFree(cardSlots);
	AEGfxMeshFree(trash);
	PauseScreen::FreePause();

	shopCards.clear(); // clear shop array

	// print player stats to console
	// calculates player stats
	f32 hp = calculate_max_stats(0);
	f32 dmg = calculate_max_stats(1);
	f32 speed = calculate_max_stats(2);
	f32 fire_rate = calculate_max_stats(3);
	f32 xp_mult = calculate_max_stats(4);

	// reset null pointers
	pSelectedCard = nullptr;
	pHoveredCard = nullptr;

	std::cout 
		<< "\n====================================\n" 
		<< "HP: " << hp
		<< "\nDamage: " << dmg
		<< "\nSpeed: " << speed
		<< "\nFire Rate: " << fire_rate
		<< "\nXP Mult: " << xp_mult
		<< "\n====================================\n";
}

void UnloadCardShop() {
	// unload audio
	AEAudioUnloadAudio(mainbgm);
	AEAudioUnloadAudioGroup(bgm);
	// Unload Graphics
	AEGfxTextureUnload(cardSpriteSheet);
	// unload panel textures
	if (pBgTex) { AEGfxTextureUnload(pBgTex);     pBgTex = nullptr; }
	if (pShopTex) { AEGfxTextureUnload(pShopTex);   pShopTex = nullptr; }
	if (pSlotsTex) { AEGfxTextureUnload(pSlotsTex);  pSlotsTex = nullptr; }
	if (pTrashTex) { AEGfxTextureUnload(pTrashTex);  pTrashTex = nullptr; }
	if (pPanelMesh) { AEGfxMeshFree(pPanelMesh);      pPanelMesh = nullptr; }
}

namespace Cards {
	// Parses a CardEffect array from a JSON array, allows for multiple effects per card
	std::vector<CardEffect> parseEffects(const rapidjson::Value& effectArray) {
		// declare resultant object
		std::vector<CardEffect> effects;

		// for each card effect
		for (const auto& e : effectArray.GetArray()) {
			// declare placeholder object
			CardEffect effect;
			// if data member exists, parse json value into object
			if (e.HasMember("id")) effect.id = e["id"].GetString();
			else effect.id = "";
			
			if (e.HasMember("type")) effect.type = e["type"].GetString();
			else effect.type = "";
			
			if (e.HasMember("desc")) effect.desc = e["desc"].GetString();
			else effect.desc = "";
			
			if (e.HasMember("valuetype")) effect.valuetype = e["valuetype"].GetString();
			else effect.valuetype = "";
			
			if (e.HasMember("value")) effect.value = e["value"].GetFloat();
			else effect.value = 0.f;
			// append to resultant object
			effects.push_back(effect);
		}

		return effects;
	}

	// Loads cards from JSON file into cardPool
	// Documentation here: https://rapidjson.org/md_doc_stream.html#FileStreams
	// code will run insya allah
	void Load_Cards(std::string const& filename) {
		// clear cards vector
		for (int i{}; i < NUM_OF_RARITIES; i++) {
			cardPool[i].clear();
		}
		
		// open filestream
		std::ifstream file{ filename };
		// produce error if file cannot be opened
		if (!file.is_open()) {
			std::cout << "[ ERROR ] " << filename << " failed to open!!\n";
			return;
		}

		// creates string stream
		std::stringstream buffer;
		// read JSON file into string stream
		buffer << file.rdbuf();
		// assign contents of stream to string
		std::string json = buffer.str();

		// parses string into a type rapidjson::Document via c style strings
		rapidjson::Document doc;
		// copied this part from the documentation lol
		// adapted from c to c++
		rapidjson::ParseResult ok = doc.Parse(json.c_str());

		// if somehow (god forbid) got error parsing, output error and return
		if (ok.IsError()) {
			std::cout << "[ ERROR ] JSON Parse Error: " << GetParseError_En(ok.Code())
				<< " at offset " << ok.Offset() << '\n';
			return;
		}

		// for every parsed card within the "cards" container
		for (const auto& cardJson : doc["cards"].GetArray()) {
			// placeholder CardStats object
			CardStats card;
			// translate parsing to object data members
			card.ID = cardJson["id"].GetString();
			card.rarity = cardJson["rarity"].GetInt();
			card.row = cardJson["sprite_row"].GetInt();
			card.col = cardJson["sprite_col"].GetInt();

			// oursource this stuff to a helper function
			card.active = parseEffects(cardJson["active"]);
			card.passive = parseEffects(cardJson["passive"]);

			// push complete Card Stats to corresponding rarity card pool
			::cardPool[card.rarity].push_back(card);
		}
	}

	// returns a rarity at random based on pre-set weights
	int randomiseRarity() {
		// rand num and normalsie from 0 - 100
		float rng = AERandFloat() * 100.0f;
		float total = 0.0f;

		// for each rarity, add its weight and compare to rand float
		for (int i = 0; i < NUM_OF_RARITIES; i++) {
			total += RARITY_WEIGHTS[i];
			// if its a hit, return that rarity
			if (rng < total) return static_cast<Rarity>(i);
		}
		// return default case
		return COMMON;
	}

	// Assigns a random card of a random rarity from cardPool to the card param
	void generateCard(Card& card) {
		int cardRarity;
		do {
			cardRarity = randomiseRarity();
		} while (::cardPool[cardRarity].empty());

		// create reference to rarity cardPool to choose from
		if (cardRarity >= 4) cardRarity = 3;
		std::vector<CardStats>& rarityPool = ::cardPool[cardRarity];

		// random float and normalise within size of card pool of corresponding rarity
		int index = (int)(AERandFloat() * (rarityPool.size()));
		if (index == rarityPool.size()) --index;
		card.info = ::cardPool[cardRarity][index];
	}

	// resets player stats modifiers
	void resetModifiers() {
		// reset player base modifiers
		cardBaseMod.hp = 0.f;
		cardBaseMod.dmg = 0.f;
		cardBaseMod.fireRate = 0.f;
		cardBaseMod.moveSpeed = 0.f;
		cardBaseMod.xp = 0.f;

		// reset player mult modifiers
		cardMultMod.hp = 1.f;
		cardMultMod.dmg = 1.f;
		cardMultMod.fireRate = 1.f;
		cardMultMod.moveSpeed = 1.f;
		cardMultMod.xp = 1.f;

		// reset passive upgrades
		upgradeFlag = UPGRADE_NONE;
		num_activeCards = base_activeCards;
		num_shopCards = base_shopCards;
		num_buyable = base_buyable;
	}

	// clears all cards, ready for next game
	void resetCards() {
		// clears shop (in case)
		if (!shopCards.empty()) shopCards.clear();
		// clears inventory and active cards
		if (!inventoryCards.empty()) inventoryCards.clear();
		if (!activeCards.empty()) activeCards.clear();
		resetModifiers();
	}

	// computes card effects
	void computeCardEffects() {
		// get the max hp before reseting
		float oldMaxHp = get_max_hp();
		// reset modifiers
		resetModifiers();

		// ACTIVE:	only counted if card in active deck
		// PASSIVE: only counted if card is in inventory deck

		// for each card in active deck
		for (Card& card : activeCards) {
			// for each active effect in each active card
			for (CardEffect& effect : card.info.active) {
				// pick which modifier to add to
				PlayerStatsModifier& mod = (effect.valuetype == "mult") ? cardMultMod : cardBaseMod;

				// figure out what data member to add to
				if	    (effect.type == "HP")         mod.hp += effect.value;
				else if (effect.type == "DMG")        mod.dmg += effect.value;
				else if (effect.type == "FIRE_RATE")  mod.fireRate += effect.value;
				else if (effect.type == "MOVE_SPEED") mod.moveSpeed += effect.value;
				else if (effect.type == "XP")         mod.xp += effect.value;
			} // endfor active effects
		} // endfor active cards

		// enforce maximums / minimums
		// fire rate mult must not go below 90% reduction
		if (cardMultMod.fireRate <= 0.1f) cardMultMod.fireRate = 0.1f;
		
		// for each card in inventory deck
		for (Card& card : inventoryCards) {
			for (CardEffect& effect : card.info.passive) {
				// increment corresponding passive upgrade
				if		(effect.id == "active_cards_up") num_activeCards++;
				else if (effect.id == "shop_cards_up")   num_shopCards++;
				else if (effect.id == "shop_buys_up")    num_buyable++;
				// flip corresponding upgrade flag
				else if (effect.id == "big_cannon")  upgradeFlag |= UPGRADE_BIG_CANNON;
				else if (effect.id == "cannon_180")  upgradeFlag |= UPGRADE_CANNON_180;
				else if (effect.id == "dual_cannon") upgradeFlag |= UPGRADE_DUAL_CANNON; // not compatible with prev upgrades
				else if (effect.id == "orbit")       upgradeFlag |= UPGRADE_ORBIT;
			} // endfor passive effects
		} // endfor inventory cards

		// CLAMP VALUES
		// if num_shopCards exceeds max
		if (num_shopCards   > max_shopCards)	num_shopCards = max_shopCards;
		if (num_activeCards > max_activeCards)	num_activeCards = max_activeCards;
		if (num_buyable		> buyable_max)		num_buyable = buyable_max;
		// if dual_cannon active, disable big_cannon and cannon_180
		if (upgradeFlag & UPGRADE_DUAL_CANNON) {
			if (upgradeFlag & UPGRADE_BIG_CANNON) upgradeFlag &= ~UPGRADE_BIG_CANNON;
			if (upgradeFlag & UPGRADE_CANNON_180) upgradeFlag &= ~UPGRADE_CANNON_180;
		}
		UpdateCurrentHpAfterCards(oldMaxHp); //calls debugxp to update hp
	} // endfunction

	// computes player stats
	void computePlayerStats() {
		//
	}

}
