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
	int num_inventoryCards { 5 };

	// declare array for cards
	std::array<std::vector<Card>, 3>    allCards;	// vector for all cards
	// reference for easy ref
	std::vector<Card>& shopCards      = allCards[0];// reference to shop cards
	std::vector<Card>& activeCards    = allCards[1];// ref to active cards
	std::vector<Card>& inventoryCards = allCards[2];// reference to cards in bag
	// boolean for selected card
	bool cardSelected{};

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

	//GfxText bagtext{ {575, 375}, 0.5, "INVENTORY" };
	//GfxText shoptext{ {-200, 195}, 0.8, "SHOP" };
	//GfxText cardSlotstext{ {-125, -225}, 0.5, "ACTIVE CARDS" };
	//GfxText trashtext{ {-695, -300}, 2, "X" };
	//GfxText activeNumtext{ {-125, -375}, 0.5, "5/5" };
	//GfxText inventoryNumtext{ {575, -375}, 0.5, "5/15" };
	//
	//GfxText desctext{ {-200, 325}, 1,"DESCRIPTION GOES HERE", 255, 255, 255, 255 };


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
			card.generateCard();  // generate card stats
			shop.push_back(card); // push card into vector
		}
	}

	// FOR TESTING ONLY DELETE LATER
	void initActiveCards(std::vector<Card>& shop) {
		// create 3 cards for shop
		Card card{};
		for (int i{ 0 }; i < num_activeCards; ++i) {
			card.generateCard();  // generate card stats
			shop.push_back(card); // push card into vector
		}
	}

	// FOR TESTING ONLY DELETE LATER
	void initInventoryCards(std::vector<Card>& shop) {
		// create 3 cards for shop
		Card card{};
		for (int i{ 0 }; i < num_inventoryCards; ++i) {
			card.generateCard();  // generate card stats
			shop.push_back(card); // push card into vector
		}
	}

	void computeXCardPositions(std::vector<Card>& arr, f32 start, f32 end, f32 y, f32 scale = 10) {
		// EDGECASE: if array empty, return
		if (arr.empty()) return;

		// SET NUM OF DISPLAY CARDS
		int displayCards = arr.size();
		// set range for shop INCL PADDING ON EITHER SIDE
		start += (arr[0].size.x / 2) * scale;
		end   -= (arr[0].size.x / 2) * scale;
		f32 totalRange = end - start;

		f32 maxGap = 200.0f; // Set max padding
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
				arr[i].pos.x = (0.5f*range); // centers one card
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
	}

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
	}

	// only called if theres a card selected (via updateCardPosition)
	Card& findSelectedCard() {
		// Check shop
		for (Card& card : shopCards) {
			if (card.selected) return card;
		}
		// Check active
		for (Card& card : activeCards) {
			if (card.selected) return card;
		}
		// Check inventory
		for (Card& card : inventoryCards) {
			if (card.selected) return card;
		}
	}

	void updateCardPosition() {
		// get selected card
		Card &card = findSelectedCard();

		// update card position
		AEVec2 deltacursorpos{};
		Comp::getDeltaCursorPos(deltacursorpos);
		card.pos.x += deltacursorpos.x;
		card.pos.y -= deltacursorpos.y;

		// if user NOT holding left click
		if (!AEInputCheckCurr(AEVK_LBUTTON)) {
			// deselect card
			card.selected = false;
			// reset to home pos
			card.pos = card.homepos;
			// set global bool to false
			cardSelected = false;
		}
	}

	bool checkCardCollision() {
		// get cursor position
		AEVec2 cursorpos{};
		Comp::getCursorPos(cursorpos);

		// for every card in ALL arrays, check for a hit
		// if hit, set selected boolean to true and return hit
		for (Card &card : shopCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				card.selected = true;
				return 1;
			}
		}
		for (Card &card : activeCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				card.selected = true;
				return 1;
			}
		}
		for (Card &card : inventoryCards) {
			if (Comp::collisionPointRect(cursorpos, card.boundingBox)) {
				card.selected = true;
				return 1;
			}
		}
		// else return no hit
		return 0;
	}
}

void LoadDebug4() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void InitializeDebug4() {
	// create container meshes
	rectMesh = Gfx::createRectMesh();
	bag = Gfx::createRectMesh("center", cbag);
	shop = Gfx::createRectMesh("center", cshop);
	desc = Gfx::createRectMesh("center", cdesc);
	cardSlots = Gfx::createRectMesh("center", ccardSlots);
	trash = Gfx::createRectMesh("center", ctrash);

	// [ DO NOT TOUCH ] Reserve space in memory for different card categories
	shopCards.reserve(4);
	activeCards.reserve(8);
	inventoryCards.reserve(16);

	// initializes shop cards
	initCardShop(shopCards);
	computeXCardPositions(shopCards, -700, 300, 20, CARD_SHOP_SCALE);
	for (Card &card : shopCards) {
		Comp::computeBoundingBox(card.boundingBox, card.homepos, card.size, CARD_SHOP_SCALE);
	}
	// set selected card boolean to false
	cardSelected = false;

	// [ FOR DEMO ONLY. DELETE LATER ] init other cards
	initActiveCards(activeCards);
	computeXCardPositions(activeCards, -550, 300, -300, ACTIVE_CARD_SCALE);
	initInventoryCards(inventoryCards);
	computeYCardPositions(inventoryCards, -335, 340, 575, INVENTORY_CARD_SCALE);
}

void UpdateDebug4() {
	// if theres a card selected, update selected card position
	if (cardSelected) {
		updateCardPosition();
	}
	// else if user clicked left click
	else if (AEInputCheckTriggered(AEVK_LBUTTON)) {
		// check for collision for ALL cards
		// if theres a hit, set bool to true
		if (checkCardCollision()) cardSelected = true;
	} 
	// else set selected card bool to false
	else cardSelected = false;
	// endif
}

void DrawDebug4() {
	//AEGfxStart();
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
}

void FreeDebug4() {
	// free font
	AEGfxDestroyFont(boldPixels);
	// free mesh
	AEGfxMeshFree(rectMesh);
	// clear shop array
	shopCards.clear();
	activeCards.clear();
	inventoryCards.clear();
}