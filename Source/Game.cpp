// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Game.cpp
// Authors: [Men of Pause II]
// Brief:   Core game loop implementation. Handles collision, loading, updating,
//          drawing, and freeing all gameplay systems including the player, enemies,
//          bosses, bullets, waves, and HUD elements.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================

// ~ Brief: Extern flags for special player abilities — set by card upgrades.
extern bool dualback, orbitActive;
extern float orbitAngle, orbitPosX, orbitPosY;

// ~ Brief: Tracks whether the left mouse button was released this frame.
bool mousereleased = false;

// ~ Brief: Current wave number — increments each time a wave is cleared.
int currentWave = 1;

// ~ Brief: Countdown timer that flashes the player red when hit.
//          Set to a positive value on damage, decrements each frame.
float playerFlashTimer = 0.0f;

// ~ Brief: Accumulates time since the current wave became active.
//          Used to prevent instant wave-clear detection on wave start.
float waveActiveTimer = 0.0f;

// ~ Brief: Tracks the upgrade flags from the previous frame to detect
//          newly applied card upgrades during cheat mode.
u32 prevUpgradeFlag = UPGRADE_NONE;

namespace {
    // ~ Brief: Shared meshes for drawing all colored geometry in the game world.
    AEGfxVertexList* MeshRect = nullptr;
    AEGfxVertexList* MeshCircle = nullptr;
    AEGfxVertexList* MeshTriangle = nullptr;

    // ~ Brief: The player tank instance — position, angle, scale, and barrel count.
    shape player = { GameConfig::Tank::SCALE, 0.0f, 0.0f, 0.0f, 1 };
}

// =============================================================================
// COLLISION LOGIC
// =============================================================================

// ~ Brief: Resolve all player-enemy, bullet-enemy, and bullet-player collisions
//          each frame. Checks player body and barrel against each enemy, orbit
//          ability against enemies, player bullets against enemies and minions,
//          and enemy bullets against the player body and barrel.
//          Triggers explosions, flash timers, and HP changes on contact.
void circlerectcollision() {
    for (auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive) continue;

        // -- Player body vs enemy --
        float bodyDiffX = player.pos_x - currentEnemy.pos.x;
        float bodyDiffY = player.pos_y - currentEnemy.pos.y;
        float bodyDistSq = (bodyDiffX * bodyDiffX) + (bodyDiffY * bodyDiffY);
        float bodyColRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;

        if (bodyDistSq < (bodyColRadius * bodyColRadius)) {
            player_init.current_hp -= currentEnemy.hp / 3.0f;
            currentEnemy.hp = 0;
            TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
            playerFlashTimer = 0.15f;
        }
        else {
            // -- Player barrel vs enemy — check 3 points along the barrel length --
            float barrelLen = GameConfig::Tank::BARREL_LENGTH * (player.scale / GameConfig::Tank::SCALE);
            float cosA = cosf(player.currentAngle);
            float sinA = sinf(player.currentAngle);
            float checkpoints[3] = { 0.5f, 0.75f, 1.0f };

            for (int p = 0; p < 3; p++) {
                float checkX = player.pos_x + (0.0f * cosA - (barrelLen * checkpoints[p]) * sinA);
                float checkY = player.pos_y + (0.0f * sinA + (barrelLen * checkpoints[p]) * cosA);
                float barrelDiffX = currentEnemy.pos.x - checkX;
                float barrelDiffY = currentEnemy.pos.y - checkY;
                float barrelDistSq = (barrelDiffX * barrelDiffX) + (barrelDiffY * barrelDiffY);
                float enemyRadius = currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO;
                float barrelWidth = 22.0f;
                float barrelThreshold = (barrelWidth / 2.0f) + enemyRadius;

                if (barrelDistSq < (barrelThreshold * barrelThreshold)) {
                    player_init.current_hp -= currentEnemy.hp / 3.0f;
                    currentEnemy.hp = 0;
                    TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
                    playerFlashTimer = 0.15f;
                    break;
                }
            }
        }

        // -- Orbit ability vs enemy --
        if (orbitActive) {
            float orbitDiffX = orbitPosX - currentEnemy.pos.x;
            float orbitDiffY = orbitPosY - currentEnemy.pos.y;
            float orbitDistSq = (orbitDiffX * orbitDiffX) + (orbitDiffY * orbitDiffY);
            float orbitSize = 20.0f;
            float orbitRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + orbitSize;

            if (orbitDistSq < (orbitRadius * orbitRadius)) {
                currentEnemy.hp = 0;
                TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
            }
        }

        // -- Player bullets vs enemy --
        float currentdmg = calculate_max_stats(1); // calculate once per enemy, not per bullet
        for (auto& boolet : bulletList) {
            if (!boolet.isActive) continue;

            float bulletDiffX = boolet.posX - currentEnemy.pos.x;
            float bulletDiffY = boolet.posY - currentEnemy.pos.y;
            float bulletDistSq = (bulletDiffX * bulletDiffX) + (bulletDiffY * bulletDiffY);
            float bulletRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + boolet.size;

            if (bulletDistSq < (bulletRadius * bulletRadius)) {
                SFX::playSFX(SFX_ENEMY_HIT);
                TriggerBulletImpact(boolet.posX, boolet.posY, boolet.directionX, boolet.directionY);
                currentEnemy.hp -= (int)(currentdmg * boolet.damagemul);
                boolet.isActive = false;

                if (currentEnemy.hp <= 0 && currentEnemy.alive)
                    TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
            }
        }

        // -- Enemy bullets vs player body and barrel --
        for (auto& enBullet : enemyBulletList) {
            if (!enBullet.isActive) continue;

            float enBulletDiffX = enBullet.posX - player.pos_x;
            float enBulletDiffY = enBullet.posY - player.pos_y;
            float enBulletDistSq = (enBulletDiffX * enBulletDiffX) + (enBulletDiffY * enBulletDiffY);
            float enBulletRadius = player.scale + enBullet.size;

            bool hit = (enBulletDistSq < (enBulletRadius * enBulletRadius));

            // If body not hit, also check 3 points along the barrel
            if (!hit) {
                float barrelLen = GameConfig::Tank::BARREL_LENGTH * (player.scale / GameConfig::Tank::SCALE);
                float cosA = cosf(player.currentAngle);
                float sinA = sinf(player.currentAngle);
                float checkpoints[3] = { 0.5f, 0.75f, 1.0f };

                for (int p = 0; p < 3; p++) {
                    float checkX = player.pos_x + (0.0f * cosA - (barrelLen * checkpoints[p]) * sinA);
                    float checkY = player.pos_y + (0.0f * sinA + (barrelLen * checkpoints[p]) * cosA);
                    float ebDiffX = enBullet.posX - checkX;
                    float ebDiffY = enBullet.posY - checkY;
                    float ebDistSq = (ebDiffX * ebDiffX) + (ebDiffY * ebDiffY);
                    float checkRadius = enBullet.size;

                    if (ebDistSq < (checkRadius * checkRadius)) {
                        hit = true;
                        break;
                    }
                }
            }

            if (hit) {
                SFX::playSFX(SFX_PLAYER_HIT);
                TriggerBulletImpact(enBullet.posX, enBullet.posY, enBullet.directionX, enBullet.directionY);
                player_init.current_hp -= 10;
                enBullet.isActive = false;
                playerFlashTimer = 0.15f;
            }
        }
    }

    // -- Minion body vs player and player bullets --
    for (auto& minion : minionPool) {
        if (!minion.alive) continue;

        float minionDiffX = player.pos_x - minion.pos.x;
        float minionDiffY = player.pos_y - minion.pos.y;
        float minionDistSq = (minionDiffX * minionDiffX) + (minionDiffY * minionDiffY);
        float minionRadius = (minion.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;

        if (minionDistSq < minionRadius * minionRadius) {
            player_init.current_hp -= minion.hp / 3.0f;
            minion.hp = 0;
            TriggerExplosion(minion.pos.x, minion.pos.y, minion.scale);
            playerFlashTimer = 0.15f;
        }

        float minionDmg = calculate_max_stats(1);
        for (auto& boolet : bulletList) {
            if (!boolet.isActive) continue;

            float mbDiffX = boolet.posX - minion.pos.x;
            float mbDiffY = boolet.posY - minion.pos.y;
            float mbDistSq = (mbDiffX * mbDiffX) + (mbDiffY * mbDiffY);
            float mbRadius = (minion.scale * GameConfig::Enemy::HITBOX_RATIO) + boolet.size;

            if (mbDistSq < mbRadius * mbRadius) {
                TriggerBulletImpact(boolet.posX, boolet.posY, boolet.directionX, boolet.directionY);
                minion.hp -= (int)(minionDmg * boolet.damagemul);
                boolet.isActive = false;
                if (minion.hp <= 0 && minion.alive)
                    TriggerExplosion(minion.pos.x, minion.pos.y, minion.scale);
            }
        }
    }
}

// =============================================================================
// LOAD GAME
// =============================================================================

// ~ Brief: Load all assets needed for gameplay — meshes, textures, audio, world,
//          and subsystem resources. Called once when entering the game state.
void LoadGame() {
    LoadDebug1();
    LoadBullets();
    LoadEnemies();
    LoadBoss();
    LoadResults();
    Animations_Load();
    PauseScreen::LoadPause();
    World::Load_World();

    // Shared geometry meshes used throughout the game draw calls
    MeshCircle = Gfx::createCircleMesh(0xFFFFFFFF);
    MeshRect = Gfx::createRectMesh(0xFFFFFFFF);
    MeshTriangle = Gfx::createTriangleMesh(0xFFFFFFFF);
}

// ~ Brief: Initialize all runtime game state. If a valid save file exists and this
//          is not a continuation of a run already in progress (gamecurrrun == false),
//          restore wave, player stats, and cards from the save. Otherwise start fresh.
//          Always resets player position, bullet lists, and enemy pools.
void InitializeGame() {
    srand(static_cast<unsigned int>(time(NULL)));

    World::Init_World();

    // Reset player transform and fire state
    player.pos_x = 0;
    player.pos_y = 0;
    player.currentAngle = 0;
    player.scale = GameConfig::Tank::SCALE;
    player.barrelCount = 1;

    bulletFireTimer = 0;
    enemySpawnTimer = 0;
    waveActiveTimer = 0.0f;

    // Clear bullet and enemy pools
    for (int i = 0; i < GameConfig::MAX_BULLETS_COUNT; i++) bulletList[i].isActive = false;
    for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; i++) ResetEnemy(&enemyPool[i]);
    for (auto& minion : minionPool) ResetEnemy(&minion);

    // Attempt to restore a saved run — only on the first initialization this session
    SaveData data;
    if (::LoadGame(data) && gamecurrrun == false) {
        // Restore player progression
        currentWave = data.currentWave;
        player_init.current_hp = data.current_hp;
        player_init.current_xp = data.current_xp;
        player_init.player_level = data.player_level;
        player_init.skill_point = data.skill_point;
        for (int i = 0; i < 5; i++)
            player_init.upgradeLevels[i] = data.upgradeLevels[i];

        // Restore card pools — each card is looked up by ID and assigned to its deck
        for (auto& id : data.shopCardIDs) {
            Card card;
            Cards::GetCardByID(id, card);
            card.from = DECK::SHOP;
            allCards[0].push_back(card);
        }
        for (auto& id : data.activeCardIDs) {
            Card card;
            Cards::GetCardByID(id, card);
            card.from = DECK::ACTIVE;
            allCards[1].push_back(card);
        }
        for (auto& id : data.inventoryCardIDs) {
            Card card;
            Cards::GetCardByID(id, card);
            card.from = DECK::BAG;
            allCards[2].push_back(card);
        }

        // Resume at card shop if player saved from there
        if (data.lastGameState == GS_CARD_SHOP)
            GS_next = GS_CARD_SHOP;

        GenerateWave(currentWave, player);
        //std::cout << "Loaded saves\n";
    }
    else {
        // No save or mid-session restart — begin a fresh run
        GenerateWave(currentWave, player);
        //std::cout << "New Run\n";
    }

    SFX::playBGM();
    gamecurrrun = true;        // flag prevents re-loading save on subsequent Init calls this session
    Cards::computeCardEffects();
}

// =============================================================================
// UPDATE GAME
// =============================================================================

// ~ Brief: Main gameplay update — processes input, movement, combat, wave management,
//          and win/loss conditions each frame. Skips most updates when the upgrade
//          menu or pause screen is active. Collision is skipped while paused.
void UpdateGame() {
    float deltaTime = (float)AEFrameRateControllerGetFrameTime();
    UpdateDebug1();

    if (!player_init.menu_open) {
        PauseScreen::UpdatePause();

        if (!player_init.menu_open && !PauseScreen::isPaused) {

            // -- Cheat mode: detect newly applied card upgrades and sync state --
            if (cheatsOn) {
                if (upgradeFlag != prevUpgradeFlag) {
                    Cards::computeCardEffects();

                    // Apply each newly set upgrade flag to the relevant game state
                    if ((upgradeFlag & UPGRADE_BIG_CANNON) && !(prevUpgradeFlag & UPGRADE_BIG_CANNON))  bigcannon = true;
                    if ((upgradeFlag & UPGRADE_CANNON_180) && !(prevUpgradeFlag & UPGRADE_CANNON_180))  dualback = true;
                    if ((upgradeFlag & UPGRADE_DUAL_CANNON) && !(prevUpgradeFlag & UPGRADE_DUAL_CANNON)) player.barrelCount = 2;
                    if ((upgradeFlag & UPGRADE_ORBIT) && !(prevUpgradeFlag & UPGRADE_ORBIT))       orbitActive = true;

                    prevUpgradeFlag = upgradeFlag;
                }
            }

            // -- Core gameplay systems --
            UpdateWaveSpawning(deltaTime, player);
            waveActiveTimer += deltaTime;
            Animations_Update(deltaTime);
            DualBack(player);
            drawBigCannon(player);
            drawBigTank(player);
            movePlayer(player, deltaTime);
            rotatePlayer(player);
            ShootBullet(player, deltaTime);
            updateOrbit(player, deltaTime);
            updateBullets(player, deltaTime);
            updateSmoke(deltaTime);
            updateEnemyPhysics(player, deltaTime);
            updateEnemyBullets(deltaTime);
            UpdateBossPhysics(currentboss, player, deltaTime);
            BossCollision(currentboss, player, orbitActive, orbitPosX, orbitPosY);
            updateMinionPhysics(player, deltaTime);

            // Tick down player hit flash
            if (playerFlashTimer > 0.0f) playerFlashTimer -= deltaTime;

            // -- Loss condition: player HP depleted --
            if (static_cast<int>(player_init.current_hp) <= 0) {
                gameWon = false;
                GS_next = GS_RESULTS;
                return;
            }

            // -- Wave / win management: only check after wave has been active for 2s
            //    to prevent instant clear on spawn --
            if (GS_next == GS_GAME && IsWaveCleared() && waveActiveTimer > 2.0f) {
                bool isFinalBoss = (currentWave == (numofBosses * 5));

                if (isFinalBoss) {
                    // Win condition: all boss waves completed
                    gameWon = true;
                    GS_next = GS_RESULTS;
                }
                else {
                    // Advance to next wave via card shop
                    currentWave++;
                    GS_next = GS_CARD_SHOP;
                }
                return;
            }

            // -- Debug: skip current wave with key 8 when cheats are on --
            if (cheatsOn && AEInputCheckTriggered(AEVK_8)) {
                bool isFinalBoss = (currentWave == (numofBosses * 5));
                skipWave(player);
                if (isFinalBoss) {
                    gameWon = true;
                    GS_next = GS_RESULTS;
                }
                else {
                    GS_next = GS_CARD_SHOP;
                }
                return;
            }
        }

        // Run collision every frame except when paused
        if (!PauseScreen::isPaused) circlerectcollision();

        AEGfxSetCamPosition(player.pos_x, player.pos_y);
    }
}

// =============================================================================
// DRAW GAME
// =============================================================================

// ~ Brief: Render all visible game elements each frame in draw order:
//          world tiles, indicators, bullets, orbit, smoke, player tank,
//          enemies, minions, boss, HUD, animations, and pause overlay.
void DrawGame() {
    if (MeshRect == nullptr || MeshCircle == nullptr) return;

    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
    AEGfxSetCamPosition(player.pos_x, player.pos_y);

    // -- World tiles --
    World::Draw_World();

    // -- Tutorial overlay (if enabled) --
    if (tutorialOn) printtutorial();

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // -- Off-screen enemy indicators --
    DrawEnemyIndicators(player, MeshTriangle);

    // -- Player bullets --
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBulletTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    for (const auto& boolet : bulletList) {
        if (boolet.isActive) {
            float angle = atan2f(boolet.directionY, boolet.directionX) - HALF_PI;
            Gfx::printMesh(pBulletMesh, { boolet.posX, boolet.posY }, { boolet.size * 1.5f, boolet.size * 2.0f }, angle, { 0.f, 0.f }, true);
        }
    }

    // -- Enemy bullets --
    AEGfxTextureSet(pEnemyBulletTex, 0, 0);
    for (const auto& enBullet : enemyBulletList) {
        if (enBullet.isActive) {
            float angle = atan2f(enBullet.directionY, enBullet.directionX) - HALF_PI;
            Gfx::printMesh(pEnemyBulletMesh, { enBullet.posX, enBullet.posY }, { enBullet.size * 1.5f, enBullet.size * 2.0f }, angle, { 0.f, 0.f }, true);
        }
    }
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // -- Orbit shield --
    DrawOrbit(player.scale);

    // -- Smoke particles (low HP effect) --
    AEGfxSetColorToMultiply(0.4f, 0.4f, 0.4f, 1.0f);
    for (const auto& s : smokes) {
        if (s.isActive)
            Gfx::printMesh(MeshCircle, { s.posX, s.posY }, { s.size, s.size }, 0.0f);
    }

    // -- Player tank --
    AEVec2 playerPos = { player.pos_x, player.pos_y };
    float visualScale = player.scale / GameConfig::Tank::SCALE;

    // Tracks — flash red when hit, dark grey otherwise
    AEGfxSetColorToMultiply(
        playerFlashTimer > 0.0f ? 1.0f : 0.1f,
        playerFlashTimer > 0.0f ? 0.0f : 0.1f,
        playerFlashTimer > 0.0f ? 0.0f : 0.1f,
        1.0f
    );
    AEVec2 trackSize = { GameConfig::Tank::TRACK_WIDTH * visualScale, GameConfig::Tank::TRACK_HEIGHT * visualScale };
    float scaledTrackOffset = GameConfig::Tank::TRACK_OFFSET_X * visualScale;
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { -scaledTrackOffset, 0.0f }); // left track
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { scaledTrackOffset, 0.0f }); // right track

    // Body — flash red when hit, green otherwise
    AEGfxSetColorToMultiply(
        playerFlashTimer > 0.0f ? 1.0f : 0.2f,
        playerFlashTimer > 0.0f ? 0.0f : 0.6f,
        playerFlashTimer > 0.0f ? 0.0f : 0.2f,
        1.0f
    );
    Gfx::printMesh(MeshRect, playerPos, { GameConfig::Tank::BODY_WIDTH * visualScale, GameConfig::Tank::BODY_HEIGHT * visualScale }, player.currentAngle);

    // Barrels — width doubled if bigcannon upgrade is active
    float cannonwidthnow = bigcannon ? GameConfig::Tank::BARREL_WIDTH * 2.0f : GameConfig::Tank::BARREL_WIDTH;
    if (dualback) {
        // Forward and backward barrels for 180-degree shot upgrade
        DrawMultiBarrels(1, 0.0f, GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale, player.currentAngle, player.pos_x, player.pos_y, cannonwidthnow * visualScale, GameConfig::Tank::BARREL_LENGTH * visualScale, MeshRect);
        DrawMultiBarrels(1, 0.0f, GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale, player.currentAngle + PI, player.pos_x, player.pos_y, cannonwidthnow * visualScale, GameConfig::Tank::BARREL_LENGTH * visualScale, MeshRect);
    }
    else {
        DrawMultiBarrels(
            static_cast<int>(player.barrelCount),
            GameConfig::Tank::BARREL_GAP * visualScale,
            GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale,
            player.currentAngle,
            player.pos_x,
            player.pos_y,
            cannonwidthnow * visualScale,
            GameConfig::Tank::BARREL_LENGTH * visualScale,
            MeshRect
        );
    }

    // Turret — flash red when hit, light green otherwise
    AEGfxSetColorToMultiply(
        playerFlashTimer > 0.0f ? 1.0f : 0.3f,
        playerFlashTimer > 0.0f ? 0.0f : 0.7f,
        playerFlashTimer > 0.0f ? 0.0f : 0.3f,
        1.0f
    );
    Gfx::printMesh(MeshCircle, playerPos, { GameConfig::Tank::TURRET_SIZE * visualScale, GameConfig::Tank::TURRET_SIZE * visualScale }, player.currentAngle);

    // -- Enemies -- textured, color-coded by type via texture index --
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetTransparency(1.0f);
    for (const auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive && currentEnemy.scale <= 0) continue;

        float rotationRad = currentEnemy.rotation * (PI / 180.0f);

        if (currentEnemy.enemtype == PASSIVE) {
            // Index 0 = small box, index 1 = big box
            int idx = (currentEnemy.scale > GameConfig::Enemy::SIZE_SMALL + 10.0f) ? 1 : 0;
            AEGfxTextureSet(pEnemyTex[idx], 0, 0);
        }
        else if (currentEnemy.enemtype == ATTACK)  AEGfxTextureSet(pEnemyTex[2], 0, 0);
        else if (currentEnemy.enemtype == SHOOTER) AEGfxTextureSet(pEnemyTex[3], 0, 0);

        Gfx::printMesh(pEnemyMesh, currentEnemy.pos, { currentEnemy.scale, currentEnemy.scale }, rotationRad, { 0.f, 0.f }, true);
    }
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // -- Minions (BOSS2 spawned enemies) --
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pMinionTex, 0, 0);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    for (const auto& minion : minionPool) {
        if (minion.alive || minion.scale > 0) {
            float rotationRad = minion.rotation * (PI / 180.0f);
            Gfx::printMesh(pBossMesh, minion.pos, { minion.scale, minion.scale }, rotationRad, { 0.f, 0.f }, true);
        }
    }
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // -- Boss body and HP bar --
    if (currentboss.alive) {
        DrawBoss(currentboss, MeshRect, MeshCircle);
        DrawBossHP(currentboss, MeshRect, MeshCircle, player);
    }

    // -- HUD and overlays --
    printEnemyCount();
    Animations_Draw();
    DrawDebug1();
    PauseScreen::DrawPauseButton();
    PauseScreen::DrawPause();
}

// =============================================================================
// FREE / UNLOAD
// =============================================================================

// ~ Brief: Free all meshes and release all subsystem resources when leaving the
//          game state. Resets the camera to the origin to avoid stale transforms.
void FreeGame() {
    AEGfxSetCamPosition(0.0f, 0.0f);

    if (MeshRect) { AEGfxMeshFree(MeshRect);     MeshRect = nullptr; }
    if (MeshCircle) { AEGfxMeshFree(MeshCircle);   MeshCircle = nullptr; }
    if (MeshTriangle) { AEGfxMeshFree(MeshTriangle); MeshTriangle = nullptr; }

    FreeDebug1();
    FreeBoss();
    FreeEnemies();
    FreeBullets();
    FreeResults();
    Animations_Free();
    PauseScreen::FreePause();
    World::Free_World();
}

// ~ Brief: No persistent assets to unload for the game state —
//          all cleanup is handled in FreeGame.
void UnloadGame() {}