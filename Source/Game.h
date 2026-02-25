#pragma once
// ===========================================================================
// CONFIGURATION & VARIABLES
// ===========================================================================
namespace GameConfig {
    // -- Math Constants --

    const float MOUSE_JITTER_THRESHOLD = 1.0f;

    // -- World/Engine Settings --
    const int   MAX_BULLETS_COUNT = 1000;
    const int   MAX_ENEMIES_COUNT = 50;
    const float OFF_SCREEN_COORD = 10000.0f;
    const float DESPAWN_DISTANCE_SQ = 2500.0f * 2500.0f;

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
        const float BARREL_GAP = 35.0f; // <--- Distance between multiple barrels
    }

    // -- Bullet Settings --
    namespace Bullet {
        const float DEFAULT_SIZE = 15.0f;
        const float BASE_SPEED = 800.0f;
    }

    // -- Enemy Settings --
    namespace Enemy {
        const float SPAWN_INTERVAL = 3.0f;
        const float HITBOX_RATIO = 0.6f;
        const float SEPARATION_FORCE = 500.0f;
        const float FRICTION = 0.92f;
        const float SPAWN_SAFE_ZONE = 300.0f;
        const float SPAWN_PUSH_DIST = 400.0f;
        const float SIZE_SMALL = 30.0f;
        const float SIZE_BIG = 67.0f;
        const float HP_SMALL = 15.0f;
        const float HP_BIG = 100.0f;
        const float SHRINK_SPEED = 100.0f;
    }
}

extern std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;

void LoadGame();
void UpdateGame();
void DrawGame();
void FreeGame();