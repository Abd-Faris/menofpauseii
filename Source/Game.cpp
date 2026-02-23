#include "MasterHeader.h"
//#include "DebugMenus.h"
//#include "Structs.h"
//#include "AEEngine.h"
//#include "AEGraphics.h"
//#include "AEMath.h"
//#include <cmath>
//#include <array>
//#include <vector>
//#define PI 3.14159265f
//#define HALF_PI 3.14159265f/ 2.0f
//#define TWO_PI 3.14159265f * 2.0f

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

// ===========================================================================
// GLOBAL OBJECTS
// ===========================================================================

// -- Enemy Pool --
std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;
f64 enemySpawnTimer = 0;

namespace {
    // -- Assets --
    s8 boldPixelsFont;
    AEGfxVertexList* MeshRect = nullptr;
    AEGfxVertexList* MeshCircle = nullptr;
    AEGfxVertexList* MeshTriangle = nullptr;

    // -- Player State --
    // Note: Ensure your struct in Structs.h has 'int barrelCount'
    shape player = { GameConfig::Tank::SCALE, 0.0f, 0.0f, 0.0f, 1 };

    // -- Bullet Pool --
    struct BulletObj {
        float posX, posY;
        float directionX, directionY;
        float speed;
        float size;
        bool isActive;
    };

    BulletObj bulletList[GameConfig::MAX_BULLETS_COUNT];
    float bulletFireTimer = 0.0f;

}

// ===========================================================================
// HELPER FUNCTIONS
// ===========================================================================

// --- NEW FUNCTION: DRAW MULTIPLE BARRELS ---

void DrawMultiBarrels(int count, float gap, float pivotOffset, float tankRot, float tankX, float tankY) {
    if (count <= 0) return;

    // 1. Calculate the starting X offset to keep the barrels centered on the turret
    float formationWidth = (float)(count - 1) * gap;
    float startX = -formationWidth / 2.0f;

    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);

    for (int i = 0; i < count; i++) {
        // 2. Calculate local position relative to the tank's center
        // Spread them along the X-axis and push them forward along Y
        float currentLocalX = startX + (i * gap);
        AEVec2 barrelOffset = { currentLocalX, pivotOffset };

        // 3. Define the size of each barrel
        AEVec2 barrelSize = { GameConfig::Tank::BARREL_WIDTH, GameConfig::Tank::BARREL_LENGTH };

        // 4. Use the generalized printMesh to handle the TRS math
        Gfx::printMesh(
            MeshRect,            // The barrel shape
            { tankX, tankY },    // Tank Position (World Coords)
            barrelSize,          // Scale
            tankRot,             // Rotation (Radians)
            barrelOffset         // Local Offset (Position relative to tank center)
        );
    }
}

//void DrawMultiBarrels(int count, float gap, float pivotOffset, float tankRot, float tankX, float tankY) {
//    if (count <= 0) return;
//
//    // 1. Calculate total width to center them
//    float formationWidth = (float)(count - 1) * gap;
//    float startX = -formationWidth / 2.0f;
//
//    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);
//
//    for (int i = 0; i < count; i++) {
//        float currentLocalX = startX + (i * gap);
//
//        AEMtx33 scaleMatrix, offsetMatrix, rotationMatrix, transMatrix, finalMatrix;
//
//        // Scale
//        AEMtx33Scale(&scaleMatrix, GameConfig::Tank::BARREL_WIDTH, GameConfig::Tank::BARREL_LENGTH);
//
//        // Translate Locally (Spread + Push forward)
//        AEMtx33Trans(&offsetMatrix, currentLocalX, pivotOffset);
//
//        // Rotate (Tank Angle)
//        AEMtx33Rot(&rotationMatrix, tankRot);
//
//        // Translate Global (Tank Pos)
//        AEMtx33Trans(&transMatrix, tankX, tankY);
//
//        // Combine: GlobalTrans * GlobalRot * LocalOffset * Scale
//        AEMtx33Concat(&finalMatrix, &offsetMatrix, &scaleMatrix);
//        AEMtx33Concat(&finalMatrix, &rotationMatrix, &finalMatrix);
//        AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
//
//        AEGfxSetTransform(finalMatrix.m);
//        AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
//    }
//}

void ResetEnemy(Enemies* enemyToReset) {
    enemyToReset->alive = false;
    enemyToReset->pos.x = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->pos.y = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->scale = 0;
    enemyToReset->hp = 0;
}

void SpawnOneEnemy(bool isBigEnemy) {
    for (auto& newEnemy : enemyPool) {
        if (!newEnemy.alive) {
            float windowWidth = (float)AEGfxGetWindowWidth();
            float windowHeight = (float)AEGfxGetWindowHeight();

            float spawnX = (AERandFloat() * windowWidth) - windowWidth / 2.0f;
            float spawnY = (AERandFloat() * windowHeight) - windowHeight / 2.0f;

            float differenceX = spawnX - player.pos_x;
            float differenceY = spawnY - player.pos_y;
            float distanceToPlayer = sqrt((differenceX * differenceX) + (differenceY * differenceY));

            if (distanceToPlayer < GameConfig::Enemy::SPAWN_SAFE_ZONE) {
                spawnX += GameConfig::Enemy::SPAWN_PUSH_DIST;
            }

            newEnemy.pos = { spawnX, spawnY };
            newEnemy.velocity = { 0, 0 };
            newEnemy.alive = true;
            newEnemy.rotation = AERandFloat() * 360.0f;
            newEnemy.scale = isBigEnemy ? GameConfig::Enemy::SIZE_BIG : GameConfig::Enemy::SIZE_SMALL;

            int initialHP = (int)(isBigEnemy ? GameConfig::Enemy::HP_BIG : GameConfig::Enemy::HP_SMALL);
            newEnemy.hp = initialHP;
            newEnemy.maxhp = initialHP;
            newEnemy.enemtype = PASSIVE;
            break;
        }
    }
}

void SpawnAttackEnemy() {
    for (auto& newEnemy : enemyPool) {
        if (!newEnemy.alive) {
            float windowWidth = (float)AEGfxGetWindowWidth();
            float windowHeight = (float)AEGfxGetWindowHeight();

            float spawnX = (AERandFloat() * windowWidth) - windowWidth / 2.0f;
            float spawnY = (AERandFloat() * windowHeight) - windowHeight / 2.0f;

            float differenceX = spawnX - player.pos_x;
            float differenceY = spawnY - player.pos_y;
            float distanceToPlayer = sqrt((differenceX * differenceX) + (differenceY * differenceY));

            if (distanceToPlayer < GameConfig::Enemy::SPAWN_SAFE_ZONE) {
                spawnX += GameConfig::Enemy::SPAWN_PUSH_DIST;
            }

            newEnemy.pos = { spawnX, spawnY };
            newEnemy.velocity = { 0, 0};
            newEnemy.alive = true;
            newEnemy.rotation = AERandFloat() * 360.0f;
            newEnemy.scale = GameConfig::Enemy::SIZE_BIG ;
            newEnemy.enemtype = ATTACK;

            int initialHP = (int)(GameConfig::Enemy::HP_BIG);
            newEnemy.hp = initialHP;
            newEnemy.maxhp = initialHP;
            break;
        }
    }
}

// ===========================================================================
// COLLISION LOGIC
// ===========================================================================
void circlerectcollision() {
    for (auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive) continue;

        if (currentEnemy.enemtype == ATTACK) {
            float differenceX = player.pos_x - currentEnemy.pos.x;
            float differenceY = player.pos_y - currentEnemy.pos.y;
            float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

            float collisionRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;
            float currentdmg = calculate_max_stats(1);

            if (distanceSquared < (collisionRadius * collisionRadius)) {
                player_init.current_hp -= currentEnemy.hp / 10;
                currentEnemy.hp = 0;

            }
                
        }

        for (auto& boolet : bulletList) {
            if (!boolet.isActive) continue;

            float differenceX = boolet.posX - currentEnemy.pos.x;
            float differenceY = boolet.posY - currentEnemy.pos.y;
            float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

            float collisionRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + boolet.size;
            float currentdmg = calculate_max_stats(1);

            if (distanceSquared < (collisionRadius * collisionRadius)) {
                currentEnemy.hp -= (int)currentdmg;
                boolet.isActive = false;

                if (currentEnemy.hp <= 0 && currentEnemy.alive) {
                    // Enemy Dead
                }
            }
        }
    }
}

// ===========================================================================
// LOAD GAME
// ===========================================================================
void LoadGame() {
    LoadDebug1();
    boldPixelsFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);

    // Mesh: Circle
    //AEGfxMeshStart();
    //int slices = 40;
    //float angleStep = TWO_PI/ slices;
    //for (int i = 0; i < slices; i++) {
    //    AEGfxTriAdd(
    //        0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,
    //        cosf(i * angleStep), sinf(i * angleStep), 0xFFFFFFFF, 1.0f, 1.0f,
    //        cosf((i + 1) * angleStep), sinf((i + 1) * angleStep), 0xFFFFFFFF, 0.0f, 0.0f);
    //}
    //MeshCircle = AEGfxMeshEnd();
    MeshCircle = Gfx::createCircleMesh(0xFFFFFFFF);

    // Mesh: Rect
    //AEGfxMeshStart();
    //AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    //AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    //MeshRect = AEGfxMeshEnd();
    MeshRect = Gfx::createRectMesh("center", 0xFFFFFFFFF);

    MeshTriangle = Gfx::createTriangleMesh(0xFFFFFFFF);

    // Reset Player
    player.pos_x = 0;
    player.pos_y = 0;
    player.currentAngle = 0;
    player.scale = GameConfig::Tank::SCALE;
    player.barrelCount = 1; // Default to Single Barrel

    bulletFireTimer = 0;
    enemySpawnTimer = 0;

    for (int i = 0; i < GameConfig::MAX_BULLETS_COUNT; i++) bulletList[i].isActive = false;
    for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; i++) ResetEnemy(&enemyPool[i]);
    for (int i = 0; i < 5; i++) SpawnOneEnemy(false);
}

// ===========================================================================
// UPDATE GAME
// ===========================================================================
void UpdateGame() {
    float deltaTime = (float)AEFrameRateControllerGetFrameTime();
    UpdateDebug1();

    if (!player_init.menu_open) {

        // --- KEY '7': TOGGLE UPGRADE ---
        if (AEInputCheckTriggered(AEVK_7)) {
            if (player.barrelCount == 1) {
                player.barrelCount = 2; // Upgrade to Dual
                player.scale *= 1.2;
            }
            else {
                player.barrelCount = 1; // Downgrade to Single
               
            }
        }

        // 1. Move Player
        float playerSpeed = calculate_max_stats(2);
        if (AEInputCheckCurr(AEVK_W)) player.pos_y += playerSpeed * deltaTime;
        if (AEInputCheckCurr(AEVK_S)) player.pos_y -= playerSpeed * deltaTime;
        if (AEInputCheckCurr(AEVK_A)) player.pos_x -= playerSpeed * deltaTime;
        if (AEInputCheckCurr(AEVK_D)) player.pos_x += playerSpeed * deltaTime;

        // 2. Rotate Player
        s32 mouseX, mouseY;
        AEInputGetCursorPosition(&mouseX, &mouseY);
        float windowWidth = (float)AEGfxGetWindowWidth();
        float windowHeight = (float)AEGfxGetWindowHeight();
        float mouseRelativeX = (float)mouseX - windowWidth / 2.0f;
        float mouseRelativeY = windowHeight / 2.0f - (float)mouseY;

        if ((mouseRelativeX * mouseRelativeX) + (mouseRelativeY * mouseRelativeY) > GameConfig::MOUSE_JITTER_THRESHOLD) {
            float targetAngle = atan2f(mouseRelativeY, mouseRelativeX) - HALF_PI;
            float angleDifference = targetAngle - player.currentAngle;
            while (angleDifference > PI) angleDifference -= TWO_PI;
            while (angleDifference < -PI) angleDifference += TWO_PI;
            player.currentAngle += angleDifference * 0.1f;
        }

        // 3. Shooting (UPDATED FOR MULTI-BARREL)
        if (AEInputCheckCurr(AEVK_SPACE)) {
            bulletFireTimer -= deltaTime;

            if (bulletFireTimer <= 0) {
                float fire_rate = calculate_max_stats(3);
                bulletFireTimer = fire_rate;

                // We need to find 'barrelCount' inactive bullets
                int bulletsNeeded = player.barrelCount;

                // Calculate Start Offset for bullets (Same math as drawing)
                float totalWidth = (float)(player.barrelCount - 1) * GameConfig::Tank::BARREL_GAP;
                float startOffset = -totalWidth / 2.0f;

                int bulletsFound = 0;
                for (auto& boolet : bulletList) {
                    if (!boolet.isActive) {
                        boolet.isActive = true;

                        // -- Calculate Multi-Barrel Spawn Position --
                        float offsetLocalX = startOffset + (bulletsFound * GameConfig::Tank::BARREL_GAP);
                        float offsetLocalY = GameConfig::Tank::BARREL_LENGTH; // Spawn at tip

                        // Rotate Local Offset by Tank Angle
                        // Formula: x' = x*cos - y*sin, y' = x*sin + y*cos
                        // Note: AE Engine Rotation is usually Standard Math Rotation
                        float cosA = cosf(player.currentAngle);
                        float sinA = sinf(player.currentAngle);

                        // Because Tank "Forward" is Angle + 90deg (Y-Axis), we adjust coordinate rotation:
                        // Tank Right Vector (X axis) is (cos(a), sin(a))
                        // Tank Forward Vector (Y axis) is (cos(a+90), sin(a+90)) -> (-sin(a), cos(a))

                        float spawnOffsetX = (offsetLocalX * cosA) - (offsetLocalY * sinA);
                        float spawnOffsetY = (offsetLocalX * sinA) + (offsetLocalY * cosA);

                        boolet.posX = player.pos_x + spawnOffsetX;
                        boolet.posY = player.pos_y + spawnOffsetY;

                        // Direction is same for all parallel barrels
                        boolet.directionX = -sinA; // Forward Vector X
                        boolet.directionY = cosA;  // Forward Vector Y

                        boolet.speed = GameConfig::Bullet::BASE_SPEED;
                        boolet.size = GameConfig::Bullet::DEFAULT_SIZE;

                        bulletsFound++;
                        if (bulletsFound >= bulletsNeeded) break;
                    }
                }
            }
        }

        // 4. Move Bullets
        for (auto& boolet : bulletList) {
            if (boolet.isActive) {
                boolet.posX += boolet.directionX * boolet.speed * deltaTime;
                boolet.posY += boolet.directionY * boolet.speed * deltaTime;

                float diffX = boolet.posX - player.pos_x;
                float diffY = boolet.posY - player.pos_y;
                if ((diffX * diffX + diffY * diffY) > GameConfig::DESPAWN_DISTANCE_SQ) {
                    boolet.isActive = false;
                }
            }
        }

        // 5. Spawn Enemies
        enemySpawnTimer += deltaTime;
        if (enemySpawnTimer >= GameConfig::Enemy::SPAWN_INTERVAL) {
            bool spawnBig = (AERandFloat() * 10.0f) < 4.0f;
            SpawnOneEnemy(spawnBig);
            if (player_init.player_level >= 5)
            {
                SpawnAttackEnemy();
            }
            enemySpawnTimer = 0;
        }

        // 6. Enemy Physics
        for (auto& currentEnemy : enemyPool) {
            if (!currentEnemy.alive) continue;

            if (currentEnemy.enemtype == ATTACK) {
                AEVec2 PlayerPos = { player.pos_x, player.pos_y };
                AEVec2 EnemyPos = { currentEnemy.pos };
                AEVec2 dir = {};
                AEVec2Sub(&dir, &PlayerPos, &EnemyPos);

                f32 hyp = sqrt(dir.x * dir.x + dir.y * dir.y);

                /*f32 enemydir = atan2f(dir.y, dir.x) - HALF_PI;
                currentEnemy.rotation = enemydir;*/

                if ((dir.x * dir.x) + (dir.y * dir.y) > GameConfig::MOUSE_JITTER_THRESHOLD) {
                    float targetAngle = atan2f(dir.y, dir.x) * (180.f/PI);
                    float angleDifference = targetAngle - currentEnemy.rotation;
                    while (angleDifference > 180.f) angleDifference -= 360.f;
                    while (angleDifference < -180.f) angleDifference += 360.f;
                    currentEnemy.rotation += angleDifference * 0.1f;
                }


                dir.x /= hyp;
                dir.y /= hyp;
                

                currentEnemy.velocity.x += dir.x * 500 * deltaTime;
                currentEnemy.velocity.y += dir.y * 500 * deltaTime;
                currentEnemy.velocity.x *= GameConfig::Enemy::FRICTION;
                currentEnemy.velocity.y *= GameConfig::Enemy::FRICTION;
                currentEnemy.pos.x += currentEnemy.velocity.x * deltaTime;
                currentEnemy.pos.y += currentEnemy.velocity.y * deltaTime;
            }

            AEVec2 separationForce = { 0,0 };
            for (auto& otherEnemy : enemyPool) {
                if (&currentEnemy == &otherEnemy || !otherEnemy.alive) continue;
                float diffX = currentEnemy.pos.x - otherEnemy.pos.x;
                float diffY = currentEnemy.pos.y - otherEnemy.pos.y;
                float distance = sqrt(diffX * diffX + diffY * diffY);
                float minDistance = (currentEnemy.scale + otherEnemy.scale) * GameConfig::Enemy::HITBOX_RATIO;

                if (distance < minDistance && distance > 0.1f) {
                    float pushStrength = (minDistance - distance) / minDistance * GameConfig::Enemy::SEPARATION_FORCE;
                    separationForce.x += (diffX / distance) * pushStrength;
                    separationForce.y += (diffY / distance) * pushStrength;
                }
            }
            currentEnemy.velocity.x += separationForce.x * deltaTime;
            currentEnemy.velocity.y += separationForce.y * deltaTime;
            currentEnemy.velocity.x *= GameConfig::Enemy::FRICTION;
            currentEnemy.velocity.y *= GameConfig::Enemy::FRICTION;
            currentEnemy.pos.x += currentEnemy.velocity.x * deltaTime;
            currentEnemy.pos.y += currentEnemy.velocity.y * deltaTime;

            if (currentEnemy.hp <= 0) {
                currentEnemy.scale -= GameConfig::Enemy::SHRINK_SPEED * deltaTime;
                if (currentEnemy.scale <= 0) {
                    float xp_multiplier = calculate_max_stats(4);
                    float baseReward = (currentEnemy.maxhp >= (int)GameConfig::Enemy::HP_BIG) ? 80.0f : 20.0f;
                    player_init.current_xp += (baseReward * xp_multiplier);
                    ResetEnemy(&currentEnemy);
                }
            }
        }
    }
    circlerectcollision();
    AEGfxSetCamPosition(player.pos_x, player.pos_y);

}

// ===========================================================================
// DRAW GAME
// ===========================================================================

void DrawGame() {
    if (MeshRect == nullptr || MeshCircle == nullptr) return;

    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetCamPosition(player.pos_x, player.pos_y);

    // -- Draw Bullets --
    // Bullets use a simple scale and position with no rotation
    AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f);
    for (const auto& boolet : bulletList) {
        if (boolet.isActive) {
            Gfx::printMesh(MeshCircle, { boolet.posX, boolet.posY }, { boolet.size, boolet.size }, 0.0f);
        }
    }

    // -- Draw Player Tank --
    AEVec2 playerPos = { player.pos_x, player.pos_y };

    // 1. Draw Tracks
    AEGfxSetColorToMultiply(0.1f, 0.1f, 0.1f, 1.0f);
    AEVec2 trackSize = { GameConfig::Tank::TRACK_WIDTH, GameConfig::Tank::TRACK_HEIGHT };

    // Left Track: Offset to the left before rotating
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { -GameConfig::Tank::TRACK_OFFSET_X, 0.0f });

    // Right Track: Offset to the right before rotating
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { GameConfig::Tank::TRACK_OFFSET_X, 0.0f });

    // 2. Draw Main Body
    AEGfxSetColorToMultiply(0.2f, 0.6f, 0.2f, 1.0f);
    Gfx::printMesh(MeshRect, playerPos, { GameConfig::Tank::BODY_WIDTH, GameConfig::Tank::BODY_HEIGHT }, player.currentAngle);

    // 3. Draw Barrels
    DrawMultiBarrels(
        player.barrelCount,
        GameConfig::Tank::BARREL_GAP,
        GameConfig::Tank::BARREL_PIVOT_OFFSET,
        player.currentAngle,
        player.pos_x,
        player.pos_y
    );

    // 4. Draw Turret
    AEGfxSetColorToMultiply(0.3f, 0.7f, 0.3f, 1.0f);
    Gfx::printMesh(MeshCircle, playerPos, { GameConfig::Tank::TURRET_SIZE, GameConfig::Tank::TURRET_SIZE }, player.currentAngle);

    // -- Draw Enemies --
    for (const auto& currentEnemy : enemyPool) {
        if (currentEnemy.alive || currentEnemy.scale > 0) {
            // Set color based on enemy size
            if (currentEnemy.enemtype == PASSIVE) {
            if (currentEnemy.scale > (GameConfig::Enemy::SIZE_SMALL + 10.0f))
                AEGfxSetColorToMultiply(0.2f, 0.2f, 0.8f, 1);
            else
                AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1);
            }

            if (currentEnemy.enemtype == ATTACK) {
                AEGfxSetColorToMultiply(0.6f, 0.2f, 0.6f, 1);
                float rotationRad = currentEnemy.rotation * (PI / 180.0f);
                Gfx::printMesh(MeshTriangle, currentEnemy.pos, { currentEnemy.scale, currentEnemy.scale }, rotationRad);
                continue;
            }
            // Convert degrees to radians for the rotation parameter
            float rotationRad = currentEnemy.rotation * (PI / 180.0f);
            Gfx::printMesh(MeshRect, currentEnemy.pos, { currentEnemy.scale, currentEnemy.scale }, rotationRad);
        }
    }
    DrawDebug1(); // Render Stats UI

}

//void DrawGame() {
//    if (MeshRect == nullptr || MeshCircle == nullptr) return;
//
//    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
//    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
//    AEGfxSetCamPosition(player.pos_x, player.pos_y);
//
//    // -- Draw Bullets --
//    AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f);
//    for (const auto& boolet : bulletList) {
//        if (boolet.isActive) {
//            AEMtx33 scaleMatrix, transMatrix, finalMatrix;
//            AEMtx33Scale(&scaleMatrix, boolet.size, boolet.size);
//            AEMtx33Trans(&transMatrix, boolet.posX, boolet.posY);
//            AEMtx33Concat(&finalMatrix, &transMatrix, &scaleMatrix);
//            AEGfxSetTransform(finalMatrix.m);
//            AEGfxMeshDraw(MeshCircle, AE_GFX_MDM_TRIANGLES);
//        }
//    }
//
//    // -- Draw Player Tank --
//    AEMtx33 scaleMatrix, rotationMatrix, transMatrix, offsetMatrix, finalMatrix;
//    AEMtx33Rot(&rotationMatrix, player.currentAngle);
//    AEMtx33Trans(&transMatrix, player.pos_x, player.pos_y);
//
//    // 1. Draw Tracks
//    AEGfxSetColorToMultiply(0.1f, 0.1f, 0.1f, 1.0f);
//    // Left
//    AEMtx33Scale(&scaleMatrix, GameConfig::Tank::TRACK_WIDTH, GameConfig::Tank::TRACK_HEIGHT);
//    AEMtx33Trans(&offsetMatrix, -GameConfig::Tank::TRACK_OFFSET_X, 0.0f);
//    AEMtx33Concat(&finalMatrix, &offsetMatrix, &scaleMatrix);
//    AEMtx33Concat(&finalMatrix, &rotationMatrix, &finalMatrix);
//    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
//    AEGfxSetTransform(finalMatrix.m);
//    AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
//    // Right
//    AEMtx33Scale(&scaleMatrix, GameConfig::Tank::TRACK_WIDTH, GameConfig::Tank::TRACK_HEIGHT);
//    AEMtx33Trans(&offsetMatrix, GameConfig::Tank::TRACK_OFFSET_X, 0.0f);
//    AEMtx33Concat(&finalMatrix, &offsetMatrix, &scaleMatrix);
//    AEMtx33Concat(&finalMatrix, &rotationMatrix, &finalMatrix);
//    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
//    AEGfxSetTransform(finalMatrix.m);
//    AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
//
//    // 2. Draw Main Body
//    AEGfxSetColorToMultiply(0.2f, 0.6f, 0.2f, 1.0f);
//    AEMtx33Scale(&scaleMatrix, GameConfig::Tank::BODY_WIDTH, GameConfig::Tank::BODY_HEIGHT);
//    AEMtx33Concat(&finalMatrix, &rotationMatrix, &scaleMatrix);
//    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
//    AEGfxSetTransform(finalMatrix.m);
//    AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
//
//    // 3. Draw Barrels (Dynamic Multi-Barrel)
//    DrawMultiBarrels(
//        player.barrelCount,
//        GameConfig::Tank::BARREL_GAP,
//        GameConfig::Tank::BARREL_PIVOT_OFFSET,
//        player.currentAngle,
//        player.pos_x,
//        player.pos_y
//    );
//
//    // 4. Draw Turret
//    AEGfxSetColorToMultiply(0.3f, 0.7f, 0.3f, 1.0f);
//    AEMtx33Scale(&scaleMatrix, GameConfig::Tank::TURRET_SIZE, GameConfig::Tank::TURRET_SIZE);
//    AEMtx33Concat(&finalMatrix, &rotationMatrix, &scaleMatrix);
//    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
//    AEGfxSetTransform(finalMatrix.m);
//    AEGfxMeshDraw(MeshCircle, AE_GFX_MDM_TRIANGLES);
//
//    // -- Draw Enemies --
//    for (const auto& currentEnemy : enemyPool) {
//        if (currentEnemy.alive || currentEnemy.scale > 0) {
//            if (currentEnemy.scale > (GameConfig::Enemy::SIZE_SMALL + 10.0f))
//                AEGfxSetColorToMultiply(0.2f, 0.2f, 0.8f, 1);
//            else
//                AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1);
//
//            AEMtx33Scale(&scaleMatrix, currentEnemy.scale, currentEnemy.scale);
//            AEMtx33RotDeg(&rotationMatrix, currentEnemy.rotation);
//            AEMtx33Trans(&transMatrix, currentEnemy.pos.x, currentEnemy.pos.y);
//            AEMtx33Concat(&finalMatrix, &transMatrix, &rotationMatrix);
//            AEMtx33Concat(&finalMatrix, &finalMatrix, &scaleMatrix);
//            AEGfxSetTransform(finalMatrix.m);
//            AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
//        }
//    }
//
//    DrawDebug1();
//}

void FreeGame() {
    AEGfxDestroyFont(boldPixelsFont);
    if (MeshRect) AEGfxMeshFree(MeshRect);
    if (MeshCircle) AEGfxMeshFree(MeshCircle);
    FreeDebug1();
}