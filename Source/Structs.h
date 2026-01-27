#pragma once
#include "AEEngine.h"

struct gfxtext {
	const char* text;
	f32 x, y, scale, r, g, b, a;
};

// OLD: USE native AEVec2 instead
//struct Point {
//	f32 x, y;
//};

struct shape {
	f32 scale;			//scale
	f32 pos_x, pos_y;	// x and y positions
	f32 currentAngle;
	AEMtx33 transform;  // Final transformation matrix for rendering
};

typedef struct Enemies {
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
