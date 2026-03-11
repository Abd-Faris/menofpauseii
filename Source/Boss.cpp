#include "MasterHeader.h"

std::array<Enemies, MAX_MINIONS_COUNT> minionPool;
extern int currentWave;

Boss boss;

void SpawnBoss(BossType type, shape& player) {
	
    float spawnX = player.pos_x + 400.f;
    float spawnY = player.pos_y + 400.f;
    float mult = (1 + (currentWave / 5 * 0.5f));

    boss.pos = { spawnX, spawnY };
    boss.velocity = { 0, 0 };
    boss.alive = true;
    boss.rotation = 0.f;
    boss.bosstype = type;
    boss.state = BossState::IDLE;
    boss.stateTimer = 0.f;
    boss.lungeDirection = { 0, 0 };

    if (type == BOSS1) {
        boss.scale = GameConfig::Enemy::SIZE_BIG * 2.f;
        boss.hp = static_cast<int>(500.f * mult);
        boss.maxhp = static_cast<int>(500.f * mult);
        boss.xp = static_cast<int>(200.f * mult);
        boss.chaseSpeed = 150.f;
        boss.lungeSpeed = 1500.f;
        boss.idleDuration = 1.5f;
        boss.telegraphDuration = 0.6f;
        boss.lungeDuration = 0.75f;
        boss.cooldownDuration = 0.8f;
    }
    else if (type == BOSS2) {
        // Faster, more aggressive
    boss.scale            = GameConfig::Enemy::SIZE_BIG * 3.f;
    boss.hp               = static_cast<int>(800.f * mult);
    boss.xp               = static_cast<int>(400.f * mult);
    boss.idleDuration     = 999.f; // never lunges
    boss.telegraphDuration = 999.f;
    boss.lungeDuration    = 999.f;
    boss.cooldownDuration = 999.f;
    boss.spawnInterval    = 5.f;
    }

    else if (type == BOSS3) {
        boss.scale = GameConfig::Enemy::SIZE_BIG * 2.f;
        boss.hp = 600;
        boss.xp = 300;
        boss.idleDuration = 999.f; // stationary like BOSS2
        boss.chaseSpeed = 250.f;
        boss.telegraphDuration = 999.f;
        boss.lungeDuration = 999.f;
        boss.cooldownDuration = 999.f;
        boss.attackDuration = 3.f;
        boss.betweenAttackDelay = 2.f;
    }

    boss.maxhp = boss.hp;
}

void BossShootRing(Boss& boss) {
    float angleStep = (2.f * PI) / boss.bulletCount;
    for (int i = 0; i < boss.bulletCount; i++) {
        float angle = angleStep * i;
        for (auto& boolet : enemyBulletList) {
            if (boolet.isActive) continue;

            boolet.isActive = true;
            boolet.posX = boss.pos.x;
            boolet.posY = boss.pos.y;
            boolet.directionX = cosf(angle);
            boolet.directionY = sinf(angle);
            boolet.speed = GameConfig::Bullet::BASE_SPEED;
            boolet.size = GameConfig::Bullet::DEFAULT_SIZE;
            boolet.damagemul = 0.5f;
            break;
        }
    }
}

void Boss3Spiral(Boss& boss, float deltaTime) {
    float spiralSpeed = 10.f;
    float bulletSpeed = GameConfig::Bullet::BASE_SPEED * 0.5f;
    int   arms = 3;  // number of arms

    boss.shootTimer += deltaTime;
    if (boss.shootTimer < 0.15f) return;
    boss.shootTimer = 0.f;

    float angleStep = (2.f * PI) / arms; // evenly space the arms

    for (int i = 0; i < arms; i++) {
        float angle = boss.spiralAngle + angleStep * i; // offset each arm

        for (auto& boolet : enemyBulletList) {
            if (boolet.isActive) continue;

            boolet.isActive = true;
            boolet.posX = boss.pos.x;
            boolet.posY = boss.pos.y;
            boolet.directionX = cosf(angle);
            boolet.directionY = sinf(angle);
            boolet.speed = bulletSpeed;
            boolet.size = 13.f;
            boolet.damagemul = 0.5f;
            break;
        }
    }

    boss.spiralAngle += spiralSpeed * deltaTime;
}

void Boss3AimedShot(Boss& boss, shape& player) {
    AEVec2 toPlayer = { player.pos_x - boss.pos.x,
                        player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    if (dist < 1.f) return;

    // Fire a tight cluster of 3 bullets straight at the player
    float baseAngle = atan2f(toPlayer.y, toPlayer.x);
    float spreadStep = 0.15f; // radians between each bullet in the cluster
    int   count = 3;

    for (int i = 0; i < count; i++) {
        float angle = baseAngle + spreadStep * (i - count / 2);
        for (auto& boolet : enemyBulletList) {
            if (boolet.isActive) continue;

            boolet.isActive = true;
            boolet.posX = boss.pos.x;
            boolet.posY = boss.pos.y;
            boolet.directionX = cosf(angle);
            boolet.directionY = sinf(angle);
            boolet.speed = GameConfig::Bullet::BASE_SPEED * 0.8;
            boolet.size = 15.f;  // slightly bigger than spiral bullets
            boolet.damagemul = 1.0f;  // full damage
            break;
        }
    }
}

void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime) {
    if (!boss.alive) return;

    AEVec2 toPlayer = { player.pos_x - boss.pos.x,
                        player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    boss.stateTimer += deltaTime;

    if (boss.bosstype == BOSS1) {
    switch (boss.state) {

    case BossState::IDLE:
        if (dist > 1.f) {
            boss.velocity.x += (toPlayer.x / dist) * boss.chaseSpeed * deltaTime;
            boss.velocity.y += (toPlayer.y / dist) * boss.chaseSpeed * deltaTime;

            // Smoothly rotate toward player
            float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float angleDiff = targetRotation - boss.rotation;
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;
            boss.rotation += angleDiff * 8.0f * deltaTime; // 8.0f = turn speed
        }
        boss.velocity.x *= GameConfig::Enemy::FRICTION;
        boss.velocity.y *= GameConfig::Enemy::FRICTION;
        boss.pos.x += boss.velocity.x * deltaTime;
        boss.pos.y += boss.velocity.y * deltaTime;

        if (boss.stateTimer >= boss.idleDuration) {
            boss.state = BossState::TELEGRAPHING;
            boss.stateTimer = 0.f;
            if (dist > 1.f)
                boss.lungeDirection = { toPlayer.x / dist, toPlayer.y / dist };
        }
        break;

    case BossState::TELEGRAPHING:
        boss.velocity = { 0, 0 };

        if (boss.stateTimer >= boss.telegraphDuration) {
            boss.state = BossState::LUNGING;
            boss.stateTimer = 0.f;
        }
        break;

    case BossState::LUNGING: {
        float t = boss.stateTimer / boss.lungeDuration;
        float speed = boss.lungeSpeed * (1.0f - t);
        boss.velocity.x = boss.lungeDirection.x * speed;
        boss.velocity.y = boss.lungeDirection.y * speed;
        boss.pos.x += boss.velocity.x * deltaTime;
        boss.pos.y += boss.velocity.y * deltaTime;

        // Lock rotation to lunge direction — never updates from here
        boss.rotation = atan2f(boss.lungeDirection.y, boss.lungeDirection.x) * (180.f / PI);

        if (boss.stateTimer >= boss.lungeDuration) {
            boss.state = BossState::COOLDOWN;
            boss.stateTimer = 0.f;
            boss.velocity = { 0, 0 };
        }
        break;
    }

    case BossState::COOLDOWN:

        boss.shootTimer += deltaTime;

        // Shoot once shortly after the lunge lands
        if (!boss.hasShot && boss.shootTimer >= 0.1f) {
            BossShootRing(boss);
            boss.shootTimer = boss.cooldownDuration; // prevent shooting again this cooldown
            boss.hasShot = true;
        }

        if (boss.stateTimer >= boss.cooldownDuration) {
            boss.state = BossState::IDLE;
            boss.stateTimer = 0.f;
            boss.shootTimer = 0.f; // reset for next lunge
            boss.hasShot = false;
        }
        break;
    }
    }
    if (boss.bosstype == BOSS2) {
        // Slowly rotate to face player
        if (dist > 1.f) {
            float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float angleDiff = targetRotation - boss.rotation;
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;
            boss.rotation += angleDiff * 5.0f * deltaTime;
        }

        // Spawn tracking enemies on interval
        boss.spawnTimer += deltaTime;
        if (boss.spawnTimer >= boss.spawnInterval) {
            BossSpawnMinion(boss, player); //
            boss.spawnTimer = 0.f;
        }

        // Death check
        if (boss.hp <= 0) {
            player_init.current_xp += boss.xp;
            TriggerXpPopup((float)boss.xp);
            TriggerExplosion(boss.pos.x, boss.pos.y, boss.scale);
            boss.alive = false;
        }

        return; // skip lunge state machine entirely
    }
    if (boss.bosstype == BOSS3) {
        // Face player
        AEVec2 toPlayer = { player.pos_x - boss.pos.x,
                                player.pos_y - boss.pos.y };
        float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

        // Chase player
        if (dist > 1.f) {
            boss.velocity.x += (toPlayer.x / dist) * boss.chaseSpeed * deltaTime;
            boss.velocity.y += (toPlayer.y / dist) * boss.chaseSpeed * deltaTime;

            // Smooth rotation toward player
            float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float angleDiff = targetRotation - boss.rotation;
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;
            boss.rotation += angleDiff * 5.f * deltaTime;
        }
        boss.velocity.x *= GameConfig::Enemy::FRICTION;
        boss.velocity.y *= GameConfig::Enemy::FRICTION;
        boss.pos.x += boss.velocity.x * deltaTime;
        boss.pos.y += boss.velocity.y * deltaTime;


        // Between attacks — pick next attack randomly
        if (boss.currentAttack == Boss3Attack::NONE) {
            boss.betweenAttackTimer += deltaTime;
            if (boss.betweenAttackTimer >= boss.betweenAttackDelay) {
                // Randomly pick spiral or aimed
                boss.currentAttack = (AERandFloat() > 0.5f)
                    ? Boss3Attack::SPIRAL
                    : Boss3Attack::AIMED;
                boss.attackTimer = 0.f;
                boss.betweenAttackTimer = 0.f;
                boss.spiralAngle = 0.f; // reset spiral each time
            }
        }

        // Run the selected attack
        if (boss.currentAttack == Boss3Attack::SPIRAL) {
            boss.attackTimer += deltaTime;
            Boss3Spiral(boss, deltaTime);  // <-- pass deltaTime

            if (boss.attackTimer >= boss.attackDuration) {
                boss.currentAttack = Boss3Attack::NONE;
                boss.shootTimer = 0.f; // reset for next attack
            }
        }
        else if (boss.currentAttack == Boss3Attack::AIMED) {
            boss.attackTimer += deltaTime;

            // Fire a burst every 0.4s during the attack window
            if (boss.attackTimer >= boss.attackDuration) {
                boss.currentAttack = Boss3Attack::NONE;
            }
            else if (fmodf(boss.attackTimer, 0.4f) < deltaTime) {
                Boss3AimedShot(boss, player);
            }
        }

        // Death check
        if (boss.hp <= 0) {
            player_init.current_xp += boss.xp;
            TriggerXpPopup((float)boss.xp);
            TriggerExplosion(boss.pos.x, boss.pos.y, boss.scale);
            boss.alive = false;
        }

        return;
    }

    // Death check
    if (boss.hp <= 0) {
        player_init.current_xp += boss.xp;
        TriggerXpPopup((float)boss.xp);
        TriggerExplosion(boss.pos.x, boss.pos.y, boss.scale);
        boss.alive = false;
    }
}
void BossCollision(Boss& boss, shape &player, bool orbitActive, float orbitPosX, float orbitPosY) {
    if (!boss.alive) return;

    // Bullets hit boss
    for (auto& boolet : bulletList) {
        if (!boolet.isActive) continue;

        float dx = boolet.posX - boss.pos.x;
        float dy = boolet.posY - boss.pos.y;
        float distSq = dx * dx + dy * dy;
        float colRadius = (boss.scale * GameConfig::Enemy::HITBOX_RATIO) + boolet.size;

        if (distSq < colRadius * colRadius) {
            float dmg = calculate_max_stats(1);
            boss.hp -= (int)(dmg * boolet.damagemul);
            boolet.isActive = false;
        }
    }

    // Orbit hits boss
    if (orbitActive) {
        float dx = orbitPosX - boss.pos.x;
        float dy = orbitPosY - boss.pos.y;
        float distSq = dx * dx + dy * dy;
        float orbitSize = 20.0f;
        float colRadius = (boss.scale * GameConfig::Enemy::HITBOX_RATIO) + orbitSize;

        if (distSq < colRadius * colRadius) {
            float dmg = calculate_max_stats(1);
            boss.hp -= (int)dmg;
        }
    }
    for (auto& enBullet : enemyBulletList) {
        if (!enBullet.isActive) continue;

        float differenceX = enBullet.posX - player.pos_x;
        float differenceY = enBullet.posY - player.pos_y;
        float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

        // Calculate collision radius (Player scale + bullet size)
        float collisionRadius = player.scale + enBullet.size;

        if (distanceSquared < (collisionRadius * collisionRadius)) {
            player_init.current_hp -= 10; // player
            enBullet.isActive = false;     // Destroy the enemy bullet


            TriggerExplosion(player.pos_x, player.pos_y, 20.0f);
        }
    }

    // Player touches boss — player takes damage, boss is fine
    float dx = player.pos_x - boss.pos.x;
    float dy = player.pos_y - boss.pos.y;
    float distSq = dx * dx + dy * dy;
    float colRadius = (boss.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;

    if (distSq < colRadius * colRadius) {
        player_init.current_hp -= boss.maxhp / 5;
    }
}

void DrawBoss(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle) {
    if (!boss.alive) return;

    // Flash white during telegraph
    bool telegraphing = (boss.state == BossState::TELEGRAPHING);
    AEGfxSetColorToMultiply(
        telegraphing ? 1.f : 0.8f,
        telegraphing ? 1.f : 0.0f,
        telegraphing ? 1.f : 0.0f,
        1.f
    );

    float rotRad = boss.rotation * (PI / 180.f);
    Gfx::printMesh(MeshRect, boss.pos, { boss.scale, boss.scale }, rotRad);

}

void DrawBossHP(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle, shape& player) {

    // HP bar above boss
    float hpPct = (float)boss.hp / (float)boss.maxhp;
    float barWidth = 1000.f;
    float barY = boss.pos.y + boss.scale + 20.f;

    AEGfxSetColorToMultiply(0.3f, 0.f, 0.f, 1.f); // background
    Gfx::printMesh(MeshRect, { player.pos_x, player.pos_y + 400.f}, { barWidth, 50.f}, 0.f);

    AEGfxSetColorToMultiply(1.f, 0.f, 0.f, 1.f);  // foreground
    float filledWidth = barWidth * hpPct;
    Gfx::printMesh(MeshRect,
        { player.pos_x - (barWidth - filledWidth) / 2.f, player.pos_y + 400 },
        { filledWidth, 50.f }, 0.f);
}

void BossSpawnMinion(Boss& boss, shape& player) {
    float angleStep = (2.f * PI) / boss.minionCount;
    float radius = boss.scale + 40.f;

    for (int i = 0; i < boss.minionCount; i++) {
        float angle = angleStep * i;

        for (auto& newEnemy : minionPool) {
            if (!newEnemy.alive) {
                newEnemy.pos = { boss.pos.x + cosf(angle) * radius,
                                      boss.pos.y + sinf(angle) * radius };
                newEnemy.velocity = { 0, 0 };
                newEnemy.alive = true;
                newEnemy.rotation = angle * (180.f / PI); // face outward on spawn
                newEnemy.scale = GameConfig::Enemy::SIZE_BIG;
                newEnemy.enemtype = ATTACK;
                newEnemy.detect = true;

                int initialHP = 10;
                newEnemy.hp = initialHP;
                newEnemy.maxhp = initialHP;
                break;
            }
        }
    }
}

void updateMinionPhysics(shape& player, float deltaTime) {
    for (auto& currentEnemy : minionPool) {
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


                currentEnemy.velocity.x += dir.x * 2500 * deltaTime;
                currentEnemy.velocity.y += dir.y * 2500 * deltaTime;
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
            //float xp_multiplier = calculate_max_stats(4);
            //float baseReward = (currentEnemy.maxhp >= (int)GameConfig::Enemy::HP_BIG) ? 80.0f : 20.0f;
            //float finalReward = baseReward * xp_multiplier;
            //player_init.current_xp += finalReward;
            //TriggerXpPopup(finalReward);
            ResetEnemy(&currentEnemy);

        }
    }

}