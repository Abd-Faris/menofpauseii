#include "MasterHeader.h"

// ===========================================================================
// GLOBAL OBJECTS
// ===========================================================================
extern bool dualback, orbitActive;
extern float orbitAngle, orbitPosX, orbitPosY;


namespace {
    s8 boldPixelsFont;
    AEGfxVertexList* MeshRect = nullptr;
    AEGfxVertexList* MeshCircle = nullptr;
    AEGfxVertexList* MeshTriangle = nullptr;

    shape player = { GameConfig::Tank::SCALE, 0.0f, 0.0f, 0.0f, 1 };
    
}

// ===========================================================================
// COLLISION LOGIC
// ===========================================================================
void circlerectcollision1() {
    for (auto& currentEnemy : minionPool) {
        if (!currentEnemy.alive) continue;

        if (1) {
            float differenceX = player.pos_x - currentEnemy.pos.x;
            float differenceY = player.pos_y - currentEnemy.pos.y;
            float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

            float collisionRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;
            float currentdmg = calculate_max_stats(1);

            if (distanceSquared < (collisionRadius * collisionRadius)) {
                player_init.current_hp -= currentEnemy.hp / 5;
                currentEnemy.hp = 0;
                TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
            }

        }
        if (orbitActive) {
            float diffX = orbitPosX - currentEnemy.pos.x;
            float diffY = orbitPosY - currentEnemy.pos.y;
            float distanceSquared = (diffX * diffX) + (diffY * diffY);

            float orbitSize = 20.0f; // Radius of the ball
            float collisionRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + orbitSize;

            if (distanceSquared < (collisionRadius * collisionRadius)) {
                currentEnemy.hp = 0;
                TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
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
                currentEnemy.hp -= (int)(currentdmg * boolet.damagemul);
                boolet.isActive = false;

                if (currentEnemy.hp <= 0 && currentEnemy.alive) {
                    //triggers explosion animation at enemy position
                    TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
                }
            }
        }
        for (auto& enBullet : enemyBulletList) {
            if (!enBullet.isActive) continue;

            float differenceX = enBullet.posX - player.pos_x;
            float differenceY = enBullet.posY - player.pos_y;
            float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

            // Calculate collision radius (Player scale + bullet size)
            float collisionRadius = player.scale + enBullet.size;

            bool hit = (distanceSquared < (collisionRadius * collisionRadius));

            if (!hit) {
                //check for collision along the barrel
                float barrelLen = GameConfig::Tank::BARREL_LENGTH * (player.scale / GameConfig::Tank::SCALE);
                float cosA = cosf(player.currentAngle);
                float sinA = sinf(player.currentAngle);

                //3 points on the barrel
                float checkpoints[3] = { 0.5f, 0.75f, 1.0f };

                //check each point for collision
                for (int p = 0; p < 3; p++) {
                    //get worldposition of the checkpoint
                    float checkX = player.pos_x + (0.0f * cosA - (barrelLen * checkpoints[p]) * sinA);
                    float checkY = player.pos_y + (0.0f * sinA + (barrelLen * checkpoints[p]) * cosA);
                    //check distance from bullet to checkpoint
                    float diffX = enBullet.posX - checkX;
                    float diffY = enBullet.posY - checkY;
                    float distSq = (diffX * diffX) + (diffY * diffY);
                    float checkRadius = enBullet.size;
                    if (distSq < (checkRadius * checkRadius)) {
                        hit = true;
                        break;
                    }
                }
            }

            if (hit) {
                player_init.current_hp -= 10; // player
                enBullet.isActive = false;     // Destroy the enemy bullet

            }
        }
    }
}

// ===========================================================================
// LOAD
// ===========================================================================
void LoadDebug5() {
    LoadDebug1();
    Animations_Load();
    boldPixelsFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
    World::Load_World();
    World::Init_World();

    MeshCircle = Gfx::createCircleMesh(0xFFFFFFFF);
    MeshRect = Gfx::createRectMesh("center", 0xFFFFFFFFF);
    MeshTriangle = Gfx::createTriangleMesh(0xFFFFFFFF);

    // Reset player
    player.pos_x = 0;
    player.pos_y = 0;
    player.currentAngle = 0;
    player.scale = GameConfig::Tank::SCALE;
    player.barrelCount = 1;

    bulletFireTimer = 0;
    for (auto& minion : minionPool) ResetEnemy(&minion);

    for (int i = 0; i < GameConfig::MAX_BULLETS_COUNT; i++) bulletList[i].isActive = false;



    // Spawn boss
    SpawnBoss(BOSS3, player);
}

// ===========================================================================
// UPDATE
// ===========================================================================
void UpdateDebug5() {
    float deltaTime = (float)AEFrameRateControllerGetFrameTime();
    UpdateDebug1();

    if (!player_init.menu_open) {
        Animations_Update(deltaTime);

        drawBigTank(player);
        drawBigCannon(player);
        DualBack(player);

        movePlayer(player, deltaTime);
        rotatePlayer(player);
        ShootBullet(player, deltaTime);
        updateOrbit(player, deltaTime);
        updateBullets(player, deltaTime);
        updateMinionPhysics(player, deltaTime);

        // No EnemySpawner, no updateEnemyPhysics — boss only
        UpdateBossPhysics(boss, player, deltaTime);
    }

    BossCollision(boss, player, orbitActive,  orbitPosX,  orbitPosY);
    circlerectcollision1();
    AEGfxSetCamPosition(player.pos_x, player.pos_y);

    //// Win condition
    //if (!boss.alive) {
    //    player_init.current_xp += boss.xp;
    //    AEGameStateMgrSetNextState(GS_GAME);
    //}

    //// Lose condition
    //if (player_init.current_hp <= 0) {
    //    AEGameStateMgrSetNextState(GS_GAMEOVER);
    //}
}

// ===========================================================================
// DRAW
// ===========================================================================
void DrawDebug5() {
    if (MeshRect == nullptr || MeshCircle == nullptr) return;

    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
    AEGfxSetCamPosition(player.pos_x, player.pos_y);

    // -- Draw World --

    World::Draw_World();
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // -- Draw Bullets --
    AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f);
    for (const auto& boolet : bulletList) {
        if (boolet.isActive)
            Gfx::printMesh(MeshCircle, { boolet.posX, boolet.posY }, { boolet.size, boolet.size }, 0.0f);
    }

    AEGfxSetColorToMultiply(1.0f, 0.2f, 0.2f, 1.0f); // Bright Red
    for (const auto& enBullet : enemyBulletList) {
        if (enBullet.isActive) {
            Gfx::printMesh(MeshCircle, { enBullet.posX, enBullet.posY }, { enBullet.size, enBullet.size }, 0.0f);
        }
    }

    // -- Draw Orbit --
    if (orbitActive) {
        AEGfxSetColorToMultiply(0.0f, 1.0f, 1.0f, 1.0f);
        Gfx::printMesh(MeshCircle, { orbitPosX, orbitPosY }, { 40.0f, 40.0f }, orbitAngle);
    }

    // -- Draw Player Tank --
    AEVec2 playerPos = { player.pos_x, player.pos_y };
    float visualScale = player.scale / GameConfig::Tank::SCALE;

    // Tracks
    AEGfxSetColorToMultiply(0.1f, 0.1f, 0.1f, 1.0f);
    AEVec2 trackSize = { GameConfig::Tank::TRACK_WIDTH * visualScale, GameConfig::Tank::TRACK_HEIGHT * visualScale };
    float scaledTrackOffset = GameConfig::Tank::TRACK_OFFSET_X * visualScale;
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { -scaledTrackOffset, 0.0f });
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { scaledTrackOffset, 0.0f });

    // Body
    AEGfxSetColorToMultiply(0.2f, 0.6f, 0.2f, 1.0f);
    Gfx::printMesh(MeshRect, playerPos, { GameConfig::Tank::BODY_WIDTH * visualScale, GameConfig::Tank::BODY_HEIGHT * visualScale }, player.currentAngle);

    // Barrels
    float cannonwidthnow = bigcannon ? GameConfig::Tank::BARREL_WIDTH * 2.0f : GameConfig::Tank::BARREL_WIDTH;
    if (dualback) {
        DrawMultiBarrels(1, 0.0f, GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale, player.currentAngle, player.pos_x, player.pos_y, cannonwidthnow * visualScale, GameConfig::Tank::BARREL_LENGTH * visualScale, MeshRect);
        DrawMultiBarrels(1, 0.0f, GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale, player.currentAngle + PI, player.pos_x, player.pos_y, cannonwidthnow * visualScale, GameConfig::Tank::BARREL_LENGTH * visualScale, MeshRect);
    }
    else {
        DrawMultiBarrels(player.barrelCount, GameConfig::Tank::BARREL_GAP * visualScale, GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale, player.currentAngle, player.pos_x, player.pos_y, cannonwidthnow * visualScale, GameConfig::Tank::BARREL_LENGTH * visualScale, MeshRect);
    }

    // Turret
    AEGfxSetColorToMultiply(0.3f, 0.7f, 0.3f, 1.0f);
    Gfx::printMesh(MeshCircle, playerPos, { GameConfig::Tank::TURRET_SIZE * visualScale, GameConfig::Tank::TURRET_SIZE * visualScale }, player.currentAngle);

    // -- Draw Boss --
    if (boss.alive) {
        DrawBoss(boss, MeshRect, MeshCircle);
        DrawBossHP(boss, MeshRect, MeshCircle, player);
    }

    for (const auto& currentEnemy : minionPool) {
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

    Animations_Draw();
    DrawDebug1();
}

// ===========================================================================
// FREE
// ===========================================================================
void FreeDebug5() {
    AEGfxDestroyFont(boldPixelsFont);
    if (MeshRect)   AEGfxMeshFree(MeshRect);
    if (MeshCircle) AEGfxMeshFree(MeshCircle);
    FreeDebug1();
    Animations_Free();
    World::Free_World();
}