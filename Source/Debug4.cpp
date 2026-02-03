#pragma once
#include "MasterHeader.h"

// global variables
namespace {
	s8 boldPixels;
	// create texts
	std::vector<gfxtext> texts{
		{"Card Screen", 0, 0, 1, 255, 255, 255, 255}
	};

	// define meshes
	AEGfxVertexList* rectMesh{};
	
	// graphic boxes
	AEGfxVertexList* bag, * shop, * desc, * cardSlots, * trash;
	u32 cbag{ 0xFFC77014 }, cshop{ 0xFFcccc00 }, cdesc{ 0xFF000000 }, ccardSlots{ 0xFFcccc00 }, ctrash{ 0xFFe62e00 };

	// card scales for diff vector arrays
	f32 CARD_SHOP_SCALE{ 8 };
	f32 ACTIVE_CARD_SCALE{ 3 };

	// num of cards available to be displayed to player
	int num_shopCards      { 3 };
	int num_activeCards    { 5 };
	int num_inventoryCards{ 16 };

	// declare array for cards
	std::array<std::vector<Card>, 3>    allCards;	// vector for all cards
	// reference for easy ref
	std::vector<Card>& shopCards      = allCards[0];// reference to shop cards
	std::vector<Card>& activeCards    = allCards[1];// ref to active cards
	std::vector<Card>& inventoryCards = allCards[2];// reference to cards in bag

	// create mesh for cards
	//std::array<Card, 3>cardShop = { 0 };

	

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
		Card card{1}; // 1 for print bool in Card struct
		for (int i{ 0 }; i < num_shopCards; ++i) {
			card.generateCard();  // generate card stats
			shop.push_back(card); // push card into vector
		}
	}

	// FOR TESTING ONLY DELETE LATER
	void initActiveCards(std::vector<Card>& shop) {
		// create 3 cards for shop
		Card card{ 1 }; // 1 for print bool in Card struct
		for (int i{ 0 }; i < num_activeCards; ++i) {
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
		// finds range
		f32 range = end - start;

		// normalise card position within range
		for (int i{ 0 }; i < displayCards; ++i) {
			// EDGECASE: ONLY compute if more than 1 card
			if (arr.size() <= 1) {
				arr[i].pos.x = (0.5f*range); // centers one card
				arr[i].pos.y = y;
				continue;
			}
			// normalise card into range
			f32 normalized = static_cast<f32>(i) / (displayCards - 1);
			// denormalise from range into world coords
			arr[i].pos.x = start + (normalized * range);
			arr[i].pos.y = y;
		}
	}
}

void LoadDebug4() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void InitializeDebug4() {
	// create mesh
	rectMesh = Gfx::createRectMesh();
	bag = Gfx::createRectMesh("center", cbag);
	shop = Gfx::createRectMesh("center", cshop);
	desc = Gfx::createRectMesh("center", cdesc);
	cardSlots = Gfx::createRectMesh("center", ccardSlots);
	trash = Gfx::createRectMesh("center", ctrash);

	// reserve space in memory for different card categories
	shopCards.reserve(4);
	activeCards.reserve(8);
	inventoryCards.reserve(16);

	// intialises shop cards
	initCardShop(shopCards);
	computeXCardPositions(shopCards, -700, 300, 20, CARD_SHOP_SCALE);

	// init other cards [ FOR DEMO ONLY ]
	initActiveCards(activeCards);
	computeXCardPositions(activeCards, -500, 300, -300, ACTIVE_CARD_SCALE);
}

void UpdateDebug4() {
	// compute positions of cards
}

void DrawDebug4() {
	//AEGfxStart();
	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	// object drawing settings
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);
	//AEGfxVertexList* bag, * shop, * desc, * cardSlots, * trash;
	// print static boxes
	Gfx::printMesh(bag, { 575,0 }, { 350,800 });
	Gfx::printMesh(shop, { -200, 40 }, { 1100, 380 });
	Gfx::printMesh(desc, { -200, 325 }, { 1100, 150 });
	Gfx::printMesh(cardSlots, { -125, -300 }, { 950, 200 });
	Gfx::printMesh(trash, { -700, -300 }, { 100, 100 });
	// print text
	for (gfxtext g : texts) {
		Gfx::printText(g, boldPixels);
	}
	// print shop cards
	for (Card &indivCard : shopCards) {
		Gfx::printMesh(rectMesh, indivCard, CARD_SHOP_SCALE);
	}
	for (Card& indivCard : activeCards) {
		Gfx::printMesh(rectMesh, indivCard, ACTIVE_CARD_SCALE);
	}
	//AEGfxEnd();
}

void FreeDebug4() {
	// free font
	AEGfxDestroyFont(boldPixels);
	// free mesh
	AEGfxMeshFree(rectMesh);
	// clear shop array
	shopCards.clear();
	activeCards.clear();
}