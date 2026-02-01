#pragma once
#include "AEEngine.h"

// To view, Right-Click Struct Name -> Go to Definition

// Graphics.h Structs
struct gfxtext;
struct gfxbutton;

// Card Enums and Structs
enum Rarity;
enum Type;
struct Card;

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
	void reset_player_hp(float max_hp) {
		current_hp = max_hp;
	}
};
