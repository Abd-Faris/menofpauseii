#include "MasterHeader.h"



// ===========================================================================
// GLOBAL OBJECTS
// ===========================================================================
extern bool dualback, orbitActive;
extern float orbitAngle, orbitPosX, orbitPosY;
bool mousereleased = false;
int currentWave = 1;
float playerFlashTimer = 0.0f;
float waveActiveTimer = 0.0f;


namespace {
    // -- Assets --
    AEGfxVertexList* MeshRect = nullptr;
    AEGfxVertexList* MeshCircle = nullptr;
    AEGfxVertexList* MeshTriangle = nullptr;
    

    // -- Player State --
    // Note: Ensure your struct in Structs.h has 'int barrelCount'
    shape player = { GameConfig::Tank::SCALE, 0.0f, 0.0f, 0.0f, 1 };
}

// ===========================================================================
// COLLISION LOGIC
// ===========================================================================
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
            // -- Player barrel vs enemy --
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

        // -- Orbit vs enemy --
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
        float currentdmg = calculate_max_stats(1);
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

        // -- Enemy bullets vs player --
        for (auto& enBullet : enemyBulletList) {
            if (!enBullet.isActive) continue;

            float enBulletDiffX = enBullet.posX - player.pos_x;
            float enBulletDiffY = enBullet.posY - player.pos_y;
            float enBulletDistSq = (enBulletDiffX * enBulletDiffX) + (enBulletDiffY * enBulletDiffY);
            float enBulletRadius = player.scale + enBullet.size;

            bool hit = (enBulletDistSq < (enBulletRadius * enBulletRadius));

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

    // -- Minions --
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

// ===========================================================================
// LOAD GAME
// ===========================================================================
void LoadGame() {
   
    LoadDebug1();
    LoadBullets();
    LoadEnemies();
    LoadBoss();
    LoadResults();
    Animations_Load();
    PauseScreen::LoadPause();
	World::Load_World();
    
    // Create Meshes
    MeshCircle = Gfx::createCircleMesh(0xFFFFFFFF);
    MeshRect = Gfx::createRectMesh(0xFFFFFFFF);
    MeshTriangle = Gfx::createTriangleMesh(0xFFFFFFFF);
}

void InitializeGame() {
    //seed the rng
    srand(static_cast<unsigned int>(time(NULL))); 
    
    World::Init_World();

    // Reset Player
    player.pos_x = 0;
    player.pos_y = 0;
    player.currentAngle = 0;
    player.scale = GameConfig::Tank::SCALE;
    player.barrelCount = 1; 

    bulletFireTimer = 0;
    enemySpawnTimer = 0;
    waveActiveTimer = 0.0f;

    for (int i = 0; i < GameConfig::MAX_BULLETS_COUNT; i++) bulletList[i].isActive = false;
    for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; i++) ResetEnemy(&enemyPool[i]);
    for (auto& minion : minionPool) ResetEnemy(&minion);

    // Initialize Wave
    SaveData data;
    if (::LoadGame(data) && gamecurrrun == false) {
        currentWave = data.currentWave;
        player_init.current_hp = data.current_hp;
        player_init.current_xp = data.current_xp;
        player_init.player_level = data.player_level;
        player_init.skill_point = data.skill_point;
        for (int i = 0; i < 5; i++)
            player_init.upgradeLevels[i] = data.upgradeLevels[i];

        // Use globals directly — no local redeclaration
        for ( auto& id : data.shopCardIDs) {
            Card card;
            Cards::GetCardByID(id, card);
            //card.info.ID = id;
            card.from = DECK::SHOP;
            allCards[0].push_back(card);
        }
        for ( auto& id : data.activeCardIDs) {
            Card card;
            Cards::GetCardByID(id, card);
            //card.info.ID = id;
            card.from = DECK::ACTIVE;
            allCards[1].push_back(card);
        }
        for ( auto& id : data.inventoryCardIDs) {
            Card card;
            Cards::GetCardByID(id, card);
            //card.info.ID = id;
            card.from = DECK::BAG;
            allCards[2].push_back(card);
        }

        if (data.lastGameState == GS_CARD_SHOP)
            GS_next = GS_CARD_SHOP;

        GenerateWave(currentWave, player);

        std::cout << "Loaded saves\n";

       
    }
    else {
        GenerateWave(currentWave, player);
        std::cout << "New Run\n";
    }
    SFX::playBGM();
    gamecurrrun = true;
    Cards::computeCardEffects();
}

// ===========================================================================
// UPDATE GAME
// ===========================================================================
void UpdateGame() {
    float deltaTime = (float)AEFrameRateControllerGetFrameTime();
    UpdateDebug1();

    if (!player_init.menu_open) {
        PauseScreen::UpdatePause();

        if (!player_init.menu_open && !PauseScreen::isPaused) {

            // 1. Basic Movement & Combat Updates
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

            if (playerFlashTimer > 0.0f) playerFlashTimer -= deltaTime;

            // 2. LOSS CONDITION
            if (static_cast<int>(player_init.current_hp) <= 0) {
                gameWon = false;
                GS_next = GS_RESULTS; // Exit to Results state
                return;
            }

            // 3. WAVE / WIN MANAGEMENT
            if (GS_next == GS_GAME && IsWaveCleared() && waveActiveTimer > 2.0f) {
                bool isFinalBoss = (currentWave == (numofBosses * 5));

                if (isFinalBoss) {
                    gameWon = true;
                    GS_next = GS_RESULTS; // Exit to Results state
                }
                else {
                    currentWave++;
                    GS_next = GS_CARD_SHOP; // Transition to Shop
                }
                return;
            }

            // 4. DEBUG: Skip Wave
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

        if (!PauseScreen::isPaused) circlerectcollision();

        AEGfxSetCamPosition(player.pos_x, player.pos_y);
    }
}

// ===========================================================================
// DRAW GAME
// ===========================================================================

void DrawGame() {
    if (MeshRect == nullptr || MeshCircle == nullptr) return;

    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
    AEGfxSetCamPosition(player.pos_x, player.pos_y);

    // -- Draw World --
    World::Draw_World();

    if (tutorialOn) printtutorial();

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    DrawEnemyIndicators(player, MeshTriangle);
    // -- Draw Bullets --
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pBulletTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    for (const auto& boolet : bulletList) {
        if (boolet.isActive) {
            //calculate the bullet direction
            float angle = atan2f(boolet.directionY, boolet.directionX) - HALF_PI;
            Gfx::printMesh(pBulletMesh, { boolet.posX, boolet.posY }, { boolet.size * 1.5f, boolet.size * 2.0f }, angle, { 0.f, 0.f }, true);
        }
    }

    //enemy bullets
    AEGfxTextureSet(pEnemyBulletTex, 0, 0);
    for (const auto& enBullet : enemyBulletList) {
        if (enBullet.isActive) {
            float angle = atan2f(enBullet.directionY, enBullet.directionX) - HALF_PI;
            Gfx::printMesh(pEnemyBulletMesh, { enBullet.posX, enBullet.posY }, { enBullet.size * 1.5f, enBullet.size * 2.0f }, angle, { 0.f, 0.f }, true);
        }
    }
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    if (orbitActive) {
        AEGfxSetColorToMultiply(0.0f, 1.0f, 1.0f, 1.0f); // Bright Cyan
        float orbitSize = 40.0f;
        Gfx::printMesh(MeshCircle, { orbitPosX, orbitPosY }, { orbitSize, orbitSize }, orbitAngle);
    }
    // -- smokes--
    AEGfxSetColorToMultiply(0.4f, 0.4f, 0.4f, 1.0f); // Dark Grey color
    for (const auto& s : smokes) {
        if (s.isActive) {
            Gfx::printMesh(MeshCircle, { s.posX, s.posY }, { s.size, s.size }, 0.0f);
        }
    }

    // -- Draw Player Tank --
    AEVec2 playerPos = { player.pos_x, player.pos_y };
    float visualScale = player.scale / GameConfig::Tank::SCALE;

    // 1. Draw Tracks
    if (playerFlashTimer > 0.0f) {
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Flash Red
    }
    else {
        AEGfxSetColorToMultiply(0.1f, 0.1f, 0.1f, 1.0f); // Normal Dark Grey
    }
    AEVec2 trackSize = { GameConfig::Tank::TRACK_WIDTH * visualScale, GameConfig::Tank::TRACK_HEIGHT* visualScale };
    float scaledTrackOffset = GameConfig::Tank::TRACK_OFFSET_X * visualScale;

    // Left Track: Offset to the left before rotating
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { -scaledTrackOffset, 0.0f });

    // Right Track: Offset to the right before rotating
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { scaledTrackOffset, 0.0f });

    // 2. Draw Main Body
    if (playerFlashTimer > 0.0f) {
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Bright Red (or use 1,1,1 for White)
    }
    else {
        AEGfxSetColorToMultiply(0.2f, 0.6f, 0.2f, 1.0f); // Normal Green
    }

    Gfx::printMesh(MeshRect, playerPos, { GameConfig::Tank::BODY_WIDTH * visualScale, GameConfig::Tank::BODY_HEIGHT * visualScale }, player.currentAngle);
    float cannonwidthnow = bigcannon ? GameConfig::Tank::BARREL_WIDTH * 2.0f : GameConfig::Tank::BARREL_WIDTH;
    // 3. Draw Barrels

    if (dualback) {
        // Draw forward barrel
        DrawMultiBarrels(1, 0.0f, GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale, player.currentAngle, player.pos_x, player.pos_y, cannonwidthnow * visualScale, GameConfig::Tank::BARREL_LENGTH * visualScale, MeshRect);

        // Draw backward barrel (+ PI rotates it 180 degrees)
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

    // 4. Draw Turret
    if (playerFlashTimer > 0.0f) {
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Flash Red
    }
    else {
        AEGfxSetColorToMultiply(0.3f, 0.7f, 0.3f, 1.0f); // Normal Light Green
    }
    Gfx::printMesh(MeshCircle, playerPos, { GameConfig::Tank::TURRET_SIZE * visualScale, GameConfig::Tank::TURRET_SIZE * visualScale }, player.currentAngle);

    // -- Draw Enemies --
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetTransparency(1.0f);
    for (const auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive && currentEnemy.scale <= 0) continue;

        float rotationRad = currentEnemy.rotation * (PI / 180.0f);

        if (currentEnemy.enemtype == PASSIVE) {
            // index 0 = small, index 1 = big
            int idx = (currentEnemy.scale > GameConfig::Enemy::SIZE_SMALL + 10.0f) ? 1 : 0;
            AEGfxTextureSet(pEnemyTex[idx], 0, 0);
        }
        else if (currentEnemy.enemtype == ATTACK) {
            AEGfxTextureSet(pEnemyTex[2], 0, 0);
        }
        else if (currentEnemy.enemtype == SHOOTER) {
            AEGfxTextureSet(pEnemyTex[3], 0, 0);
        }

        Gfx::printMesh(pEnemyMesh, currentEnemy.pos, { currentEnemy.scale, currentEnemy.scale }, rotationRad, { 0.f, 0.f }, true);
    }
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // -- Draw Minions --
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

    if (currentboss.alive) {
        DrawBoss(currentboss, MeshRect, MeshCircle);
        DrawBossHP(currentboss, MeshRect, MeshCircle, player);
    }

    printEnemyCount();
    Animations_Draw();
    DrawDebug1();
    PauseScreen::DrawPauseButton();
    PauseScreen::DrawPause();

    
}

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

void UnloadGame() {}