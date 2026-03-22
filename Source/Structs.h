#pragma once
#include "AEEngine.h"

// To view, Right-Click Struct Name -> Go to Definition

// Graphics.h Structs
struct GfxText;
struct GfxButton;

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

enum struct DECK {
	SHOP,
	ACTIVE,
	BAG,
	TRASH
};

enum EnemyType {
	PASSIVE = 0,
	ATTACK,
	SHOOTER
};
enum BossType {
	BOSS1,
	BOSS2,
	BOSS3,
	BOSS4,
	numofBosses
};

// player upgrade flag (set to u32 so can have 32 possible flags)
enum PlayerUpgradeFlags : u32 {
	UPGRADE_NONE		= 0,
	UPGRADE_DUAL_CANNON = 1 << 0,  // 0001
	UPGRADE_BIG_CANNON  = 1 << 1,  // 0010
	UPGRADE_CANNON_180  = 1 << 2,  // 0100
	UPGRADE_ORBIT		= 1 << 3,  // 1000
};

struct AABB {
	AEVec2 max, min;
};

// individual card effects in a cardstat
struct CardEffect {
	std::string id;
	std::string type;
	std::string desc;
	std::string valuetype;
	f32 value{};
};

// card information
struct CardStats {
	std::string ID;
	int rarity{};
	int row{}, col{}; // for spritesheet coords
	std::vector<CardEffect> active;
	std::vector<CardEffect> passive;
};

// card object instance
struct Card {
	// MEMBERS
	AEVec2 pos; // dynamic world coords
	AEVec2 homepos; // anchored world coords
	AEVec2 size{ 25, 35 };
	AEGfxVertexList* mesh{};
	AABB boundingBox{}; // collision bounding box
	CardStats info;
	DECK from{}; // to be updated every time card is shifted
};

// modifier based on cards
struct PlayerStatsModifier {
	f32 hp{};
	f32 dmg{};
	f32 fireRate{};
	f32 moveSpeed{};
	f32 xp{};
};

struct shape {
	f32 scale;			//scale
	f32 pos_x, pos_y;	// x and y positions
	f32 currentAngle;
	AEMtx33 transform;  // Final transformation matrix for rendering
	f32 barrelCount;
};


struct bullets {
	f32 x, y;// x an y coordinates
	f32 directx, directy;// direction x and y
	bool active;
	f32 speed;
};

struct EnemyTypeInfo {
	int id;
	int cost;
	int baseWeight;
	float growthRate;
	int minWave;
	//int maxWave;
};

struct Enemies {
	AEVec2 pos = { 0, 0 };
	AEVec2 velocity = { 0, 0 };
	int xp{};
	float scale{};
	float rotation{};
	bool alive = false;
	int hp{};
	int maxhp{};
	int enemtype = PASSIVE;
	bool detect = false;
	float cooldown{};
};

enum class BossState { IDLE, TELEGRAPHING, LUNGING, COOLDOWN };
enum class Boss3Attack { NONE, SPIRAL, AIMED, GUNS, LASER };
struct Boss {
	AEVec2 pos = {0, 0};
	AEVec2 velocity = { 0, 0 };
	int xp{};
	float scale{};
	float rotation{};
	bool alive = false;
	int hp{};
	int maxhp{};
	int bosstype = BOSS1;
	bool detect = false;

	// State machine
	BossState state = BossState::IDLE;
	float stateTimer = 0.f;

	// BOSS1 lunge
	AEVec2 lungeDirection = { 0, 0 };
	float lungeSpeed = 1400.f;

	// Shared tuning
	float chaseSpeed = 150.f;
	float idleDuration = 1.5f;
	float telegraphDuration = 0.6f;
	float lungeDuration = 0.25f;
	float cooldownDuration = 0.8f;

	// Shooting
	float shootTimer = 0.f;
	int   bulletCount = 8;
	bool  hasShot = false;

	// BOSS2
	int minionCount = 6;

	// BOSS3
	Boss3Attack currentAttack = Boss3Attack::NONE;
	float attackTimer = 0.f;
	float spiralAngle = 0.f;

	// BOSS4
	float laserAngle = 0.f;  // current laser direction
	float laserTargetAngle = 0.f;  // angle locked onto player at telegraph
	float laserSweepSpeed = 0.8f; // radians per second sweep
	float laserTimer = 0.f;
	bool  laserActive = false;
	float gunFireTimer = 0.f;
	float gunFireRate = 0.3f; // seconds between gun shots
	float gunAngle = 0.f; // independent of boss body rotation
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

struct Explosion {
	AEVec2 pos;
	float timer;
	int currentFrame;
	bool active;
	float scale;
};

// -- Bullet Pool --
struct BulletObj {
	float posX, posY;
	float directionX, directionY;
	float speed;
	float size;
	bool isActive;
	float damagemul;
};

struct SmokeParticle {
	float posX, posY;
	float size;
	bool isActive;
};

// Externs
extern PlayerStats player_init;
extern SmokeParticle smokes[100];


