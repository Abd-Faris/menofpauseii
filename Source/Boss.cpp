// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Boss.cpp
// Authors: [Men of Pause II]
// Brief:   This file implements functions for spawning bosses, loading their
//          textures, and all logic for each boss's movement, attacks, collision,
//          drawing, and minion management.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// ~ Brief: Object pool for minions spawned by BOSS2 — separate from the main enemy pool
//          to prevent minions from interfering with regular enemy spawning logic.
std::array<Enemies, MAX_MINIONS_COUNT> minionPool;
extern int currentWave;

// ~ Brief: Shared texture and mesh assets for all boss types.
//          pBossTex is applied to all boss bodies; pBossMesh is a UV-mapped unit quad.
AEGfxTexture* pBossTex = nullptr;
AEGfxTexture* pMinionTex = nullptr;
AEGfxVertexList* pBossMesh = nullptr;

// ~ Brief: The single active boss instance. Only one boss is alive at a time.
Boss currentboss;

// =============================================================================
// LOAD / FREE
// =============================================================================

// ~ Brief: Load boss and minion textures, and build the UV-mapped unit quad mesh
//          shared by all boss draw calls.
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

// ~ Brief: Unload all boss textures and free the boss mesh to prevent memory leaks.
//          All pointers are set to nullptr after freeing.
void FreeBoss() {
    if (pBossTex) { AEGfxTextureUnload(pBossTex);   pBossTex = nullptr; }
    if (pMinionTex) { AEGfxTextureUnload(pMinionTex);  pMinionTex = nullptr; }
    if (pBossMesh) { AEGfxMeshFree(pBossMesh);        pBossMesh = nullptr; }
}

// =============================================================================
// SPAWN
// =============================================================================

// ~ Brief: Initialize the boss instance for the given type, scaling HP by the current
//          wave number. All stats are pulled from GameConfig::Boss constants so tuning
//          is centralized. The boss spawns offset from the player to avoid overlap.
void SpawnBoss(BossType type, shape& player) {
    currentboss = {};  // zero all fields before setting type-specific values

    // Wave scaling multiplier — boss HP increases every 5 waves
    f32 mult = (1 + (currentWave / 5 * 0.5f));

    currentboss.pos = { player.pos_x + 400.f, player.pos_y + 400.f };
    currentboss.velocity = { 0, 0 };
    currentboss.alive = true;
    currentboss.bosstype = type;
    currentboss.state = BossState::IDLE;
    currentboss.currentAttack = Boss3Attack::NONE;

    switch (type) {
    case BOSS1: // ~ Brief: Lunging boss — chases and dashes at the player, fires a bullet ring on landing.
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

    case BOSS2: // ~ Brief: Stationary boss — periodically telegraphs and spawns a ring of tracking minions.
        currentboss.scale = GameConfig::Enemy::SIZE_BIG * GameConfig::Boss::B2_SCALE;
        currentboss.hp = static_cast<int>(GameConfig::Boss::B2_BASE_HP * mult);
        currentboss.xp = GameConfig::Boss::B2_XP;
        currentboss.chaseSpeed = 0.f;  // stationary
        currentboss.idleDuration = GameConfig::Boss::B2_IDLE_DUR;
        currentboss.telegraphDuration = GameConfig::Boss::B2_TELEGRAPH_DUR;
        currentboss.lungeDuration = GameConfig::Boss::B2_LUNGE_DUR;
        currentboss.cooldownDuration = GameConfig::Boss::B2_COOLDOWN_DUR;
        currentboss.minionCount = GameConfig::Boss::B2_MINION_COUNT;
        break;

    case BOSS3: // ~ Brief: Chasing boss — randomly alternates between a spiral bullet attack and an aimed burst.
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

    case BOSS4: // ~ Brief: Gun boss — chases the player and alternates between dual-gun bursts and a sweeping laser.
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

    currentboss.maxhp = currentboss.hp; // set max HP after all stats are assigned
}

// =============================================================================
// ATTACK FUNCTIONS
// =============================================================================

// ~ Brief: Fire a ring of evenly spaced bullets outward from the boss center.
//          Used by BOSS1 immediately after a lunge lands. Bullet count is set per boss instance.
//          Damage scales with the current wave multiplier.
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

// ~ Brief: Fire one bullet per spiral arm at evenly spaced angles, rotating the
//          spiral angle each call to create a spinning pattern. Fire rate is throttled
//          by SPIRAL_FIRE_RATE to control bullet density. Used by BOSS3.
void Boss3Spiral(Boss& boss, float deltaTime) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));

    boss.shootTimer += deltaTime;
    if (boss.shootTimer < GameConfig::Boss::SPIRAL_FIRE_RATE) return;
    boss.shootTimer = 0.f;

    float angleStep = (2.f * PI) / boss.bulletCount; // evenly space arms around 360 degrees

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

    // Advance the spiral angle so the next volley rotates further
    boss.spiralAngle += GameConfig::Boss::SPIRAL_SPEED * deltaTime;
}

// ~ Brief: Fire a tight cluster of bullets directly toward the player's current position.
//          Bullets are spread slightly apart by AIMED_SPREAD radians. Used by BOSS3.
void Boss3AimedShot(Boss& boss, shape& player) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));

    AEVec2 toPlayer = { player.pos_x - boss.pos.x, player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    if (dist < 1.f) return;

    float baseAngle = atan2f(toPlayer.y, toPlayer.x);

    for (int i = 0; i < GameConfig::Boss::AIMED_COUNT; i++) {
        // Offset each bullet symmetrically around the base angle
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

// ~ Brief: Calculate the world position of BOSS4's left or right gun barrel tip,
//          based on the boss's independent gun angle. Used for both shooting origins
//          and laser draw positions.
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

// ~ Brief: Fire one bullet from each of BOSS4's two gun barrels toward the player.
//          The gun angle snaps partially toward the player each shot interval, giving
//          gradual tracking without instant aim correction. Respects gunFireRate cooldown.
void Boss4ShootGuns(Boss& boss, shape& player, float deltaTime) {
    f32 mult = (1 + (currentWave / 5 * 0.5f));

    boss.gunFireTimer += deltaTime;
    if (boss.gunFireTimer < boss.gunFireRate) return;
    boss.gunFireTimer = 0.f;

    AEVec2 toPlayer = { player.pos_x - boss.pos.x, player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    if (dist < 1.f) return;

    // Snap gun angle partially toward player each shot — gives gradual tracking
    float targetAngle = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
    float angleDiff = targetAngle - boss.gunAngle;
    while (angleDiff > 180.f) angleDiff -= 360.f;
    while (angleDiff < -180.f) angleDiff += 360.f;
    boss.gunAngle += angleDiff * 0.5f;

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

// =============================================================================
// DRAW FUNCTIONS
// =============================================================================

// ~ Brief: Draw BOSS4's laser beam from each gun barrel in the current laser direction.
//          During telegraph the beam is narrow and faint red — a visual warning to the player.
//          During the active attack the beam is wide and orange.
//          Returns early if the current attack is not LASER or the laser is inactive.
void DrawBossLaser(Boss& boss, AEGfxVertexList* MeshRect) {
    if (boss.state == BossState::TELEGRAPHING && boss.currentAttack != Boss3Attack::LASER) return;
    if (!boss.laserActive && boss.state != BossState::TELEGRAPHING) return;

    AEVec2 guns[2] = { GetGunPosition(boss, true), GetGunPosition(boss, false) };
    float laserLength = GameConfig::Boss::LASER_LENGTH;
    float laserWidth = boss.laserActive
        ? GameConfig::Boss::LASER_WIDTH_ACTIVE
        : GameConfig::Boss::LASER_WIDTH_TELEGRAPH;

    for (int i = 0; i < 2; i++) {
        // Center the rectangle along the beam direction starting from the gun tip
        float midX = guns[i].x + cosf(boss.laserAngle) * laserLength * 0.5f;
        float midY = guns[i].y + sinf(boss.laserAngle) * laserLength * 0.5f;

        if (boss.laserActive)
            AEGfxSetColorToMultiply(1.0f, 0.2f, 0.0f, 1.f);  // orange — active laser
        else
            AEGfxSetColorToMultiply(0.60f, 0.0f, 0.0f, 0.2f); // faint red — telegraph warning

        Gfx::printMesh(MeshRect, { midX, midY }, { laserLength, laserWidth }, boss.laserAngle);
    }
}

// ~ Brief: Draw the boss body texture with a telegraph flash effect.
//          During TELEGRAPHING state, an additive white color boost is applied to signal
//          an incoming attack. BOSS4 additionally draws its gun barrels and laser.
void DrawBoss(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle) {
    if (!boss.alive) return;
    (void)MeshCircle; // unused — reserved for future circular boss elements

    float rotRad = boss.rotation * (PI / 180.f);

    // Draw boss body using texture
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBossTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);

    // Add a white flash during telegraph to signal an incoming attack
    bool telegraphing = (boss.state == BossState::TELEGRAPHING);
    AEGfxSetColorToAdd(
        telegraphing ? 0.5f : 0.f,
        telegraphing ? 0.5f : 0.f,
        telegraphing ? 0.5f : 0.f,
        0.f
    );

    Gfx::printMesh(pBossMesh, boss.pos, { boss.scale, boss.scale }, rotRad, { 0.f, 0.f }, true);

    // Reset additive color immediately after boss mesh to avoid bleeding onto other draw calls
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // BOSS4 extras — draw gun barrels and laser beam
    if (boss.bosstype == BOSS4) {
        float gunRotRad = boss.gunAngle * (PI / 180.f);
        AEVec2 guns[2] = { GetGunPosition(boss, true), GetGunPosition(boss, false) };

        DrawBossLaser(boss, MeshRect);

        // Draw gun barrels as dark grey rectangles aligned to gun angle
        AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.f);
        for (int i = 0; i < 2; i++) {
            Gfx::printMesh(MeshRect, guns[i], { boss.scale * 0.4f, boss.scale * 0.15f }, gunRotRad);
        }
    }
}

// ~ Brief: Draw a boss HP bar anchored to the top of the player's screen view.
//          The bar scales from full width (full HP) to zero width (dead).
//          A dark red background is drawn first, then the red fill on top.
void DrawBossHP(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle, shape& player) {
    (void)MeshCircle; // unused

    float hpPct = (float)boss.hp / (float)boss.maxhp;
    float barWidth = 1000.f;

    // Background bar — full width, dark red
    AEGfxSetColorToMultiply(0.3f, 0.f, 0.f, 1.f);
    Gfx::printMesh(MeshRect, { player.pos_x, player.pos_y + 400.f }, { barWidth, 50.f }, 0.f);

    // Foreground bar — scales with current HP, bright red
    AEGfxSetColorToMultiply(1.f, 0.f, 0.f, 1.f);
    float filledWidth = barWidth * hpPct;
    Gfx::printMesh(MeshRect,
        { player.pos_x - (barWidth - filledWidth) / 2.f, player.pos_y + 400 },
        { filledWidth, 50.f }, 0.f);
}

// =============================================================================
// MINION FUNCTIONS
// =============================================================================

// ~ Brief: Spawn BOSS2's minions in an evenly spaced ring around the boss position.
//          Each minion is placed just outside the boss hitbox and immediately set to
//          chase the player (detect = true). Minion count is set per boss instance.
void BossSpawnMinion(Boss& boss) {
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
                newEnemy.rotation = angle * (180.f / PI); // face outward on spawn
                newEnemy.scale = GameConfig::Enemy::SIZE_BIG;
                newEnemy.enemtype = ATTACK;
                newEnemy.detect = true;  // skip detection range — immediately chase
                newEnemy.hp = GameConfig::Boss::MINION_BASE_HP;
                newEnemy.maxhp = GameConfig::Boss::MINION_BASE_HP;
                break;
            }
        }
    }
}

// ~ Brief: Update physics for all active minions each frame.
//          ATTACK minions chase the player once detected or damaged, with smooth rotation
//          toward the player and separation forces to prevent stacking.
//          SHOOTER minions maintain distance, aim at the player, and fire on a cooldown.
//          All minions are pushed out of walls each frame and reset when HP reaches zero.
void updateMinionPhysics(shape& player, float deltaTime) {
    for (auto& currentEnemy : minionPool) {
        if (!currentEnemy.alive) continue;

        // -- ATTACK minion: detect and chase the player --
        if (currentEnemy.enemtype == ATTACK) {
            AEVec2 PlayerPos = { player.pos_x, player.pos_y };
            AEVec2 EnemyPos = { currentEnemy.pos };
            AEVec2 dir = {};
            AEVec2Sub(&dir, &PlayerPos, &EnemyPos);

            f32 hyp = sqrt(dir.x * dir.x + dir.y * dir.y);

            // Activate detection when player is within 600 units
            if (hyp <= 600)
                currentEnemy.detect = true;

            if (currentEnemy.detect || currentEnemy.hp < currentEnemy.maxhp) {
                // Smoothly rotate to face the player
                if ((dir.x * dir.x) + (dir.y * dir.y) > GameConfig::MOUSE_JITTER_THRESHOLD) {
                    float targetAngle = atan2f(dir.y, dir.x) * (180.f / PI);
                    float angleDifference = targetAngle - currentEnemy.rotation;
                    while (angleDifference > 180.f) angleDifference -= 360.f;
                    while (angleDifference < -180.f) angleDifference += 360.f;
                    currentEnemy.rotation += angleDifference * 0.1f;
                }

                // Normalize and accelerate toward player
                dir.x /= hyp;
                dir.y /= hyp;
                currentEnemy.velocity.x += dir.x * 2500 * deltaTime;
                currentEnemy.velocity.y += dir.y * 2500 * deltaTime;
                currentEnemy.velocity.x *= GameConfig::Enemy::FRICTION;
                currentEnemy.velocity.y *= GameConfig::Enemy::FRICTION;
                currentEnemy.pos.x += currentEnemy.velocity.x * deltaTime;
                currentEnemy.pos.y += currentEnemy.velocity.y * deltaTime;
            }

            World::PushOutOfWalls(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale * 0.5f);
        }

        // -- SHOOTER minion: keep distance, aim, and fire on cooldown --
        if (currentEnemy.enemtype == SHOOTER) {
            AEVec2 PlayerPos = { player.pos_x, player.pos_y };
            AEVec2 EnemyPos = { currentEnemy.pos.x, currentEnemy.pos.y };
            AEVec2 dir = {};
            AEVec2Sub(&dir, &PlayerPos, &EnemyPos);

            f32 hyp = sqrt(dir.x * dir.x + dir.y * dir.y);

            // Smoothly rotate to face the player
            if ((dir.x * dir.x) + (dir.y * dir.y) > GameConfig::MOUSE_JITTER_THRESHOLD) {
                float targetAngle = atan2f(dir.y, dir.x) * (180.f / PI);
                float angleDifference = targetAngle - currentEnemy.rotation;
                while (angleDifference > 180.f) angleDifference -= 360.f;
                while (angleDifference < -180.f) angleDifference += 360.f;
                currentEnemy.rotation += angleDifference * 0.1f;
            }

            // Normalize direction for movement and shooting
            if (hyp > 0) {
                dir.x /= hyp;
                dir.y /= hyp;
            }

            // Move closer only if further than the preferred engagement distance
            if (hyp > 400.0f) {
                currentEnemy.velocity.x += dir.x * 300 * deltaTime;
                currentEnemy.velocity.y += dir.y * 300 * deltaTime;
            }

            // Fire toward player when cooldown expires and player is in range
            currentEnemy.cooldown -= deltaTime;
            if (currentEnemy.cooldown <= 0.0f && hyp < 800.0f) {
                currentEnemy.cooldown = 1.5f; // reset fire cooldown

                for (auto& eBullet : enemyBulletList) {
                    if (!eBullet.isActive) {
                        eBullet.isActive = true;
                        eBullet.posX = currentEnemy.pos.x;
                        eBullet.posY = currentEnemy.pos.y;
                        eBullet.directionX = dir.x;
                        eBullet.directionY = dir.y;
                        eBullet.speed = 400.0f;
                        eBullet.size = 15.0f;
                        eBullet.damagemul = 1.0f;
                        break;
                    }
                }
            }
        }

        // -- Separation — push minions apart to prevent stacking --
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

        World::PushOutOfWalls(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale * 0.5f);

        // Remove minion from the pool when HP reaches zero
        if (currentEnemy.hp <= 0)
            ResetEnemy(&currentEnemy);
    }
}

// =============================================================================
// PHYSICS
// =============================================================================

// ~ Brief: Update the active boss's state machine and movement each frame.
//          Each boss type has its own behavior block that returns early after processing.
//          All bosses share the same BossState enum (IDLE, TELEGRAPHING, LUNGING, COOLDOWN)
//          but interpret each state differently based on their attack patterns.
void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime) {
    if (!boss.alive) return;

    // Wave scaling multiplier — shared across all boss damage calculations
    f32 mult = (1 + (currentWave / 5 * 0.5f));
    (void)mult; // suppress warning if not used in this scope

    AEVec2 toPlayer = { player.pos_x - boss.pos.x,
                        player.pos_y - boss.pos.y };
    float dist = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    boss.stateTimer += deltaTime;

    // -----------------------------------------------------------------------
    // BOSS1 — lunging boss
    // -----------------------------------------------------------------------
    if (boss.bosstype == BOSS1) {
        switch (boss.state) {

        case BossState::IDLE:
            // Chase player and smoothly rotate to face them
            if (dist > 1.f) {
                boss.velocity.x += (toPlayer.x / dist) * boss.chaseSpeed * deltaTime;
                boss.velocity.y += (toPlayer.y / dist) * boss.chaseSpeed * deltaTime;

                float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
                float angleDiff = targetRotation - boss.rotation;
                while (angleDiff > 180.f) angleDiff -= 360.f;
                while (angleDiff < -180.f) angleDiff += 360.f;
                boss.rotation += angleDiff * 8.0f * deltaTime;
            }
            boss.velocity.x *= GameConfig::Enemy::FRICTION;
            boss.velocity.y *= GameConfig::Enemy::FRICTION;
            boss.pos.x += boss.velocity.x * deltaTime;
            boss.pos.y += boss.velocity.y * deltaTime;

            // Lock lunge direction toward player and begin telegraph
            if (boss.stateTimer >= boss.idleDuration) {
                boss.state = BossState::TELEGRAPHING;
                boss.stateTimer = 0.f;
                if (dist > 1.f)
                    boss.lungeDirection = { toPlayer.x / dist, toPlayer.y / dist };
            }
            break;

        case BossState::TELEGRAPHING:
            // Freeze in place — player's window to dodge
            boss.velocity = { 0, 0 };
            if (boss.stateTimer >= boss.telegraphDuration) {
                boss.lungehit = false; // reset contact damage flag for this lunge
                boss.state = BossState::LUNGING;
                boss.stateTimer = 0.f;
            }
            break;

        case BossState::LUNGING: {
            // Decelerate from full lunge speed to zero over lungeDuration
            float t = boss.stateTimer / boss.lungeDuration;
            float speed = boss.lungeSpeed * (1.0f - t);
            boss.velocity.x = boss.lungeDirection.x * speed;
            boss.velocity.y = boss.lungeDirection.y * speed;
            boss.pos.x += boss.velocity.x * deltaTime;
            boss.pos.y += boss.velocity.y * deltaTime;

            // Lock rotation to lunge direction for the duration of the dash
            boss.rotation = atan2f(boss.lungeDirection.y, boss.lungeDirection.x) * (180.f / PI);

            if (boss.stateTimer >= boss.lungeDuration) {
                boss.state = BossState::COOLDOWN;
                boss.stateTimer = 0.f;
                boss.velocity = { 0, 0 };
            }
            break;
        }

        case BossState::COOLDOWN:
            // Fire bullet ring once shortly after the lunge lands
            boss.shootTimer += deltaTime;
            if (!boss.hasShot && boss.shootTimer >= 0.1f) {
                BossShootRing(boss);
                boss.shootTimer = boss.cooldownDuration; // block further shots this cooldown
                boss.hasShot = true;
            }

            if (boss.stateTimer >= boss.cooldownDuration) {
                boss.state = BossState::IDLE;
                boss.stateTimer = 0.f;
                boss.shootTimer = 0.f;
                boss.hasShot = false;
            }
            break;
        }

        World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);
    }

    // -----------------------------------------------------------------------
    // BOSS2 — stationary minion spawner
    // -----------------------------------------------------------------------
    if (boss.bosstype == BOSS2) {
        // Rotate to face player even though stationary
        if (dist > 1.f) {
            float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float angleDiff = targetRotation - boss.rotation;
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;
            boss.rotation += angleDiff * 5.0f * deltaTime;
        }

        switch (boss.state) {
        case BossState::IDLE:
            if (boss.stateTimer >= boss.idleDuration) {
                boss.state = BossState::TELEGRAPHING;
                boss.stateTimer = 0.f;
            }
            break;

        case BossState::TELEGRAPHING:
            // Flash — no movement, gives player time to position
            if (boss.stateTimer >= boss.telegraphDuration) {
                boss.state = BossState::LUNGING;
                boss.stateTimer = 0.f;
            }
            break;

        case BossState::LUNGING:
            // Spawn minion ring instantly then transition to cooldown
            BossSpawnMinion(boss);
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
    // BOSS3 — chasing boss with random attack selection
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

        switch (boss.state) {
        case BossState::IDLE:
            if (boss.stateTimer >= boss.idleDuration) {
                // Pick attack before telegraphing so draw code can preview which attack is coming
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
                boss.state = BossState::LUNGING;
                boss.stateTimer = 0.f;
                boss.attackTimer = 0.f;
            }
            break;

        case BossState::LUNGING:
            boss.attackTimer += deltaTime;

            if (boss.currentAttack == Boss3Attack::SPIRAL)
                Boss3Spiral(boss, deltaTime);
            else if (boss.currentAttack == Boss3Attack::AIMED) {
                // Fire aimed burst every 0.4 seconds during the attack window
                if (fmodf(boss.attackTimer, 0.4f) < deltaTime)
                    Boss3AimedShot(boss, player);
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
    // BOSS4 — dual-gun and laser boss
    // -----------------------------------------------------------------------
    if (boss.bosstype == BOSS4) {
        // Body slowly faces player — separate from gun angle tracking
        if (dist > 1.f) {
            float targetRotation = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float angleDiff = targetRotation - boss.rotation;
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;
            boss.rotation += angleDiff * 3.f * deltaTime;
        }

        // Gun angle tracks player independently of body rotation
        // Speed varies by state — slower during laser (driven externally), slower during cooldown recovery
        if (dist > 1.f) {
            float targetGunAngle = atan2f(toPlayer.y, toPlayer.x) * (180.f / PI);
            float gunAngleDiff = targetGunAngle - boss.gunAngle;
            while (gunAngleDiff > 180.f) gunAngleDiff -= 360.f;
            while (gunAngleDiff < -180.f) gunAngleDiff += 360.f;

            float trackSpeed = 60.f;
            if (boss.state == BossState::LUNGING && boss.currentAttack == Boss3Attack::GUNS)
                trackSpeed = 40.f;  // gradual tracking while shooting
            else if (boss.state == BossState::LUNGING && boss.currentAttack == Boss3Attack::LASER)
                trackSpeed = 0.f;   // laser sweeps gunAngle directly — skip lerp
            else if (boss.state == BossState::COOLDOWN)
                trackSpeed = 30.f;  // slow return after laser ends

            // Dead zone prevents micro-jitter when nearly aligned
            if (fabsf(gunAngleDiff) > 0.5f && trackSpeed > 0.f)
                boss.gunAngle += gunAngleDiff * trackSpeed * deltaTime;
        }

        // Chase player
        boss.velocity.x += (dist > 1.f ? (toPlayer.x / dist) : 0.f) * boss.chaseSpeed * deltaTime;
        boss.velocity.y += (dist > 1.f ? (toPlayer.y / dist) : 0.f) * boss.chaseSpeed * deltaTime;
        boss.velocity.x *= GameConfig::Enemy::FRICTION;
        boss.velocity.y *= GameConfig::Enemy::FRICTION;
        boss.pos.x += boss.velocity.x * deltaTime;
        boss.pos.y += boss.velocity.y * deltaTime;

        switch (boss.state) {
        case BossState::IDLE:
            boss.laserActive = false;
            if (boss.stateTimer >= boss.idleDuration) {
                // Randomly pick guns or laser attack
                boss.currentAttack = (AERandFloat() > 0.5f)
                    ? Boss3Attack::GUNS
                    : Boss3Attack::LASER;
                boss.state = BossState::TELEGRAPHING;
                boss.stateTimer = 0.f;

                // Pre-aim laser toward player at the moment of selection
                if (boss.currentAttack == Boss3Attack::LASER && dist > 1.f) {
                    boss.laserAngle = atan2f(toPlayer.y, toPlayer.x);
                    boss.laserTargetAngle = boss.laserAngle;
                }
            }
            break;

        case BossState::TELEGRAPHING:
            if (boss.currentAttack == Boss3Attack::LASER) {
                // Slowly sweep laser toward player during telegraph — gives the player time to reposition
                if (dist > 1.f) {
                    float targetAngle = atan2f(toPlayer.y, toPlayer.x);
                    float angleDiff = targetAngle - boss.laserAngle;
                    while (angleDiff > PI) angleDiff -= 2.f * PI;
                    while (angleDiff < -PI) angleDiff += 2.f * PI;
                    boss.laserAngle += angleDiff * 0.3f * deltaTime; // slow warning sweep

                    while (boss.laserAngle > PI) boss.laserAngle -= 2.f * PI;
                    while (boss.laserAngle < -PI) boss.laserAngle += 2.f * PI;
                }
                // Sync gun angle to laser so barrels visually point at the beam
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
                // Sweep laser toward player at laserSweepSpeed radians per second
                if (dist > 1.f) {
                    float targetAngle = atan2f(toPlayer.y, toPlayer.x);
                    float angleDiff = targetAngle - boss.laserAngle;
                    while (angleDiff > PI) angleDiff -= 2.f * PI;
                    while (angleDiff < -PI) angleDiff += 2.f * PI;
                    boss.laserAngle += angleDiff * boss.laserSweepSpeed * deltaTime;

                    while (boss.laserAngle > PI) boss.laserAngle -= 2.f * PI;
                    while (boss.laserAngle < -PI) boss.laserAngle += 2.f * PI;
                }

                // Keep gun barrels visually aligned with the beam during sweep
                boss.gunAngle = boss.laserAngle * (180.f / PI);

                // Laser collision — project player position onto beam and check perpendicular distance
                AEVec2 guns[2] = { GetGunPosition(boss, true), GetGunPosition(boss, false) };
                float laserLength = 1200.f;
                float laserWidth = 8.f;

                for (int i = 0; i < 2; i++) {
                    float dx = player.pos_x - guns[i].x;
                    float dy = player.pos_y - guns[i].y;
                    float laserDirX = cosf(boss.laserAngle);
                    float laserDirY = sinf(boss.laserAngle);
                    float along = dx * laserDirX + dy * laserDirY; // distance along beam
                    float perp = dx * laserDirY - dy * laserDirX; // distance from beam centerline

                    if (along > 0.f && along < laserLength && fabsf(perp) < laserWidth + player.scale)
                        player_init.current_hp -= GameConfig::Boss::LASER_DAMAGE_PER_SEC * mult * deltaTime;
                }
            }

            if (boss.attackTimer >= boss.lungeDuration) {
                boss.laserActive = false;

                // Sync gun angle from laser before returning control to the lerp — prevents snap
                if (boss.currentAttack == Boss3Attack::LASER) {
                    boss.gunAngle = boss.laserAngle * (180.f / PI);
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

        World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);

        if (boss.hp <= 0) {
            player_init.current_xp += boss.xp;
            TriggerXpPopup((float)boss.xp);
            TriggerExplosion(boss.pos.x, boss.pos.y, boss.scale);
            boss.alive = false;
        }
        return;
    }

    // Fallback death check for any boss type not handled above
    if (boss.hp <= 0) {
        player_init.current_xp += boss.xp;
        TriggerXpPopup((float)boss.xp);
        TriggerExplosion(boss.pos.x, boss.pos.y, boss.scale);
        boss.alive = false;
    }
}

// =============================================================================
// COLLISION
// =============================================================================

// ~ Brief: Handle all collision for the active boss each frame:
//          - Clamp boss position to world boundaries and push out of walls.
//          - Player bullets and orbit damage the boss.
//          - Enemy bullets fired by the boss damage the player.
//          - Direct player contact damages the player (BOSS1 only once per lunge).
void BossCollision(Boss& boss, shape& player, bool orbitActive, float orbitPosX, float orbitPosY) {
    if (!boss.alive) return;

    // -- World boundary clamp --
    float margin = boss.scale * 0.5f + World::TILE_SIZE;
    float minX = -World::HALF_WIDTH + margin;
    float maxX = World::HALF_WIDTH - margin;
    float minY = -World::HALF_HEIGHT + margin;
    float maxY = World::HALF_HEIGHT - margin;

    if (boss.pos.x < minX) { boss.pos.x = minX; boss.velocity.x = 0.f; }
    if (boss.pos.x > maxX) { boss.pos.x = maxX; boss.velocity.x = 0.f; }
    if (boss.pos.y < minY) { boss.pos.y = minY; boss.velocity.y = 0.f; }
    if (boss.pos.y > maxY) { boss.pos.y = maxY; boss.velocity.y = 0.f; }

    World::PushOutOfWalls(boss.pos.x, boss.pos.y, boss.scale * 0.5f);

    // -- Player bullets hit boss --
    for (auto& boolet : bulletList) {
        if (!boolet.isActive) continue;

        float dx = boolet.posX - boss.pos.x;
        float dy = boolet.posY - boss.pos.y;
        float distSq = dx * dx + dy * dy;
        float colRadius = (boss.scale * GameConfig::Enemy::HITBOX_RATIO) + boolet.size;

        if (distSq < colRadius * colRadius) {
            TriggerBulletImpact(boolet.posX, boolet.posY, boolet.directionX, boolet.directionY);
            float dmg = calculate_max_stats(1);
            boss.hp -= (int)(dmg * boolet.damagemul);
            boolet.isActive = false;
        }
    }

    // -- Orbit ability hits boss --
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

    // -- Boss's enemy bullets hit player --
    for (auto& enBullet : enemyBulletList) {
        if (!enBullet.isActive) continue;

        float differenceX = enBullet.posX - player.pos_x;
        float differenceY = enBullet.posY - player.pos_y;
        float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);
        float collisionRadius = player.scale + enBullet.size;

        if (distanceSquared < (collisionRadius * collisionRadius)) {
            TriggerBulletImpact(enBullet.posX, enBullet.posY, enBullet.directionX, enBullet.directionY);
            player_init.current_hp -= 10;
            playerFlashTimer = 0.15f;
            enBullet.isActive = false;
        }
    }

    // -- Player body contact with boss --
    // BOSS1: one damage instance per lunge (lungehit flag prevents continuous damage).
    // All other bosses: damage on every contact frame.
    float dx = player.pos_x - boss.pos.x;
    float dy = player.pos_y - boss.pos.y;
    float distSq = dx * dx + dy * dy;
    float colRadius = (boss.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;

    if (distSq < colRadius * colRadius) {
        if (boss.bosstype == BOSS1) {
            if (!boss.lungehit) {
                playerFlashTimer = 0.15f;
                player_init.current_hp -= boss.maxhp / 8;
                boss.lungehit = true;
            }
        }
        else {
            playerFlashTimer = 0.15f;
            player_init.current_hp -= boss.maxhp / 8;
        }
    }
}