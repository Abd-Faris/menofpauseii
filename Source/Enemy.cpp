// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Enemy.cpp
// Authors: [Men of Pause II]
// Brief:   This file defines the creation and behaviours of enemies, including
//          spawning, physics, bullet handling, and off-screen indicators.
// ------------------------------------------------------------------------- //


// ------INCLUDE FILES------------------------------------------------------ //

#include "MasterHeader.h"

// =============================================================================
// GLOBAL VARIABLES & POOLS
// =============================================================================

// ~Brief: Object pool for all active enemies.Fixed size matches MAX_ENEMIES_COUNT
//          to avoid dynamic allocation during gameplay.
std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;

// ~ Brief: Accumulates time between enemy spawns during wave spawning.
f64 enemySpawnTimer = 0;

extern int currentWave;

// ~ Brief: Pool of enemy-fired bullets. Shared across all enemy types and the boss.
BulletObj enemyBulletList[GameConfig::MAX_BULLETS_COUNT];

// ~ Brief: Textures for each enemy variant — indexed by enemy type.
//          [0] = PASSIVE small, [1] = PASSIVE big, [2] = ATTACK (kamikaze), [3] = SHOOTER.
AEGfxTexture* pEnemyTex[4] = { nullptr, nullptr, nullptr, nullptr };
AEGfxVertexList* pEnemyMesh = nullptr;
AEGfxTexture* pEnemyBulletTex = nullptr;
AEGfxVertexList* pEnemyBulletMesh = nullptr;

// ~ Brief: File paths for enemy textures — order must match pEnemyTex index convention.
const char* enemyTextures[4] = {
    "./Assets/smallbox.png",    // PASSIVE small
    "./Assets/bigbox.png",      // PASSIVE big
    "./Assets/kamikaze.png",   // ATTACK (kamikaze)
    "./Assets/shooter.png"   // SHOOTER
};

// ------FUNCTIONS------------------------------------------- //

// =============================================================================
// LOAD / FREE
// =============================================================================

// ~ Brief: Load all enemy textures and build the shared UV-mapped unit quad mesh
//          used to render all enemy types and their bullets.
void LoadEnemies() {
    for (int i = 0; i < 4; ++i) {
        pEnemyTex[i] = AEGfxTextureLoad(enemyTextures[i]);
    }

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pEnemyMesh = AEGfxMeshEnd();

    //bullet mesh
    pEnemyBulletTex = AEGfxTextureLoad("./Assets/bulletred.png");

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pEnemyBulletMesh = AEGfxMeshEnd();
}

// ~ Brief: Unload all enemy and bullet textures and free their meshes.
//          All pointers are set to nullptr after freeing to prevent double-free.
void FreeEnemies() {
    for (int i = 0; i < 4; ++i) {
        if (pEnemyTex[i]) { AEGfxTextureUnload(pEnemyTex[i]); pEnemyTex[i] = nullptr; }
    }
    if (pEnemyMesh) { AEGfxMeshFree(pEnemyMesh); pEnemyMesh = nullptr; }
    if (pEnemyBulletTex) { AEGfxTextureUnload(pEnemyBulletTex);  pEnemyBulletTex = nullptr; }
    if (pEnemyBulletMesh) { AEGfxMeshFree(pEnemyBulletMesh);      pEnemyBulletMesh = nullptr; }
}

// =============================================================================
// SPAWN FUNCTIONS
// =============================================================================

// ~ Brief: Reset an enemy slot back to its default inactive state.
//          Moves the enemy off-screen so it won't be drawn or collided with.
void ResetEnemy(Enemies* enemyToReset) {
    enemyToReset->alive = false;
    enemyToReset->pos.x = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->pos.y = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->scale = 0;
    enemyToReset->hp = 0;
}

// ~ Brief: Spawn a passive box enemy (small or big) at a safe world position.
//          HP scales with the current wave multiplier.
void SpawnOneEnemy(bool isBigEnemy, shape player) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    for (auto& newEnemy : enemyPool) {
        if (!newEnemy.alive) {
            // Spawn within world borders (one tile inset from each edge)
            float minX = -World::HALF_WIDTH + World::TILE_SIZE * 1.5f;
            float maxX = World::HALF_WIDTH - World::TILE_SIZE * 1.5f;
            float minY = -World::HALF_HEIGHT + World::TILE_SIZE * 1.5f;
            float maxY = World::HALF_HEIGHT - World::TILE_SIZE * 1.5f;

            float spawnX = minX + (AERandFloat() * (maxX - minX));
            float spawnY = minY + (AERandFloat() * (maxY - minY));

            float differenceX = spawnX - player.pos_x;
            float differenceY = spawnY - player.pos_y;
            float distanceToPlayer = sqrt((differenceX * differenceX) + (differenceY * differenceY));

            if (distanceToPlayer < GameConfig::Enemy::SPAWN_SAFE_ZONE) {
                spawnX += GameConfig::Enemy::SPAWN_PUSH_DIST;
                spawnX = AEClamp(spawnX, minX, maxX);
            }

            // ensures it doesnt spawn in the trees
            int attempts = 0;
            while (World::isPointColliding(spawnX, spawnY) && attempts < 20) {
                spawnX = minX + (AERandFloat() * (maxX - minX));
                spawnY = minY + (AERandFloat() * (maxY - minY));
                attempts++;
            }

            newEnemy.pos = { spawnX, spawnY };
            newEnemy.velocity = { 0, 0 };
            newEnemy.alive = true;
            newEnemy.rotation = AERandFloat() * 360.0f;
            newEnemy.scale = isBigEnemy ? GameConfig::Enemy::SIZE_BIG : GameConfig::Enemy::SIZE_SMALL;

            int initialHP = (int)((isBigEnemy ? GameConfig::Enemy::HP_BIG : GameConfig::Enemy::HP_SMALL)*mult);
            newEnemy.hp = initialHP;
            newEnemy.maxhp = initialHP;
            newEnemy.enemtype = PASSIVE;
            break;
        }
    }
}

// ~ Brief: Spawn a kamikaze (ATTACK) enemy at a safe world position.
//          Kamikazes detect the player at range and charge directly at them.
//          HP scales with the current wave multiplier.
void SpawnAttackEnemy(shape player) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    for (auto& newEnemy : enemyPool) {
        if (!newEnemy.alive) {
            // Spawn within world borders (one tile inset from each edge)
            float minX = -World::HALF_WIDTH + World::TILE_SIZE * 1.5f;
            float maxX = World::HALF_WIDTH - World::TILE_SIZE * 1.5f;
            float minY = -World::HALF_HEIGHT + World::TILE_SIZE * 1.5f;
            float maxY = World::HALF_HEIGHT - World::TILE_SIZE * 1.5f;

            float spawnX = minX + (AERandFloat() * (maxX - minX));
            float spawnY = minY + (AERandFloat() * (maxY - minY));

            float differenceX = spawnX - player.pos_x;
            float differenceY = spawnY - player.pos_y;
            float distanceToPlayer = sqrt((differenceX * differenceX) + (differenceY * differenceY));

            if (distanceToPlayer < GameConfig::Enemy::SPAWN_SAFE_ZONE) {
                spawnX += GameConfig::Enemy::SPAWN_PUSH_DIST;
                spawnX = AEClamp(spawnX, minX, maxX);
            }

            // ensures it doesnt spawn in the trees
            int attempts = 0;
            while (World::isPointColliding(spawnX, spawnY) && attempts < 20) {
                spawnX = minX + (AERandFloat() * (maxX - minX));
                spawnY = minY + (AERandFloat() * (maxY - minY));
                attempts++;
            }

            newEnemy.pos = { spawnX, spawnY };
            newEnemy.velocity = { 0, 0 };
            newEnemy.alive = true;
            newEnemy.rotation = AERandFloat() * 360.0f;
            newEnemy.scale = GameConfig::Enemy::SIZE_BIG;
            newEnemy.enemtype = ATTACK;

            int initialHP = (int)((GameConfig::Enemy::HP_BIG)*mult);
            newEnemy.hp = initialHP;
            newEnemy.maxhp = initialHP;
            break;
        }
    }
}

// ~ Brief: Spawn a shooter enemy at a safe world position.
//          Shooters maintain engagement distance and fire projectiles at the player.
//          Starts with a 2-second initial fire cooldown. HP scales with wave multiplier.
void SpawnShooterEnemy(shape player) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    for (auto& newEnemy : enemyPool) {
        if (!newEnemy.alive) {
            // Spawn within world borders (one tile inset from each edge)
            float minX = -World::HALF_WIDTH + World::TILE_SIZE * 1.5f;
            float maxX = World::HALF_WIDTH - World::TILE_SIZE * 1.5f;
            float minY = -World::HALF_HEIGHT + World::TILE_SIZE * 1.5f;
            float maxY = World::HALF_HEIGHT - World::TILE_SIZE * 1.5f;

            float spawnX = minX + (AERandFloat() * (maxX - minX));
            float spawnY = minY + (AERandFloat() * (maxY - minY));

            float differenceX = spawnX - player.pos_x;
            float differenceY = spawnY - player.pos_y;
            float distanceToPlayer = sqrt((differenceX * differenceX) + (differenceY * differenceY));

            if (distanceToPlayer < GameConfig::Enemy::SPAWN_SAFE_ZONE) {
                spawnX += GameConfig::Enemy::SPAWN_PUSH_DIST;
                spawnX = AEClamp(spawnX, minX, maxX);
            }

            // ensures it doesnt spawn in the trees
            int attempts = 0;
            while (World::isPointColliding(spawnX, spawnY) && attempts < 20) {
                spawnX = minX + (AERandFloat() * (maxX - minX));
                spawnY = minY + (AERandFloat() * (maxY - minY));
                attempts++;
            }

            newEnemy.pos = { spawnX, spawnY };
            newEnemy.velocity = { 0, 0 };
            newEnemy.alive = true;
            newEnemy.rotation = AERandFloat() * 360.0f;
            newEnemy.scale = GameConfig::Enemy::SIZE_BIG;
            newEnemy.enemtype = SHOOTER;

            newEnemy.cooldown = 2.0f; // 2 seconds initial cooldown
            int initialHP = (int)((GameConfig::Enemy::HP_BIG)*mult);
            newEnemy.hp = initialHP;
            newEnemy.maxhp = initialHP;
            break;
        }
    }
}

// =============================================================================
// PHYSICS
// =============================================================================

// ~ Brief: Update physics for all active enemies each frame.
//          ATTACK (kamikaze) enemies detect and chase the player once in range,
//          decelerating toward them with a wave-scaled speed cap.
//          SHOOTER enemies maintain a preferred distance and fire on a cooldown.
//          All enemies apply separation forces to prevent stacking,
//          are pushed out of walls each frame, and are reset when HP reaches zero,
//          awarding XP proportional to their size and the wave multiplier.
void updateEnemyPhysics(shape& player, float deltaTime) {
    f32 mult = (1 + (currentWave / 5 * 0.5f)); // multiplier

    for (auto& currentEnemy : enemyPool) { // for entire enemy pool
        if (!currentEnemy.alive) continue; // skip not active enemies

        // --- KAMIKAZE LOGIC ---
        if (currentEnemy.enemtype == ATTACK) {
            AEVec2 PlayerPos = { player.pos_x, player.pos_y };
            AEVec2 EnemyPos = { currentEnemy.pos };
            AEVec2 dir = {};
            AEVec2Sub(&dir, &PlayerPos, &EnemyPos);

            f32 hyp = sqrt(dir.x * dir.x + dir.y * dir.y); // distance towards player

            if (hyp <= 600) {
                currentEnemy.detect = true;
            }

            if (currentEnemy.detect || currentEnemy.hp < currentEnemy.maxhp) {

                if ((dir.x * dir.x) + (dir.y * dir.y) > GameConfig::MOUSE_JITTER_THRESHOLD) {
                    float targetAngle = atan2f(dir.y, dir.x) * (180.f / PI);
                    float angleDifference = targetAngle - currentEnemy.rotation;
                    while (angleDifference > 180.f) angleDifference -= 360.f;
                    while (angleDifference < -180.f) angleDifference += 360.f;
                    currentEnemy.rotation += angleDifference * 0.1f;
                }

                dir.x /= hyp; // normalize to get direction to player
                dir.y /= hyp;

                f32 speedmult = (mult <= 3 ? mult : 3); // speed cap of enemy

                // move enemy with friction and based on delta time
                currentEnemy.velocity.x += dir.x * 500 * speedmult * deltaTime;
                currentEnemy.velocity.y += dir.y * 500 * speedmult * deltaTime;
                currentEnemy.velocity.x *= GameConfig::Enemy::FRICTION;
                currentEnemy.velocity.y *= GameConfig::Enemy::FRICTION;
                currentEnemy.pos.x += currentEnemy.velocity.x * deltaTime;
                currentEnemy.pos.y += currentEnemy.velocity.y * deltaTime;
            }

            //push out of trees and walls
            World::PushOutOfWalls(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale * 0.5f);
        }

        // --- SHOOTER LOGIC ---
        if (currentEnemy.enemtype == SHOOTER) {
            AEVec2 PlayerPos = { player.pos_x, player.pos_y };
            AEVec2 EnemyPos = { currentEnemy.pos.x, currentEnemy.pos.y };
            AEVec2 dir = {};
            AEVec2Sub(&dir, &PlayerPos, &EnemyPos);

            f32 hyp = sqrt(dir.x * dir.x + dir.y * dir.y);

            // 1. Aim at the player (Rotation visual)
            if ((dir.x * dir.x) + (dir.y * dir.y) > GameConfig::MOUSE_JITTER_THRESHOLD) {
                float targetAngle = atan2f(dir.y, dir.x) * (180.f / PI);
                float angleDifference = targetAngle - currentEnemy.rotation;
                while (angleDifference > 180.f) angleDifference -= 360.f;
                while (angleDifference < -180.f) angleDifference += 360.f;
                currentEnemy.rotation += angleDifference * 0.1f;
            }

            // Normalize direction
            if (hyp > 0) {
                dir.x /= hyp;
                dir.y /= hyp;
            }

            // 2. Keep Distance (move closer only if further than 400 units)
            if (hyp > 400.0f) {
                currentEnemy.velocity.x += dir.x * 300 * deltaTime;
                currentEnemy.velocity.y += dir.y * 300 * deltaTime;
            }

            // 3. Shooting Logic
            currentEnemy.cooldown -= deltaTime;

            if (currentEnemy.cooldown <= 0.0f && hyp < 800.0f) {
                currentEnemy.cooldown = 1.5f; // Reset cooldown

                for (auto& eBullet : enemyBulletList) {
                    if (!eBullet.isActive) {
                        SFX::playSFX(SFX_ENEMY_SHOOT);
                        eBullet.isActive = true;
                        eBullet.posX = currentEnemy.pos.x;
                        eBullet.posY = currentEnemy.pos.y;
                        eBullet.directionX = dir.x;
                        eBullet.directionY = dir.y;
                        eBullet.speed = 400.0f;
                        eBullet.size = 15.0f;
                        eBullet.damagemul = 1.0f * mult;
                        break;
                    }
                }
            }
        }

        // Push Logic
        AEVec2 separationForce = { 0, 0 };
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

        // push all enemy types out of walls
        World::PushOutOfWalls(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale * 0.5f);

        if (currentEnemy.hp <= 0) {
            float xp_multiplier = calculate_max_stats(4);
            float baseReward = (currentEnemy.maxhp >= (int)(GameConfig::Enemy::HP_BIG * mult)) ? 60.0f : 10.0f;
            float finalReward = baseReward * xp_multiplier;
            player_init.current_xp += finalReward;
            TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale * 1.5f);
            TriggerXpPopup(finalReward);
            ResetEnemy(&currentEnemy);
        }
    }
}

// ~ Brief: Move all active enemy bullets along their direction each frame.
//          Deactivates bullets that collide with world geometry (walls or trees),
//          triggering a bullet impact effect at the point of contact.
void updateEnemyBullets(float deltaTime) {
    for (auto& eBullet : enemyBulletList) {
        if (!eBullet.isActive) continue;
        eBullet.posX += eBullet.directionX * eBullet.speed * deltaTime;
        eBullet.posY += eBullet.directionY * eBullet.speed * deltaTime;

        float edgeX = eBullet.posX + (eBullet.size * eBullet.directionX);
        float edgeY = eBullet.posY + (eBullet.size * eBullet.directionY);

        if (World::isPointColliding(edgeX, edgeY)) {
            TriggerBulletImpact(eBullet.posX, eBullet.posY, eBullet.directionX, eBullet.directionY);
            eBullet.isActive = false;
            continue;
        }
    }
}

// =============================================================================
// HUD / INDICATORS
// =============================================================================

// ~ Brief: Draw a small colored arrow at the screen edge pointing toward each
//          off-screen enemy, minion, and boss. The arrow is clamped to the nearest
//          screen edge with padding, and color-coded by enemy type:
//          purple = PASSIVE/minion, blue = ATTACK, red = SHOOTER, orange = boss.
void DrawEnemyIndicators(shape& player, AEGfxVertexList* MeshTriangle) {
    float screenW = (float)AEGfxGetWindowWidth() * 0.5f;
    float screenH = (float)AEGfxGetWindowHeight() * 0.5f;
    float padding = 30.f; // distance from screen edge
    float arrowSize = 30.f;

    // ~ Brief: Draw one off-screen indicator for a given world position and color.
    //          Returns early if the target is already visible on screen.
    auto DrawIndicator = [&](AEVec2 enemyPos, float r, float g, float b) {
        float dx = enemyPos.x - player.pos_x;
        float dy = enemyPos.y - player.pos_y;

        // Check if enemy is off screen
        bool offScreen = (fabsf(dx) > screenW || fabsf(dy) > screenH);
        if (!offScreen) return;

        // Angle from player to enemy
        float angle = atan2f(dy, dx);

        // Clamp to screen edge
        float clampedX, clampedY;
        if (fabsf(dx) / screenW > fabsf(dy) / screenH) {
            // Clamp to left or right edge
            float sign = dx > 0 ? 1.f : -1.f;
            clampedX = player.pos_x + sign * (screenW - padding);
            clampedY = player.pos_y + dy * (screenW - padding) / fabsf(dx);
        }
        else {
            // Clamp to top or bottom edge
            float sign = dy > 0 ? 1.f : -1.f;
            clampedX = player.pos_x + dx * (screenH - padding) / fabsf(dy);
            clampedY = player.pos_y + sign * (screenH - padding);
        }

        // Clamp Y within screen bounds too
        clampedX = max(player.pos_x - screenW + padding, min(player.pos_x + screenW - padding, clampedX));
        clampedY = max(player.pos_y - screenH + padding, min(player.pos_y + screenH - padding, clampedY));

        AEGfxSetColorToMultiply(r, g, b, 1.f);
        Gfx::printMesh(MeshTriangle,
            { clampedX, clampedY },
            { arrowSize, arrowSize },
            angle);
        };

    // Regular enemies
    for (const auto& enemy : enemyPool) {
        if (!enemy.alive) continue;
        if (enemy.enemtype == PASSIVE)
            DrawIndicator(enemy.pos, 0.6f, 0.2f, 0.6f); // purple
        else if (enemy.enemtype == ATTACK)
            DrawIndicator(enemy.pos, 0.2f, 0.2f, 0.8f); // blue
        else if (enemy.enemtype == SHOOTER)
            DrawIndicator(enemy.pos, 0.8f, 0.2f, 0.2f); // red
    }

    // Minions
    for (const auto& minion : minionPool) {
        if (!minion.alive) continue;
        DrawIndicator(minion.pos, 0.6f, 0.2f, 0.6f); // purple
    }

    // Boss — larger, distinct color
    if (currentboss.alive) {
        DrawIndicator(currentboss.pos, 1.0f, 0.5f, 0.0f); // orange
    }
}

