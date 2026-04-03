// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Structs.h
// Authors: [Men of Pause II]
// Brief:   Central definitions for all enums, structs, and forward declarations
//          shared across the project. To inspect a type, right-click its name
//          and select Go to Definition.
// ------------------------------------------------------------------------- //

#pragma once
#include "AEEngine.h"

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

struct GfxText;    // defined in Graphics.h
struct GfxButton;  // defined in Graphics.h

// =============================================================================
// ENUMS
// =============================================================================

// ---- Card rarity tiers ----
enum Rarity {
    COMMON = 0,
    RARE,
    EPIC,
    UNIQUE,
    NUM_OF_RARITIES  // sentinel — keep last
};

// ---- Card effect stat types ----
enum Type {
    HP = 0,
    DMG,
    FIRE_RATE,
    MOVE_SPEED,
    XP,
    NUM_OF_TYPES  // sentinel — keep last
};

// ---- Deck a card currently belongs to ----
enum struct DECK {
    SHOP,    // offered in the shop this round
    ACTIVE,  // equipped in the active slot
    BAG,     // stored in the player's inventory
    TRASH    // discarded (transient — card is removed immediately)
};

// ---- Enemy behaviour archetypes ----
enum EnemyType {
    PASSIVE = 0,  // wanders, does not attack
    ATTACK,       // chases and melee-attacks the player
    SHOOTER       // keeps range and fires projectiles
};

// ---- Boss variants ----
enum BossType {
    BOSS1,
    BOSS2,
    BOSS3,
    BOSS4,
    numofBosses  // sentinel — keep last
};

// ---- SFX type identifiers (index into sfxLibrary) ----
enum SFX_ID {
    SFX_ENEMY_DEATH,
    SFX_ENEMY_SHOOT,
    SFX_ENEMY_HIT,
    SFX_PLAYER_SHOOT,
    SFX_PLAYER_HIT,
    SFX_UI_BUTTON_HOVER,
    SFX_UI_BUTTON_SELECT,
    SFX_UI_CARD_HOVER,
    SFX_UI_CARD_PICKUP,
    SFX_UI_CARD_PLACE,
    SFX_UI_CARD_TRASH,
    SFX_COUNT  // sentinel — equals the total number of SFX types
};

// ---- Passive weapon upgrade bitmask flags ----
// Stored as u32 so up to 32 independent upgrades can be tracked simultaneously.
enum PlayerUpgradeFlags : u32 {
    UPGRADE_NONE = 0,
    UPGRADE_DUAL_CANNON = 1 << 0,  // 0001 — fires two parallel shots (incompatible with BIG / 180)
    UPGRADE_BIG_CANNON = 1 << 1,  // 0010 — fires a larger, slower projectile
    UPGRADE_CANNON_180 = 1 << 2,  // 0100 — fires an additional shot directly behind the player
    UPGRADE_ORBIT = 1 << 3,  // 1000 — spawns an orbiting shield projectile
};

// ---- Boss AI state machine states ----
enum class BossState {
    IDLE,           // standing still before committing to an attack
    TELEGRAPHING,   // winding up — visual warning before the attack fires
    LUNGING,        // executing the lunge movement
    COOLDOWN        // recovery period before the next action
};

// ---- Boss 3 special attack variants ----
enum class Boss3Attack {
    NONE,    // no attack active
    SPIRAL,  // rotating spiral bullet pattern
    AIMED,   // burst aimed directly at the player
    GUNS,    // rapid-fire gun barrage
    LASER    // sweeping laser beam
};

// =============================================================================
// STRUCTS
// =============================================================================

// ---- Axis-aligned bounding box used for collision detection ----
struct AABB {
    AEVec2 max;  // top-right corner in world space
    AEVec2 min;  // bottom-left corner in world space
};

// ---- A single effect entry within a card's active or passive list ----
struct CardEffect {
    std::string id;         // unique effect identifier (matches JSON "id" field)
    std::string type;       // stat targeted: "HP", "DMG", "FIRE_RATE", "MOVE_SPEED", "XP"
    std::string desc;       // human-readable description shown in the shop UI
    std::string valuetype;  // "base" for flat bonus, "mult" for multiplier
    f32 value{};            // magnitude of the effect
};

// ---- Static data for a card type loaded from cards.json ----
struct CardStats {
    std::string ID;                    // unique card identifier
    int rarity{};                      // Rarity enum value
    int row{}, col{};                  // spritesheet grid coordinates
    std::vector<CardEffect> active;    // effects applied when card is in the active deck
    std::vector<CardEffect> passive;   // effects applied when card is in the inventory (bag)
};

// ---- Runtime instance of a card in one of the three decks ----
struct Card {
    AEVec2 pos;                         // current world position (moves while dragged)
    AEVec2 homepos;                     // anchored rest position (snaps back on drop)
    AEVec2 size{ 25, 35 };             // card dimensions in world units
    AEGfxVertexList* mesh{};            // per-card UV mesh (may be null if using shared mesh)
    AABB boundingBox{};                 // precomputed collision box for hit-testing
    CardStats info;                     // static card data (type, effects, sprite coords)
    DECK from{};                        // which deck this card currently belongs to
    bool hovered{ false };              // true on the frame the cursor first enters the card
};

// ---- Additive and multiplicative stat modifiers driven by equipped cards ----
struct PlayerStatsModifier {
    f32 hp{};         // flat HP bonus (or multiplier when used as cardMultMod)
    f32 dmg{};        // flat damage bonus
    f32 fireRate{};   // flat fire-rate reduction (or multiplier — lower = faster)
    f32 moveSpeed{};  // flat move-speed bonus
    f32 xp{};         // flat XP gain bonus
};

// ---- Player ship shape and transform state ----
struct shape {
    f32    scale;                // uniform render scale
    f32    pos_x, pos_y;        // world position
    f32    currentAngle;         // current facing angle in radians
    AEMtx33 transform;           // final concatenated transformation matrix for rendering
    f32    barrelCount;          // number of active cannon barrels
};

// ---- Player bullet instance ----
struct bullets {
    f32  x, y;           // current world position
    f32  directx, directy;  // normalised direction vector
    bool active;         // false = slot is available in the pool
    f32  speed;          // units per second
};

// ---- Enemy type metadata loaded from data tables ----
struct EnemyTypeInfo {
    int   id;          // EnemyType enum value
    int   cost;        // wave budget cost to spawn this enemy
    int   baseWeight;  // base spawn weight (higher = more likely)
    float growthRate;  // how quickly the spawn weight scales with wave number
    int   minWave;     // earliest wave this enemy type can appear
    int   maxWave;     // latest wave this enemy type can appear (-1 = no limit)
};

// ---- Runtime enemy instance ----
struct Enemies {
    AEVec2 pos = { 0, 0 };  // current world position
    AEVec2 velocity = { 0, 0 };  // current movement velocity
    int    xp{};                  // XP awarded to the player on death
    float  scale{};               // uniform render scale
    float  rotation{};            // current facing angle in radians
    bool   alive = false;      // false = slot is available in the enemy pool
    int    hp{};                  // current hit points
    int    maxhp{};               // maximum hit points (used to compute health bar fill)
    int    enemtype = PASSIVE;    // EnemyType — determines AI behaviour
    bool   detect = false;      // true once the player enters this enemy's detection range
    float  cooldown{};            // time remaining before this enemy can act again
};

// ---- Runtime boss instance ----
struct Boss {
    // ---- Core ----
    AEVec2 pos = { 0, 0 };
    AEVec2 velocity = { 0, 0 };
    int    xp{};
    float  scale{};
    float  rotation{};
    bool   alive = false;
    int    hp{};
    int    maxhp{};
    int    bosstype = BOSS1;
    bool   detect = false;

    // ---- State machine ----
    BossState state = BossState::IDLE;
    float     stateTimer = 0.f;  // time spent in the current state

    // ---- Boss 1 — lunge attack ----
    AEVec2 lungeDirection = { 0, 0 };
    float  lungeSpeed = 1400.f;

    // ---- Shared tuning parameters ----
    float chaseSpeed = 150.f;
    float idleDuration = 1.5f;
    float telegraphDuration = 0.6f;
    float lungeDuration = 0.25f;
    float cooldownDuration = 0.8f;
    bool  lungehit = false;  // true if the lunge has already struck the player this cycle

    // ---- Shared shooting ----
    float shootTimer = 0.f;
    int   bulletCount = 8;
    bool  hasShot = false;  // prevents firing more than once per attack phase

    // ---- Boss 2 — minion spawning ----
    int minionCount = 6;  // number of minions summoned per spawn wave

    // ---- Boss 3 — multi-phase attacks ----
    Boss3Attack currentAttack = Boss3Attack::NONE;
    float       attackTimer = 0.f;   // time spent in the current attack phase
    float       spiralAngle = 0.f;   // current rotation of the spiral bullet pattern

    // ---- Boss 4 — laser and gun barrage ----
    float laserAngle = 0.f;   // current sweeping laser direction (radians)
    float laserTargetAngle = 0.f;   // player angle locked at the start of the telegraph
    float laserSweepSpeed = 0.8f;  // sweep rate in radians per second
    float laserTimer = 0.f;   // time spent in the current laser phase
    bool  laserActive = false; // true while the laser beam is dealing damage
    float gunFireTimer = 0.f;   // time since the last gun shot
    float gunFireRate = 0.3f;  // seconds between gun shots
    float gunAngle = 0.f;   // gun facing angle, independent of body rotation
};

// ---- Player statistics and progression state ----
struct PlayerStats {
    // ---- Base stats (before card and upgrade modifiers) ----
    float baseHp;
    float baseDmg;
    float baseSpeed;
    float baseFireRate;
    float baseXpGain;

    // ---- Skill-point upgrade levels (one per stat, capped at MAX_UPGRADE_LEVEL) ----
    int upgradeLevels[5];

    // ---- XP and levelling ----
    float current_xp;    // XP accumulated since the last level-up
    float initial_xp;    // XP at the start of the current level (unused at runtime; reserved)
    int   player_level;  // current player level

    // ---- Skill point menu ----
    int  skill_point;  // unspent skill points available to allocate
    bool menu_open;    // true while the upgrade menu is displayed

    // ---- Runtime HP ----
    float current_hp;  // current hit points (clamped to [1, max HP])
};

// ---- Explosion animation instance ----
struct Explosion {
    AEVec2 pos;           // world position of the explosion centre
    float  timer;         // time elapsed since the explosion started
    int    currentFrame;  // current spritesheet frame index
    bool   active;        // false = slot is available in the explosion pool
    float  scale;         // uniform render scale
};

// ---- Short-lived spark particle emitted on bullet impact ----
struct BulletSpark {
    float posX, posY;      // world position
    float dirX, dirY;      // normalised movement direction
    float speed;           // units per second
    float size;            // render size
    float lifetime;        // time remaining before the spark disappears
    float maxLifetime;     // total lifespan (used to compute fade alpha)
    bool  isActive;        // false = slot is available in the spark pool
};

// ---- Pooled player or enemy bullet ----
struct BulletObj {
    float posX, posY;          // current world position
    float directionX, directionY;  // normalised direction vector
    float speed;               // units per second
    float size;                // collision and render size
    bool  isActive;            // false = slot is available in the bullet pool
    float damagemul;           // damage multiplier applied on hit (1.0 = base damage)
};

// ---- Smoke particle emitted by the player ship ----
struct SmokeParticle {
    float posX, posY;  // world position
    float size;        // render size
    bool  isActive;    // false = slot is available in the smoke pool
};

// ---- Short-lived splash particle emitted on enemy death or impact ----
struct SplashParticle {
    float x, y;          // world position
    float velX, velY;    // movement velocity (units per second)
    float life, maxLife; // time remaining and total lifespan (used to compute fade alpha)
    float scale;         // render size
    bool  active;        // false = slot is available in the splash pool
};

// =============================================================================
// EXTERNS
// =============================================================================

extern PlayerStats  player_init;       // player stats and progression state (defined in DebugXP.cpp)
extern SmokeParticle smokes[100];      // smoke particle pool (defined in Game.cpp)