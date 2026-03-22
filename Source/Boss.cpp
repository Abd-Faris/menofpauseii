#include "MasterHeader.h"

std::array<Enemies, MAX_MINIONS_COUNT> minionPool;
extern int currentWave;

//globals
AEGfxTexture* pBossTex = nullptr;
AEGfxTexture* pMinionTex = nullptr;
AEGfxVertexList* pBossMesh = nullptr;

Boss currentboss;

void LoadBoss() {
    pBossTex = AEGfxTextureLoad("./Assets/boss.png");
    pMinionTex = AEGfxTextureLoad("./Assets/minion.png");

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBossMesh = AEGfxMeshEnd();
}

void SpawnBoss(BossType type, shape& player) {
    currentboss = {};
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    currentboss.pos = { player.pos_x + 400.f, player.pos_y + 400.f };
    currentboss.velocity = { 0, 0 };
    currentboss.alive = true;
    currentboss.bosstype = type;
    currentboss.state = BossState::IDLE;
    currentboss.currentAttack = Boss3Attack::NONE;

    switch (type) {
    case BOSS1:
        currentboss.scale = GameConfig::Enemy::SIZE_BIG * GameConfig::Boss::B1_SCALE;
        currentboss.hp = static_cast<int>(GameConfig::Boss::B1_BASE_HP * mult);
        currentboss.xp = GameConfig::Boss::B1_XP;
        currentboss.chaseSpeed = GameConfig::Boss::B1_CHASE_SPEED;
        currentboss.lungeSpeed = GameConfig::Boss::B1_LUNGE_SPEED;
        currentboss.idleDuration = GameConfig::Boss::B1_IDLE_DUR;
        currentboss.telegraphDuration = GameConfig::Boss::B1_TELEGRAPH_DUR;
        currentboss.lungeDuration = GameConfig::Boss::B1_LUNGE_DUR;
        currentboss.cooldownDuration = GameConfig::Boss::B1_COOLDOWN_DUR;
        currentboss.bulletCount = GameConfig::Boss::B1_BULLET_COUNT;
        break;

    case BOSS2:
        currentboss.scale = GameConfig::Enemy::SIZE_BIG * GameConfig::Boss::B2_SCALE;
        currentboss.hp = static_cast<int>(GameConfig::Boss::B2_BASE_HP * mult);
        currentboss.xp = GameConfig::Boss::B2_XP;
        currentboss.chaseSpeed = 0.f;
        currentboss.idleDuration = GameConfig::Boss::B2_IDLE_DUR;
        currentboss.telegraphDuration = GameConfig::Boss::B2_TELEGRAPH_DUR;
        currentboss.lungeDuration = GameConfig::Boss::B2_LUNGE_DUR;
        currentboss.cooldownDuration = GameConfig::Boss::B2_COOLDOWN_DUR;
        currentboss.minionCount = GameConfig::Boss::B2_MINION_COUNT;
        break;

    case BOSS3:
        currentboss.scale = GameConfig::Enemy::SIZE_BIG * GameConfig::Boss::B3_SCALE;
        currentboss.hp = static_cast<int>(GameConfig::Boss::B3_BASE_HP * mult);
        currentboss.xp = GameConfig::Boss::B3_XP;
        currentboss.chaseSpeed = GameConfig::Boss::B3_CHASE_SPEED;
        currentboss.idleDuration = GameConfig::Boss::B3_IDLE_DUR;
        currentboss.telegraphDuration = GameConfig::Boss::B3_TELEGRAPH_DUR;
        currentboss.lungeDuration = GameConfig::Boss::B3_LUNGE_DUR;
        currentboss.cooldownDuration = GameConfig::Boss::B3_COOLDOWN_DUR;
        currentboss.bulletCount = GameConfig::Boss::B3_SPIRAL_ARMS;
        break;

    case BOSS4:
        currentboss.scale = GameConfig::Enemy::SIZE_BIG * GameConfig::Boss::B4_SCALE;
        currentboss.hp = static_cast<int>(GameConfig::Boss::B4_BASE_HP * mult);
        currentboss.xp = GameConfig::Boss::B4_XP;
        currentboss.chaseSpeed = GameConfig::Boss::B4_CHASE_SPEED;
        currentboss.idleDuration = GameConfig::Boss::B4_IDLE_DUR;
        currentboss.telegraphDuration = GameConfig::Boss::B4_TELEGRAPH_DUR;
        currentboss.lungeDuration = GameConfig::Boss::B4_LUNGE_DUR;
        currentboss.cooldownDuration = GameConfig::Boss::B4_COOLDOWN_DUR;
        currentboss.gunFireRate = GameConfig::Boss::B4_GUN_FIRE_RATE;
        currentboss.laserSweepSpeed = GameConfig::Boss::B4_LASER_SWEEP;
        break;
    }

    currentboss.maxhp = currentboss.hp;
}

void BossShootRing(Boss& boss) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));
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
            boolet.damagemul = 0.5f * mult;
            break;
        }
    }
}

void Boss3Spiral(Boss& boss, float deltaTime) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    boss.shootTimer += deltaTime;
    if (boss.shootTimer < GameConfig::Boss::SPIRAL_FIRE_RATE) return;
    boss.shootTimer = 0.f;

    float angleStep = (2.f * PI) / boss.bulletCount;
    for (int i = 0; i < boss.bulletCount; i++) {
        float angle = boss.spiralAngle + angleStep * i;
        for (auto& boolet : enemyBulletList) {
            if (boolet.isActive) continue;
            boolet.isActive = true;
            boolet.posX = boss.pos.x;
            boolet.posY = boss.pos.y;
            boolet.directionX = cosf(angle);
            boolet.directionY = sinf(angle);
            boolet.speed = GameConfig::Bullet::BASE_SPEED * 0.5f;
            boolet.size = GameConfig::Boss::SPIRAL_BULLET_SIZE;
            boolet.damagemul = GameConfig::Boss::SPIRAL_DAMAGE_MUL * mult;
            break;
        }
    }
    boss.spiralAngle += GameConfig::Boss::SPIRAL_SPEED * deltaTime;
}

void Boss3AimedShot(Boss& boss, shape& player) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    AEVec2 toPlayer = { player.pos_x - boss.pos.x, player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    if (dist < 1.f) return;

    float baseAngle = atan2f(toPlayer.y, toPlayer.x);
    for (int i = 0; i < GameConfig::Boss::AIMED_COUNT; i++) {
        float angle = baseAngle + GameConfig::Boss::AIMED_SPREAD * (i - GameConfig::Boss::AIMED_COUNT / 2);
        for (auto& boolet : enemyBulletList) {
            if (boolet.isActive) continue;
            boolet.isActive = true;
            boolet.posX = boss.pos.x;
            boolet.posY = boss.pos.y;
            boolet.directionX = cosf(angle);
            boolet.directionY = sinf(angle);
            boolet.speed = GameConfig::Bullet::BASE_SPEED * 0.8f;
            boolet.size = GameConfig::Boss::AIMED_BULLET_SIZE;
            boolet.damagemul = 1.0f * mult;
            break;
        }
    }
}

AEVec2 GetGunPosition(Boss& boss, bool leftGun) {
    float rotRad = boss.gunAngle * (PI / 180.f);
    float cosR = cosf(rotRad);
    float sinR = sinf(rotRad);
    float halfScale = boss.scale * GameConfig::Boss::GUN_POSITION_RATIO;
    float sideSign = leftGun ? -1.f : 1.f;

    return {
        boss.pos.x + cosR * halfScale + (-sinR) * halfScale * sideSign,
        boss.pos.y + sinR * halfScale + cosR * halfScale * sideSign
    };
}

void Boss4ShootGuns(Boss& boss, shape& player, float deltaTime) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    boss.gunFireTimer += deltaTime;
    if (boss.gunFireTimer < boss.gunFireRate) return;
    boss.gunFireTimer = 0.f;

    // Update gunAngle toward player once per shot instead of every frame
    AEVec2 toPlayer = { player.pos_x - boss.pos.x, player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    if (dist < 1.f) return;

    float targetAngle = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
    float angleDiff = targetAngle - boss.gunAngle;
    while (angleDiff > 180.f) angleDiff -= 360.f;
    while (angleDiff < -180.f) angleDiff += 360.f;
    boss.gunAngle += angleDiff * 0.5f; // was 0.3f — snaps more toward player each shot

    float angle = boss.gunAngle * (PI / 180.f);

    AEVec2 guns[2] = { GetGunPosition(boss, true),
                       GetGunPosition(boss, false) };

    for (int i = 0; i < 2; i++) {
        for (auto& boolet : enemyBulletList) {
            if (boolet.isActive) continue;

            boolet.isActive = true;
            boolet.posX = guns[i].x;
            boolet.posY = guns[i].y;
            boolet.directionX = cosf(angle);
            boolet.directionY = sinf(angle);
            boolet.speed = GameConfig::Bullet::BASE_SPEED * 0.8f;
            boolet.size = 10.f;
            boolet.damagemul = 1.0f * mult;
            break;
        }
    }
}

void DrawBossLaser(Boss& boss, AEGfxVertexList* MeshRect) {
    if (boss.state == BossState::TELEGRAPHING && boss.currentAttack != Boss3Attack::LASER) return;
    if (!boss.laserActive && boss.state != BossState::TELEGRAPHING) return;

    AEVec2 guns[2] = { GetGunPosition(boss, true), GetGunPosition(boss, false) };
    float laserLength = GameConfig::Boss::LASER_LENGTH;
    float laserWidth = boss.laserActive
        ? GameConfig::Boss::LASER_WIDTH_ACTIVE
        : GameConfig::Boss::LASER_WIDTH_TELEGRAPH;

    for (int i = 0; i < 2; i++) {
        float midX = guns[i].x + cosf(boss.laserAngle) * laserLength * 0.5f;
        float midY = guns[i].y + sinf(boss.laserAngle) * laserLength * 0.5f;

        if (boss.laserActive)
            AEGfxSetColorToMultiply(1.0f, 0.2f, 0.0f, 1.f);
        else
            AEGfxSetColorToMultiply(0.60f, 0.0f, 0.0f, 0.2f);

        Gfx::printMesh(MeshRect, { midX, midY }, { laserLength, laserWidth }, boss.laserAngle);
    }
}

void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime) {
    if (!boss.alive) return;

    f32 mult = (1 + (currentWave / 5 * 0.5f));

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

    //push out of trees and walls
    World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);

    }
    if (boss.bosstype == BOSS2) {
        // Always face player
        if (dist > 1.f) {
            float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float angleDiff = targetRotation - boss.rotation;
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;
            boss.rotation += angleDiff * 5.0f * deltaTime;
        }

        boss.stateTimer += deltaTime;

        switch (boss.state) {
        case BossState::IDLE:
            if (boss.stateTimer >= boss.idleDuration) {
                boss.state = BossState::TELEGRAPHING;
                boss.stateTimer = 0.f;
            }
            break;

        case BossState::TELEGRAPHING:
            // Just flash — no movement
            if (boss.stateTimer >= boss.telegraphDuration) {
                boss.state = BossState::LUNGING;
                boss.stateTimer = 0.f;
            }
            break;

        case BossState::LUNGING:
            // Spawn the ring immediately then move on
            BossSpawnMinion(boss, player);
            boss.state = BossState::COOLDOWN;
            boss.stateTimer = 0.f;
            break;

        case BossState::COOLDOWN:
            if (boss.stateTimer >= boss.cooldownDuration) {
                boss.state = BossState::IDLE;
                boss.stateTimer = 0.f;
            }
            break;
        }

        // push boss out of trees/walls  <-- ADD THIS
        World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);

        if (boss.hp <= 0) {
            player_init.current_xp += boss.xp;
            TriggerXpPopup((float)boss.xp);
            TriggerExplosion(boss.pos.x, boss.pos.y, boss.scale);
            boss.alive = false;
        }
        return;
    }

    // -----------------------------------------------------------------------
    // BOSS3 — chases player, random attack after telegraph
    // -----------------------------------------------------------------------
    if (boss.bosstype == BOSS3) {
        // Always chase and face player
        if (dist > 1.f) {
            boss.velocity.x += (toPlayer.x / dist) * boss.chaseSpeed * deltaTime;
            boss.velocity.y += (toPlayer.y / dist) * boss.chaseSpeed * deltaTime;

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

        boss.stateTimer += deltaTime;

        switch (boss.state) {
        case BossState::IDLE:
            if (boss.stateTimer >= boss.idleDuration) {
                // Pick attack before telegraphing so draw code can hint which is coming
                boss.currentAttack = (AERandFloat() > 0.5f)
                    ? Boss3Attack::SPIRAL
                    : Boss3Attack::AIMED;
                boss.state = BossState::TELEGRAPHING;
                boss.stateTimer = 0.f;
                boss.spiralAngle = 0.f;
            }
            break;

        case BossState::TELEGRAPHING:
            if (boss.stateTimer >= boss.telegraphDuration) {
                boss.state = BossState::LUNGING; // attack phase
                boss.stateTimer = 0.f;
                boss.attackTimer = 0.f;
            }
            break;

        case BossState::LUNGING:
            // Run whichever attack was picked
            boss.attackTimer += deltaTime;

            if (boss.currentAttack == Boss3Attack::SPIRAL) {
                Boss3Spiral(boss, deltaTime);
            }
            else if (boss.currentAttack == Boss3Attack::AIMED) {
                if (fmodf(boss.attackTimer, 0.4f) < deltaTime) {
                    Boss3AimedShot(boss, player);
                }
            }

            if (boss.attackTimer >= boss.lungeDuration) {
                boss.state = BossState::COOLDOWN;
                boss.stateTimer = 0.f;
                boss.currentAttack = Boss3Attack::NONE;
                boss.shootTimer = 0.f;
            }
            break;

        case BossState::COOLDOWN:
            if (boss.stateTimer >= boss.cooldownDuration) {
                boss.state = BossState::IDLE;
                boss.stateTimer = 0.f;
            }
            break;
        }

        // push boss out of trees/walls
        World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);

        if (boss.hp <= 0) {
            player_init.current_xp += boss.xp;
            TriggerXpPopup((float)boss.xp);
            TriggerExplosion(boss.pos.x, boss.pos.y, boss.scale);
            boss.alive = false;
        }
        return;
    }
    if (boss.bosstype == BOSS4) {
        AEVec2 toPlayer = { player.pos_x - boss.pos.x,
                            player.pos_y - boss.pos.y };
        float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

        // Body slowly faces player
        if (dist > 1.f) {
            float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float angleDiff = targetRotation - boss.rotation;
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;
            boss.rotation += angleDiff * 3.f * deltaTime;
        }

        // Gun tracking — replace whatever was here before with this
        if (dist > 1.f) {
            float targetGunAngle = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float gunAngleDiff = targetGunAngle - boss.gunAngle;
            while (gunAngleDiff > 180.f) gunAngleDiff -= 360.f;
            while (gunAngleDiff < -180.f) gunAngleDiff += 360.f;

            float trackSpeed = 60.f;
            if (boss.state == BossState::LUNGING && boss.currentAttack == Boss3Attack::GUNS)
                trackSpeed = 40.f;  // slow but not frozen — tracks player gradually while shooting
            else if (boss.state == BossState::LUNGING && boss.currentAttack == Boss3Attack::LASER)
                trackSpeed = 0.f;   // laser drives gunAngle directly
            else if (boss.state == BossState::COOLDOWN)
                trackSpeed = 30.f;

            if (fabsf(gunAngleDiff) > 0.5f && trackSpeed > 0.f)
                boss.gunAngle += gunAngleDiff * trackSpeed * deltaTime;
        }

        // Chase
        boss.velocity.x += (dist > 1.f ? (toPlayer.x / dist) : 0.f) * boss.chaseSpeed * deltaTime;
        boss.velocity.y += (dist > 1.f ? (toPlayer.y / dist) : 0.f) * boss.chaseSpeed * deltaTime;
        boss.velocity.x *= GameConfig::Enemy::FRICTION;
        boss.velocity.y *= GameConfig::Enemy::FRICTION;
        boss.pos.x += boss.velocity.x * deltaTime;
        boss.pos.y += boss.velocity.y * deltaTime;

        boss.stateTimer += deltaTime;

        switch (boss.state) {
        case BossState::IDLE:
            boss.laserActive = false;
            if (boss.stateTimer >= boss.idleDuration) {
                // Pick attack
                boss.currentAttack = (AERandFloat() > 0.5f)
                    ? Boss3Attack::GUNS
                    : Boss3Attack::LASER;
                boss.state = BossState::TELEGRAPHING;
                boss.stateTimer = 0.f;

                // If laser, lock the starting angle toward player now
                if (boss.currentAttack == Boss3Attack::LASER && dist > 1.f) {
                    boss.laserAngle = atan2f(toPlayer.y, toPlayer.x);
                    boss.laserTargetAngle = boss.laserAngle;
                }
            }
            break;

        case BossState::TELEGRAPHING:
            if (boss.currentAttack == Boss3Attack::LASER) {
                // Slowly track player during telegraph — slow enough to dodge
                if (dist > 1.f) {
                    float targetAngle = atan2f(toPlayer.y, toPlayer.x);
                    float angleDiff = targetAngle - boss.laserAngle;
                    while (angleDiff > PI) angleDiff -= 2.f * PI;
                    while (angleDiff < -PI) angleDiff += 2.f * PI;
                    boss.laserAngle += angleDiff * 0.3f * deltaTime; // 0.3f = slow warning sweep

                    while (boss.laserAngle > PI) boss.laserAngle -= 2.f * PI;
                    while (boss.laserAngle < -PI) boss.laserAngle += 2.f * PI;
                }

                // Sync guns to match
                boss.gunAngle = boss.laserAngle * (180.f / PI);
            }

            if (boss.stateTimer >= boss.telegraphDuration) {
                boss.state = BossState::LUNGING;
                boss.stateTimer = 0.f;
                boss.attackTimer = 0.f;
                boss.gunFireTimer = 0.f;
                if (boss.currentAttack == Boss3Attack::LASER)
                    boss.laserActive = true;
            }
            break;

        case BossState::LUNGING:
            boss.attackTimer += deltaTime;

            if (boss.currentAttack == Boss3Attack::GUNS) {
                Boss4ShootGuns(boss, player, deltaTime);
            }
            else if (boss.currentAttack == Boss3Attack::LASER) {
                // Slowly sweep laser toward player
                if (dist > 1.f) {
                    float targetAngle = atan2f(toPlayer.y, toPlayer.x);
                    float angleDiff = targetAngle - boss.laserAngle;
                    while (angleDiff > PI) angleDiff -= 2.f * PI;
                    while (angleDiff < -PI) angleDiff += 2.f * PI;
                    boss.laserAngle += angleDiff * boss.laserSweepSpeed * deltaTime;

                    // Keep laserAngle normalized
                    while (boss.laserAngle > PI) boss.laserAngle -= 2.f * PI;
                    while (boss.laserAngle < -PI) boss.laserAngle += 2.f * PI;
                }

                // Also update gunAngle to match laser so guns visually follow the beam
                boss.gunAngle = boss.laserAngle * (180.f / PI);

                // Laser collision
                AEVec2 guns[2] = { GetGunPosition(boss, true),
                                   GetGunPosition(boss, false) };
                float laserLength = 1200.f;
                float laserWidth = 8.f;

                for (int i = 0; i < 2; i++) {
                    float dx = player.pos_x - guns[i].x;
                    float dy = player.pos_y - guns[i].y;
                    float laserDirX = cosf(boss.laserAngle);
                    float laserDirY = sinf(boss.laserAngle);
                    float along = dx * laserDirX + dy * laserDirY;
                    float perp = dx * laserDirY - dy * laserDirX;

                    if (along > 0.f && along < laserLength && fabsf(perp) < laserWidth + player.scale) {
                        player_init.current_hp -= GameConfig::Boss::LASER_DAMAGE_PER_SEC * mult * deltaTime;
                    }
                }
            }

            if (boss.attackTimer >= boss.lungeDuration) {
                boss.laserActive = false;

                // Sync gunAngle cleanly before handing back to the lerp
                if (boss.currentAttack == Boss3Attack::LASER) {
                    boss.gunAngle = boss.laserAngle * (180.f / PI);

                    // Normalize to -180 to 180 range
                    while (boss.gunAngle > 180.f) boss.gunAngle -= 360.f;
                    while (boss.gunAngle < -180.f) boss.gunAngle += 360.f;
                }

                boss.state = BossState::COOLDOWN;
                boss.stateTimer = 0.f;
                boss.currentAttack = Boss3Attack::NONE;
                boss.shootTimer = 0.f;
            }
            break;

        case BossState::COOLDOWN:
            boss.laserActive = false;
            if (boss.stateTimer >= boss.cooldownDuration) {
                boss.state = BossState::IDLE;
                boss.stateTimer = 0.f;
            }
            break;
        }

        // push boss out of trees/walls
        World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);

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

    //collision check with world border
    float margin = boss.scale * 0.5f + World::TILE_SIZE;
    float minX = -World::HALF_WIDTH + margin;
    float maxX = World::HALF_WIDTH - margin;
    float minY = -World::HALF_HEIGHT + margin;
    float maxY = World::HALF_HEIGHT - margin;

    if (boss.pos.x < minX) { boss.pos.x = minX; boss.velocity.x = 0.f; }
    if (boss.pos.x > maxX) { boss.pos.x = maxX; boss.velocity.x = 0.f; }
    if (boss.pos.y < minY) { boss.pos.y = minY; boss.velocity.y = 0.f; }
    if (boss.pos.y > maxY) { boss.pos.y = maxY; boss.velocity.y = 0.f; }

    // push boss out of trees/walls
    World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);

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

    float rotRad = boss.rotation * (PI / 180.f);

    // Draw boss texture
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBossTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // Flash white during telegraph
    bool telegraphing = (boss.state == BossState::TELEGRAPHING);
    if (telegraphing)
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    else
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);

    AEGfxSetColorToAdd(
        telegraphing ? 0.5f : 0.f,
        telegraphing ? 0.5f : 0.f,
        telegraphing ? 0.5f : 0.f,
        0.f
    );

    Gfx::printMesh(pBossMesh, boss.pos, { boss.scale, boss.scale }, rotRad, { 0.f, 0.f }, true);

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

    // BOSS4 extras
    if (boss.bosstype == BOSS4) {
        float gunRotRad = boss.gunAngle * (PI / 180.f);
        AEVec2 guns[2] = { GetGunPosition(boss, true), GetGunPosition(boss, false) };
        DrawBossLaser(boss, MeshRect);
        AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.f);
        for (int i = 0; i < 2; i++) {
            Gfx::printMesh(MeshRect, guns[i], { boss.scale * 0.4f, boss.scale * 0.15f }, gunRotRad);
        }
    }
}

void DrawBossHP(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle, shape& player) {

    // HP bar above boss
    float hpPct = (float)boss.hp / (float)boss.maxhp;
    float barWidth = 1000.f;
    //float barY = boss.pos.y + boss.scale + 20.f;

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
    float radius = boss.scale + GameConfig::Boss::MINION_SPAWN_RADIUS;

    for (int i = 0; i < boss.minionCount; i++) {
        float angle = angleStep * i;
        for (auto& newEnemy : minionPool) {
            if (!newEnemy.alive) {
                newEnemy.pos = { boss.pos.x + cosf(angle) * radius,
                                      boss.pos.y + sinf(angle) * radius };
                newEnemy.velocity = { 0, 0 };
                newEnemy.alive = true;
                newEnemy.rotation = angle * (180.f / PI);
                newEnemy.scale = GameConfig::Enemy::SIZE_BIG;
                newEnemy.enemtype = ATTACK;
                newEnemy.detect = true;
                newEnemy.hp = GameConfig::Boss::MINION_BASE_HP;
                newEnemy.maxhp = GameConfig::Boss::MINION_BASE_HP;
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

            //push out of trees and walls
            World::PushOutOfWalls(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale * 0.5f);
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

        //push out of trees and walls
        World::PushOutOfWalls(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale * 0.5f);

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

void FreeBoss() {
    if (pBossTex) { AEGfxTextureUnload(pBossTex);   pBossTex = nullptr; }
    if (pMinionTex) { AEGfxTextureUnload(pMinionTex); pMinionTex = nullptr; }
    if (pBossMesh) { AEGfxMeshFree(pBossMesh);       pBossMesh = nullptr; }
}