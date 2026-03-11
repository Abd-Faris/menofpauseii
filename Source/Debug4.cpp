#pragma once
#include "MasterHeader.h"

// global variables
namespace {
	// declare font var
	s8 boldPixels;

	// define meshes
	AEGfxVertexList* rectMesh{};
	
	// graphic boxes
	AEGfxVertexList* bag, * shop, * desc, * cardSlots, * trash;
	u32 cbag{ 0xFFC77014 }, cshop{ 0xFFcccc00 }, cdesc{ 0xFF000000 }, ccardSlots{ 0xFFcccc00 }, ctrash{ 0xFFe62e00 };

	// card scales for diff vector arrays
	f32 CARD_SHOP_SCALE{ 8 };
	f32 ACTIVE_CARD_SCALE{ 3 };
	f32 INVENTORY_CARD_SCALE{ 3 };

	// num of cards available to be displayed to player
	int num_shopCards      { 3 };
	int num_activeCards    { 5 };
	int num_inventoryCards { 15 };

	// declare array for cards
	std::array<std::vector<Card>, 3>    allCards;	// vector for all cards
	// reference for easy ref
	std::vector<Card>& shopCards      = allCards[0];// reference to shop cards
	std::vector<Card>& activeCards    = allCards[1];// ref to active cards
	std::vector<Card>& inventoryCards = allCards[2];// reference to cards in bag
	// pointer to selected card
	Card* pSelectedCard{nullptr};

	// init shop texts
	std::vector<GfxText> shopTexts{
		{"INVENTORY", 0.5f, 0, 0, 0, 255, {575, 375}},
		{"SHOP", 0.8f, 0, 0, 0, 255, {-200, 195}},
		{"ACTIVE CARDS", 0.5f, 0, 0, 0, 255, {-125, -225}},
		{"X", 2.f, 0, 0, 0, 255, {-695, -300}},
		{"5/5", 0.5f, 0, 0, 0, 255, {-125, -375}},
		{"5/15", 0.5f, 0, 0, 0, 255, {575, -375}},
		{"DESCRIPTION GOES HERE", 0.5f, 255, 255, 255, 255, {-200, 325}}
	};

	// mesh pointers
	void populateCardShop(std::vector<Card>& shop) {
		int index{};
		for (Card &slot : shop) {
			// set mesh
			slot.mesh = Gfx::createRectMesh();
			// set card stats
			slot.generateCard();
			// set xpos ypos
			slot.pos = { -400.f + (index++ * 400.f), 0.f };
			// display card stats
			std::cout << "Type: " << slot.type << '\n'
					  << "Rarity: " << slot.rarity << '\n'
					  << "xpos: " << AERandFloat() << "\n\n";
		}
	}

	void initCardShop(std::vector<Card>& shop) {
		// create 3 cards for shop
		Card card{};
		for (int i{ 0 }; i < num_shopCards; ++i) {
			card.from = DECK::SHOP;
			card.generateCard();  // generate card stats
			shop.push_back(card); // push card into vector
		}
	}

	// Computes the home positions of an array of cards within a bounded range
	void computeXCardPositions(std::vector<Card>& arr, f32 start, f32 end, f32 y, f32 scale = 10, f32 maxGap = 100.f) {
		// EDGECASE: if array empty, return
		if (arr.empty()) return;

		// SET NUM OF DISPLAY CARDS
		int displayCards = arr.size();
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

	// TO BE PHASED OUT IN FAVOUR OF HARD CODED POSITIONS
	void computeYCardPositions(std::vector<Card>& arr, f32 start, f32 end, f32 x, f32 scale = 10) {
		// EDGECASE: if array empty, return
		if (arr.empty()) return;

		// SET NUM OF DISPLAY CARDS
		int displayCards = arr.size();
		// set range for shop INCL PADDING ON EITHER SIDE
		start += (arr[0].size.y / 2) * scale;
		end -= (arr[0].size.y / 2) * scale;
		// finds range
		f32 range = end - start;

		// normalise card position within range
		for (int i{ 0 }; i < displayCards; ++i) {
			// EDGECASE: ONLY compute if more than 1 card
			if (arr.size() <= 1) {
				arr[i].pos.y = (0.5f * range); // centers one card
				arr[i].pos.x = x;
				// assigns homepos to computed pos
				arr[i].homepos = arr[i].pos;
				continue;
			}
			// normalise card into range
			f32 normalized = static_cast<f32>(i) / (displayCards - 1);
			// denormalise from range into world coords
			arr[i].pos.y = start + (normalized * range);
			arr[i].pos.x = x;
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
		
		//// assign inventory cards
		//int xIndex{}, yIndex{};
		//for (Card& card : inventoryCards) {
		//	// if run out of defined grid positions, break
		//	if (yIndex >= ypos.size()) break;
		//	// if new row, increment y index
		//	if (xIndex == 3) ++yIndex;
		//	// normalise xIndex within [0,3)
		//	xIndex = xIndex % 3;
		//	// set homepos
		//	card.homepos = { xpos[xIndex++], ypos[yIndex] };
		//	card.pos = card.homepos;
		//}

		for (int i = 0; i < inventoryCards.size(); ++i) {
			int row = i / 3;
			int col = i % 3;

			// if run out of defined grid positions, break
			if (row >= ypos.size()) break;

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

	void removeFromSource(Card& card) {
		// determine and create pointer to sourceDeck
		std::vector<Card>* sourceDeck = &shopCards; // default shopCards
		switch (card.from) {
		case DECK::ACTIVE: sourceDeck = &activeCards; break;
		case DECK::BAG:    sourceDeck = &inventoryCards; break;
		}

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

	void updateCardPosition() {
		// get selected card
		Card &card = *pSelectedCard;

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
			pSelectedCard  = nullptr;
		}
	}

	void checkCardCollision() {
		// get cursor position
		AEVec2 cursorpos{};
		Comp::getCursorPos(cursorpos);

		// for every card in ALL arrays, check for a hit
		// if hit, assign card to ptr IF left button is triggered
		for (Card &card : shopCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				if (AEInputCheckTriggered(AEVK_LBUTTON)) pSelectedCard = &card;
				return;
			}
		}
		for (Card &card : activeCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				if (AEInputCheckTriggered(AEVK_LBUTTON)) pSelectedCard = &card;
				return;
			}
		}
		for (Card &card : inventoryCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				if (AEInputCheckTriggered(AEVK_LBUTTON)) pSelectedCard = &card;
				return;
			}
		}
		// else return no hit at all
		return;
	}

	void drawPrompts() {
		Card& card = *pSelectedCard;
		// check collision with non-shop inventories

		// bag deck prompt
		if (Comp::collisionPointRect(card.pos, { 575, 0 }, { 350,800 })) {
			// dont display if selectedCard was from the bag deck
			if ((card.from == DECK::BAG)) return;
			// display bag deck prompt
			Gfx::printMesh(bag, { 575,0 }, { 350,800 });
		}
		// active deck prompt
		else if (Comp::collisionPointRect(card.pos, { -125, -300 }, { 950, 200 })) {
			// dont display if selectedCard was from the active deck
			if ((card.from == DECK::ACTIVE)) return;
			// display active deck prompt
			Gfx::printMesh(cardSlots, { -125, -300 }, { 950, 200 });
		}
	}

	void drawSelectedCard() {
		// determine card size based on card type
		Card& card = *pSelectedCard;
		f32 scale{};
		// determine scale
		switch (card.from) {
		case DECK::SHOP:   scale = CARD_SHOP_SCALE; break;
		case DECK::ACTIVE: scale = ACTIVE_CARD_SCALE; break;
		case DECK::BAG:    scale = INVENTORY_CARD_SCALE; break;
		}
		Gfx::printMesh(rectMesh, *pSelectedCard, scale);
	}
}

void LoadCardShop() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void InitializeCardShop() {
	// create container meshes
	rectMesh = Gfx::createRectMesh();
	bag = Gfx::createRectMesh("center", cbag);
	shop = Gfx::createRectMesh("center", cshop);
	desc = Gfx::createRectMesh("center", cdesc);
	cardSlots = Gfx::createRectMesh("center", ccardSlots);
	trash = Gfx::createRectMesh("center", ctrash);

	// [ DO NOT TOUCH ] Reserve space in memory for different card categories
	shopCards.reserve(4);       // max cards allowed
	activeCards.reserve(10);    // max cards allowed
	inventoryCards.reserve(15); // max cards allowed

	// initializes shop cards
	initCardShop(shopCards);
	computeCardHomePos();
}

void UpdateCardShop() {
	// if theres a card selected, update selected card position
	if (pSelectedCard) {
		updateCardPosition();
	}
	// else check collision for ALL cards
	else {
		checkCardCollision();
	} // endif
}

// -600 -200 200
void DrawCardShop() {
	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	// object drawing settings
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);

	// CONTAINERS
	Gfx::printMesh(bag, { 575,0 }, { 350,800 });
	Gfx::printMesh(shop, { -200, 40 }, { 1100, 380 });
	Gfx::printMesh(desc, { -200, 325 }, { 1100, 150 });
	Gfx::printMesh(cardSlots, { -125, -300 }, { 950, 200 });
	Gfx::printMesh(trash, { -700, -300 }, { 100, 100 });

	// TEXTS
	for (GfxText text : shopTexts) {
		Gfx::printText(text, boldPixels);
	}
	
	// CARDS
	// print shop cards
	for (Card &indivCard : shopCards) {
		Gfx::printMesh(rectMesh, indivCard, CARD_SHOP_SCALE);
	}
	for (Card& indivCard : activeCards) {
		Gfx::printMesh(rectMesh, indivCard, ACTIVE_CARD_SCALE);
	}
	for (Card& indivCard : inventoryCards) {
		Gfx::printMesh(rectMesh, indivCard, INVENTORY_CARD_SCALE);
	}
	// if theres a card selected, draw prompts
	if (pSelectedCard) {
		drawPrompts();
		drawSelectedCard(); // ensure selected card always on top
	}
}

void FreeCardShop() {
	AEGfxMeshFree(rectMesh); // free mesh
	shopCards.clear(); // clear shop array
}

void UnloadCardShop() {
	// unload assets
	AEGfxDestroyFont(boldPixels); // font
}
