#pragma once
#include "AEEngine.h"

// To view, Right-Click Struct Name -> Go to Definition

// Graphics.h Structs
struct gfxtext;
struct gfxbutton;

// Card Enums and Structs
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
	// MEMBERS
	bool print{ true };
	AEVec2 pos; // world coords
	AEVec2 size{ 250, 350 };
	int type{}, val{}, rarity{ Rarity::COMMON };
	AEGfxVertexList* mesh{};

	// MEMBER FUNCTIONS

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

struct shape {
	f32 scale;			//scale
	f32 pos_x, pos_y;	// x and y positions
	f32 currentAngle;
	AEMtx33 transform;  // Final transformation matrix for rendering
};


struct bullets {
	f32 x, y;// x an y coordinates
	f32 directx, directy;// direction x and y
	bool active;
	f32 speed;
};


struct Enemies {
	AEVec2 pos;
	AEVec2 velocity;
	int xp;
	float scale;
	float rotation;
	bool alive = false;
	int hp;
};

struct PlayerStats {
	//player base stats
	float baseHp;
	float baseDmg;
	float baseSpeed;
	float baseFireRate;
	float baseXpGain;
	//how many times you can level up
	int upgradeLevels[5];
	//variables for xp logic
	float current_xp;
	float initial_xp;
	int player_level;
	//for skill points
	int skill_point;
	bool menu_open;
	//for hp stats
	float current_hp;
};

// Externs
extern PlayerStats player_init;
