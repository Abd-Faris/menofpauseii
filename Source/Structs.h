#pragma once
#include "AEEngine.h"

struct gfxtext {
	const char* text;
	f32 x, y, scale, r, g, b, a;
};

struct Point {
	f32 x, y;
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
}basl;

typedef struct Enemies {
	float pos_x;
	float pos_y;
	int xp;
	float scale;
	float rotation;
	bool alive = false;
	int hp;
};

//player base stats
struct PlayerStats {
	float baseHp;
	float baseDmg;
	float baseSpeed;
	float baseFireRate;
	float baseXpGain;
	//number of times it can upgrade
	int upgradeLevels[5];
};
