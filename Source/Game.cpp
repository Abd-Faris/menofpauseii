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
    AEAudio mainbgm{ nullptr };
    AEAudioGroup bgm{ nullptr };
    

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

            float differenceX = player.pos_x - currentEnemy.pos.x;
            float differenceY = player.pos_y - currentEnemy.pos.y;
            float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

            float collisionRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;
            float currentdmg = calculate_max_stats(1);

            if (distanceSquared < (collisionRadius * collisionRadius)) {
                player_init.current_hp -= currentEnemy.hp / 3.0f;
                currentEnemy.hp = 0;
                TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
            }

        else {
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
                float diffX = currentEnemy.pos.x - checkX;
                float diffY = currentEnemy.pos.y - checkY;
                float distSq = (diffX * diffX) + (diffY * diffY);
                float enemyRadius = currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO;

                float barrelWidth = 22.0f;
                float collisionThreshold = (barrelWidth / 2.0f) + enemyRadius;
                if (distSq < (collisionThreshold * collisionThreshold)) {
                    player_init.current_hp -= currentEnemy.hp / 3.0f;
                    currentEnemy.hp = 0;
                    TriggerExplosion(currentEnemy.pos.x, currentEnemy.pos.y, currentEnemy.scale);
                    break;
                }
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
                currentEnemy.hp -= (int)(currentdmg*boolet.damagemul);
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

            if (hit){
                player_init.current_hp -= 10; // player
                enBullet.isActive = false;     // Destroy the enemy bullet
                playerFlashTimer = 0.15f;
            }
        }
    }

    for (auto& minion : minionPool) {
        if (!minion.alive) continue;

        // Player body collision
        float dx = player.pos_x - minion.pos.x;
        float dy = player.pos_y - minion.pos.y;
        float distSq = (dx * dx) + (dy * dy);
        float radius = (minion.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;
        if (distSq < radius * radius) {
            player_init.current_hp -= minion.hp / 3.0f;
            minion.hp = 0;
            TriggerExplosion(minion.pos.x, minion.pos.y, minion.scale);
        }

        // Bullet collision
        for (auto& boolet : bulletList) {
            if (!boolet.isActive) continue;
            float bdx = boolet.posX - minion.pos.x;
            float bdy = boolet.posY - minion.pos.y;
            float bdistSq = (bdx * bdx) + (bdy * bdy);
            float bradius = (minion.scale * GameConfig::Enemy::HITBOX_RATIO) + boolet.size;
            if (bdistSq < bradius * bradius) {
                float dmg = calculate_max_stats(1);
                minion.hp -= (int)(dmg * boolet.damagemul);
                boolet.isActive = false;
                if (minion.hp <= 0 && minion.alive) {
                    TriggerExplosion(minion.pos.x, minion.pos.y, minion.scale);
                }
            }
        }
    }
}

// ===========================================================================
// LOAD GAME
// ===========================================================================
void LoadGame() {
    // load audio assets
    mainbgm = AEAudioLoadMusic("Assets/audio/bgm/game_bgm.mp3");
    bgm = AEAudioCreateGroup();
    AEAudioPlay(mainbgm, bgm, 1.f, 1.f, -1);
   
    LoadDebug1();
    LoadBullets();
    LoadEnemies();
    LoadBoss();
    Animations_Load();
    PauseScreen::LoadPause();
	World::Load_World();
    //!AEInputCheckCurr(AEVK_LBUTTON);
    
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
    player.barrelCount = 1; // Default to Single Barrel

    bulletFireTimer = 0;
    enemySpawnTimer = 0;
    waveActiveTimer = 0.0f;

    for (int i = 0; i < GameConfig::MAX_BULLETS_COUNT; i++) bulletList[i].isActive = false;
    for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; i++) ResetEnemy(&enemyPool[i]);
    for (auto& minion : minionPool) ResetEnemy(&minion);

    // Initialize Wave
    GenerateWave(currentWave, player);
}

// ===========================================================================
// UPDATE GAME
// ===========================================================================
void UpdateGame() {
	float deltaTime = (float)AEFrameRateControllerGetFrameTime();
	
    // Update Xp and levels
    UpdateDebug1();


    if (!player_init.menu_open) {
        // Update Wave Logic
            PauseScreen::UpdatePause();
           
        if (!player_init.menu_open && !PauseScreen::isPaused) {

            // Update Waves
            UpdateWaveSpawning(deltaTime, player);

            waveActiveTimer += deltaTime;
            // Update Animations
            Animations_Update(deltaTime);

            //key 8 back forth
            DualBack(player);

            // --- KEY 'U': TOGGLE BIG CANNON ---
            drawBigCannon(player);

            // --- KEY '7': TOGGLE UPGRADE --
            drawBigTank(player);

            // 1. Move Player
            movePlayer(player, deltaTime);

            // 2. Rotate Player
            rotatePlayer(player);

            // 3. Shooting (UPDATED FOR MULTI-BARREL)
            ShootBullet(player, deltaTime);

            updateOrbit(player, deltaTime);

            // 4. Move Bullets
            updateBullets(player, deltaTime);

            updateSmoke(deltaTime);

        // 5. WAVE MANAGEMENT (Final Boss & Infinite Mode Logic)
        // =========================================================
            if (GS_next == GS_GAME && IsWaveCleared() && waveActiveTimer > 2.0f) {

                // Check if the wave we JUST cleared was the Final Boss
                bool justBeatFinalBoss = (currentWave == (numofBosses * 5));

                // Increase the wave counter
                currentWave++;

                // Only trigger Victory if it was the final boss AND we haven't won yet
                if (justBeatFinalBoss && !gameWon) {
                    gameWon = true;
                    GS_next = GS_RESULTS;
                }
                else {
                    // Normal waves, early bosses, or infinite mode -> Card Shop
                    GS_next = GS_CARD_SHOP;
                }
            }

            // =========================================================
            // FOR DEBUGGING: Skip wave with Z
            // =========================================================
            if (GS_next == GS_GAME && AEInputCheckTriggered(AEVK_Z)) {

                // Check if we are currently skipping the Final Boss
                bool skippingFinalBoss = (currentWave == (numofBosses * 5));

                skipWave(player); // Note: skipWave does currentWave++ inside it!

                if (skippingFinalBoss && !gameWon) {
                    gameWon = true;
                    GS_next = GS_RESULTS;
                }
                else {
                    GS_next = GS_CARD_SHOP;
                }
            }

            if (playerFlashTimer > 0.0f) {
                playerFlashTimer -= deltaTime;
            }

            // 6. Enemy Physics
            updateEnemyPhysics(player, deltaTime);

            // 7. Enemy Bullets
            updateEnemyBullets(deltaTime);

            // 8. Boss Updates
            UpdateBossPhysics(currentboss, player, deltaTime);
            BossCollision(currentboss, player, orbitActive, orbitPosX, orbitPosY);
            updateMinionPhysics(player, deltaTime);

            if (static_cast<int>(player_init.current_hp) <= 0) {
                gameWon = false;
                GS_next = GS_RESULTS;
            }

        }
        if (!PauseScreen::isPaused) {
            circlerectcollision();
        }
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
    Animations_Free();
    PauseScreen::FreePause();
	World::Free_World();
}

void UnloadGame() {
    // unload audio
    AEAudioUnloadAudio(mainbgm);
    AEAudioUnloadAudioGroup(bgm);
}