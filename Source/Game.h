// -----------------------------Gloomy's Revenge---------------------------- //
// File:	Game.h
// Authors:	[Men of Pause II]
// Brief:	Central configuration for tank stats, enemy balancing, and boss logic.
// ------------------------------------------------------------------------- //

#pragma once
#include <array>

// =============================================================================
// CONFIGURATION & VARIABLES
// =============================================================================

namespace GameConfig {

    // -- Math Constants --
    const float MOUSE_JITTER_THRESHOLD = 1.0f; // Minimum mouse movement to register a turn

    // -- World/Engine Settings --
    const int   MAX_BULLETS_COUNT = 1000;      // Capacity of the global projectile pool
    const int   MAX_ENEMIES_COUNT = 50;        // Max concurrent standard enemies
    const float OFF_SCREEN_COORD = 10000.0f;   // Dumping ground for inactive objects
    const float DESPAWN_DISTANCE_SQ = 2500.0f * 2500.0f; // Squared distance for cleanup

    // -- Player Tank Dimensions --
    namespace Tank {
        const float SCALE = 50.0f;

        // Main Body
        const float BODY_WIDTH = 60.0f;
        const float BODY_HEIGHT = 80.0f;

        // Tracks
        const float TRACK_WIDTH = 20.0f;
        const float TRACK_HEIGHT = 90.0f;
        const float TRACK_OFFSET_X = 35.0f;

        // Turret
        const float TURRET_SIZE = 40.0f;

        // Barrel
        const float BARREL_LENGTH = 120.0f;
        const float BARREL_WIDTH = 30.0f;
        const float BARREL_PIVOT_OFFSET = BARREL_LENGTH / 2.0f;
        const float BARREL_GAP = 35.0f; // Distance between multiple barrels
    }

    // -- Bullet Settings --
    namespace Bullet {
        const float DEFAULT_SIZE = 15.0f;
        const float BASE_SPEED = 800.0f;
    }

    // -- Enemy Settings --
    namespace Enemy {
        const float SPAWN_INTERVAL = 3.0f;
        const float HITBOX_RATIO = 0.6f;      // Tightens the collision box relative to sprite size
        const float SEPARATION_FORCE = 500.0f; // Pushes overlapping enemies apart
        const float FRICTION = 0.92f;
        const float SPAWN_SAFE_ZONE = 300.0f; // Radius around player where enemies won't spawn
        const float SPAWN_PUSH_DIST = 400.0f;
        const float SIZE_SMALL = 30.0f;
        const float SIZE_BIG = 67.0f;
        const float HP_SMALL = 15.0f;
        const float HP_BIG = 100.0f;
        const float SHRINK_SPEED = 100.0f;    // Visual speed of death animation
    }

    // -- Boss Settings --
    namespace Boss {
        // Shared Mechanics
        constexpr float TURN_SPEED = 8.0f;
        constexpr float GUN_POSITION_RATIO = 0.3f;
        constexpr float LASER_LENGTH = 1200.f;
        constexpr float LASER_WIDTH_ACTIVE = 25.f;
        constexpr float LASER_WIDTH_TELEGRAPH = 6.f;
        constexpr float LASER_DAMAGE_PER_SEC = 20.f;
        constexpr float CONTACT_DAMAGE_RATIO = 0.2f;

        // BOSS1: The Stalker
        constexpr float B1_BASE_HP = 1500.f;
        constexpr float B1_SCALE = 2.0f;
        constexpr float B1_CHASE_SPEED = 150.f;
        constexpr float B1_LUNGE_SPEED = 1400.f;
        constexpr float B1_IDLE_DUR = 1.5f;
        constexpr float B1_TELEGRAPH_DUR = 0.6f;
        constexpr float B1_LUNGE_DUR = 0.75f;
        constexpr float B1_COOLDOWN_DUR = 0.8f;
        constexpr int   B1_BULLET_COUNT = 8;
        constexpr int   B1_XP = 200;

        // BOSS2: The Summoner
        constexpr float B2_BASE_HP = 3000.f;
        constexpr float B2_SCALE = 2.5f;
        constexpr float B2_IDLE_DUR = 3.f;
        constexpr float B2_TELEGRAPH_DUR = 1.0f;
        constexpr float B2_LUNGE_DUR = 0.1f;
        constexpr float B2_COOLDOWN_DUR = 5.0f;
        constexpr int   B2_MINION_COUNT = 6;
        constexpr int   B2_XP = 400;

        // BOSS3: The Spiral
        constexpr float B3_BASE_HP = 6000.f;
        constexpr float B3_SCALE = 2.0f;
        constexpr float B3_CHASE_SPEED = 120.f;
        constexpr float B3_IDLE_DUR = 2.f;
        constexpr float B3_TELEGRAPH_DUR = 1.0f;
        constexpr float B3_LUNGE_DUR = 3.f;
        constexpr float B3_COOLDOWN_DUR = 0.5f;
        constexpr int   B3_SPIRAL_ARMS = 3;
        constexpr int   B3_XP = 300;

        // BOSS4: The Juggernaut
        constexpr float B4_BASE_HP = 9000.f;
        constexpr float B4_SCALE = 2.5f;
        constexpr float B4_CHASE_SPEED = 100.f;
        constexpr float B4_IDLE_DUR = 2.f;
        constexpr float B4_TELEGRAPH_DUR = 1.2f;
        constexpr float B4_LUNGE_DUR = 4.f;
        constexpr float B4_COOLDOWN_DUR = 0.8f;
        constexpr float B4_GUN_FIRE_RATE = 0.3f;
        constexpr float B4_LASER_SWEEP = 0.9f;
        constexpr int   B4_XP = 500;

        // Attack Modifiers
        constexpr float SPIRAL_SPEED = 10.f;
        constexpr float SPIRAL_FIRE_RATE = 0.15f;
        constexpr float SPIRAL_BULLET_SIZE = 13.f;
        constexpr float SPIRAL_DAMAGE_MUL = 0.5f;

        constexpr int   AIMED_COUNT = 3;
        constexpr float AIMED_SPREAD = 0.15f;
        constexpr float AIMED_BULLET_SIZE = 15.f;
        constexpr float AIMED_FIRE_INTERVAL = 0.4f;

        constexpr int   MINION_BASE_HP = 10;
        constexpr float MINION_SPAWN_RADIUS = 40.f;
    }
}

// =============================================================================
// EXTERNS & SYSTEM FUNCTIONS
// =============================================================================

extern std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;
extern bool mousereleased;
extern int currentWave;
extern float playerFlashTimer;     // Used for the "hit" feedback effect
extern AEGfxTexture* pEnemyTex[4]; // Array of textures for different enemy types
extern AEGfxVertexList* pEnemyMesh;

// ~ Brief: Loads heavy assets (textures, sounds) from disk.
void LoadGame();

// ~ Brief: Resets game state, positions, and pools for a new session.
void InitializeGame();

// ~ Brief: Main logic update loop (input, physics, AI).
void UpdateGame();

// ~ Brief: Renders all active game objects and HUD elements.
void DrawGame();

// ~ Brief: Clears transient memory and temporary pools.
void FreeGame();

// ~ Brief: Completely unloads assets from GPU/RAM before exit.
void UnloadGame();