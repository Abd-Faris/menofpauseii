#include "MasterHeader.h"




Boss boss;

void SpawnBoss(BossType type, shape& player) {
	
    float spawnX = player.pos_x + 400.f;
    float spawnY = player.pos_y + 400.f;

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
        boss.hp = 500;
        boss.maxhp = 500;
        boss.xp = 200;
        boss.chaseSpeed = 150.f;
        boss.lungeSpeed = 1500.f;
        boss.idleDuration = 1.5f;
        boss.telegraphDuration = 0.6f;
        boss.lungeDuration = 0.75f;
        boss.cooldownDuration = 0.8f;
    }
    else if (type == BOSS2) {
        // Faster, more aggressive
        boss.scale = GameConfig::Enemy::SIZE_BIG * 2.5f;
        boss.hp = 800;
        boss.maxhp = 800;
        boss.xp = 400;
        boss.chaseSpeed = 220.f;
        boss.lungeSpeed = 1800.f;
        boss.idleDuration = 1.0f;
        boss.telegraphDuration = 0.4f;
        boss.lungeDuration = 0.2f;
        boss.cooldownDuration = 0.6f;
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

void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime) {
    if (!boss.alive) return;

    AEVec2 toPlayer = { player.pos_x - boss.pos.x,
                        player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    boss.stateTimer += deltaTime;

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

        //// Keep smoothly rotating toward player during windup
        //if (dist > 1.f) {
        //    float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
        //    float angleDiff = targetRotation - boss.rotation;
        //    while (angleDiff > 180.f) angleDiff -= 360.f;
        //    while (angleDiff < -180.f) angleDiff += 360.f;
        //    boss.rotation += angleDiff * 8.0f * deltaTime;
        //}

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
    Gfx::printMesh(MeshRect, { player.pos_x + 100, player.pos_y + 400.f}, { barWidth, 50.f}, 0.f);

    AEGfxSetColorToMultiply(1.f, 0.f, 0.f, 1.f);  // foreground
    float filledWidth = barWidth * hpPct;
    Gfx::printMesh(MeshRect,
        { player.pos_x + 100 - (barWidth - filledWidth) / 2.f, player.pos_y + 400 },
        { filledWidth, 50.f }, 0.f);
}