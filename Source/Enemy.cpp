#include "MasterHeader.h"
#include "MasterHeader.h"

std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;
f64 enemySpawnTimer = 0;

BulletObj enemyBulletList[GameConfig::MAX_BULLETS_COUNT];

void ResetEnemy(Enemies* enemyToReset) {
    enemyToReset->alive = false;
    enemyToReset->pos.x = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->pos.y = GameConfig::OFF_SCREEN_COORD;
    enemyToReset->scale = 0;
    enemyToReset->hp = 0;
}

void SpawnOneEnemy(bool isBigEnemy, shape player) {
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

void SpawnAttackEnemy(shape player) {
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
            newEnemy.scale = GameConfig::Enemy::SIZE_BIG;
            newEnemy.enemtype = ATTACK;

            int initialHP = (int)(GameConfig::Enemy::HP_BIG);
            newEnemy.hp = initialHP;
            newEnemy.maxhp = initialHP;
            break;
        }
    }
}


void SpawnShooterEnemy(shape player) {
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
            newEnemy.scale = GameConfig::Enemy::SIZE_BIG;
            newEnemy.enemtype = SHOOTER;

            newEnemy.cooldown = 2.0f;//2 seconds initial cooldown
            int initialHP = (int)(GameConfig::Enemy::HP_BIG);
            newEnemy.hp = initialHP;
            newEnemy.maxhp = initialHP;
            break;
        }
    }
}

void EnemySpawner(shape &player, float deltaTime) {
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

void updateEnemyPhysics(shape &player, float deltaTime) {
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
                /*GfxText exclaim{ "!", 1, 255, 0, 0, 255, {currentEnemy.pos.x, currentEnemy.pos.y + 50.f} };
                Gfx::printText(exclaim, boldPixelsFont);*/
            }

            if (currentEnemy.detect || currentEnemy.hp < currentEnemy.maxhp) {

                /*f32 enemydir = atan2f(dir.y, dir.x) - HALF_PI;
                currentEnemy.rotation = enemydir;*/

                if ((dir.x * dir.x) + (dir.y * dir.y) > GameConfig::MOUSE_JITTER_THRESHOLD) {
                    float targetAngle = atan2f(dir.y, dir.x) * (180.f / PI);
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
        }
        // --- NEW: SHOOTER LOGIC ---
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

            // Normalize the direction vector so we can use it for movement and shooting
            if (hyp > 0) {
                dir.x /= hyp;
                dir.y /= hyp;
            }

            // 2. Keep Distance (Move closer only if further than 400 units)
            if (hyp > 400.0f) {
                currentEnemy.velocity.x += dir.x * 300 * deltaTime;
                currentEnemy.velocity.y += dir.y * 300 * deltaTime;
            }

            // 3. Shooting Logic
            currentEnemy.cooldown -= deltaTime; // Ensure your struct uses 'cooldown' here

            // If timer is up AND player is close enough to see (e.g., within 800 units), FIRE!
            if (currentEnemy.cooldown <= 0.0f && hyp < 800.0f) {
                currentEnemy.cooldown = 1.5f; // Reset cooldown to 1.5 seconds

                // Find an empty bullet slot
                for (auto& eBullet : enemyBulletList) {
                    if (!eBullet.isActive) {
                        eBullet.isActive = true;
                        eBullet.posX = currentEnemy.pos.x;
                        eBullet.posY = currentEnemy.pos.y;

                        // Shoot exactly in the direction of the player
                        eBullet.directionX = dir.x;
                        eBullet.directionY = dir.y;

                        eBullet.speed = 400.0f; // Speed of the red bullet
                        eBullet.size = 15.0f;
                        eBullet.damagemul = 1.0f;
                        break; // Stop after firing 1 bullet
                    }
                }
            }
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
            float xp_multiplier = calculate_max_stats(4);
            float baseReward = (currentEnemy.maxhp >= (int)GameConfig::Enemy::HP_BIG) ? 80.0f : 20.0f;
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