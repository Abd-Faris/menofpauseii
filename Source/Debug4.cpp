#include "MasterHeader.h"

enum Rarity {
	COMMON = 0,
	RARE,
	EPIC,
	UNIQUE,
	NUM_OF_RARITIES
};

enum Type {
	HP = 0,
	DMG,
	FIRE_RATE,
	MOVE_SPEED,
	XP,
	NUM_OF_TYPES
};

struct Card {
	bool exists = false;
	AEGfxVertexList* mesh{};
	int type{}, val{}; // card stats
	f32 xpos{}, ypos{}; // card position (world coords)
	f32 sizex{ 250 }, sizey{ 350 }; // card size
	// lowest priority for implementation
	int rarity{ Rarity::COMMON };

	// generates new card and assigns to 
	void generateCard() {
		// randomise type
		type = static_cast<int>(AERandFloat() * Type::NUM_OF_TYPES);
		// alter percentage based on rarity
		switch (rarity) {
		case Rarity::COMMON: // 10-30%, range 20
			val = 10 + static_cast<int>(AERandFloat() * 20);
			break;
		default: break;
		};
	}
};

namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 1};
	// create texts
	gfxtext texts[NUM_OF_TEXTS] = {
		{"Card Screen", 0, 0, 1, 255, 255, 255, 255}
	};
	// create mesh for cards
	std::array<Card, 3>cardShop = { 0 };

	

	// mesh pointers
	void populateCardShop(std::array<Card,3>& shop) {
		int index{};
		for (Card &slot : shop) {
			// set mesh
			slot.mesh = Gfx::createRectMesh();
			// set card stats
			slot.generateCard();
			// set xpos ypos
			slot.xpos = -400 + (index++ * 400);
			// display card stats
			std::cout << "Type: " << slot.type << '\n'
					  << "Rarity: " << slot.rarity << '\n'
					  << "xpos: " << AERandFloat() << "\n\n";
		}
	}
}

void LoadDebug4() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void InitializeDebug4() {
	// intialises mesh pointers
	populateCardShop(cardShop);
}

void DrawDebug4() {
	//AEGfxStart();
	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	// object drawing settings
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetTransparency(1.f);
	// print text
	for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
		Graphics::printText(texts[i], boldPixels);
	}
	// print shop cards
	for (Card &indivCard : cardShop) {
		Gfx::printMesh(indivCard);
	}
	//AEGfxEnd();
}

void FreeDebug4() {
	// free font
	AEGfxDestroyFont(boldPixels);
	// free mesh
	for (Card &indivCard : cardShop) {
		AEGfxMeshFree(indivCard.mesh);
	}
}