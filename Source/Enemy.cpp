#include "MasterHeader.h"

std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;
f64 enemySpawnTimer = 0;
extern int currentWave;
BulletObj enemyBulletList[GameConfig::MAX_BULLETS_COUNT];

AEGfxTexture* pEnemyTex[4] = { nullptr, nullptr, nullptr, nullptr };
AEGfxVertexList* pEnemyMesh = nullptr;
AEGfxTexture* pEnemyBulletTex = nullptr;
AEGfxVertexList* pEnemyBulletMesh = nullptr;

//create an array of textures
const char* enemyTextures[4] = {
    "./Assets/smallbox.png",    // PASSIVE small
    "./Assets/bigbox.png",      // PASSIVE big
    "./Assets/kamikaze.png",   // ATTACK (kamikaze)
    "./Assets/shooter.png"   // SHOOTER
};

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

void ResetEnemy(Enemies* enemyToReset) {
    enemyToReset->alive = false;
    enemyToReset->pos.x = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->pos.y = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->scale = 0;
    enemyToReset->hp = 0;
}

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

void EnemySpawner(shape& player, float deltaTime) {
    enemySpawnTimer += deltaTime;
    if (enemySpawnTimer >= GameConfig::Enemy::SPAWN_INTERVAL) {
        bool spawnBig = (AERandFloat() * 10.0f) < 4.0f;
        SpawnOneEnemy(spawnBig, player);
        if (player_init.player_level >= 5)
        {
            SpawnAttackEnemy(player);
            SpawnShooterEnemy(player);
        }
        enemySpawnTimer = 0;
    }
}

void updateEnemyPhysics(shape& player, float deltaTime) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));

    for (auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive) continue;

        if (currentEnemy.enemtype == ATTACK) {
            AEVec2 PlayerPos = { player.pos_x, player.pos_y };
            AEVec2 EnemyPos = { currentEnemy.pos };
            AEVec2 dir = {};
            AEVec2Sub(&dir, &PlayerPos, &EnemyPos);

            f32 hyp = sqrt(dir.x * dir.x + dir.y * dir.y);

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

                dir.x /= hyp;
                dir.y /= hyp;

                f32 speedmult = (mult <= 3 ? mult : 3);

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
            TriggerXpPopup(finalReward);
            ResetEnemy(&currentEnemy);
        }
    }
}

void updateEnemyBullets(float deltaTime) {
    for (auto& eBullet : enemyBulletList) {
        if (!eBullet.isActive) continue;
        eBullet.posX += eBullet.directionX * eBullet.speed * deltaTime;
        eBullet.posY += eBullet.directionY * eBullet.speed * deltaTime;

        float edgeX = eBullet.posX + (eBullet.size * eBullet.directionX);
        float edgeY = eBullet.posY + (eBullet.size * eBullet.directionY);

        if (World::isPointColliding(edgeX, edgeY)) {
            eBullet.isActive = false;
            continue;
        }
    }
}

void FreeEnemies() {
    for (int i = 0; i < 4; ++i) {
        if (pEnemyTex[i]) { AEGfxTextureUnload(pEnemyTex[i]); pEnemyTex[i] = nullptr; }
    }
    if (pEnemyMesh) { AEGfxMeshFree(pEnemyMesh); pEnemyMesh = nullptr; }
    if (pEnemyBulletTex) { AEGfxTextureUnload(pEnemyBulletTex);  pEnemyBulletTex = nullptr; }
    if (pEnemyBulletMesh) { AEGfxMeshFree(pEnemyBulletMesh);      pEnemyBulletMesh = nullptr; }
}