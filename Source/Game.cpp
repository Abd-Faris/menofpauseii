#include "MasterHeader.h"



// ===========================================================================
// GLOBAL OBJECTS
// ===========================================================================

// -- Enemy Pool --
//std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;
//f64 enemySpawnTimer = 0;




namespace {
    // -- Assets --
    s8 boldPixelsFont;
    AEGfxVertexList* MeshRect = nullptr;
    AEGfxVertexList* MeshCircle = nullptr;
    AEGfxVertexList* MeshTriangle = nullptr;

    // -- Player State --
    // Note: Ensure your struct in Structs.h has 'int barrelCount'
    shape player = { GameConfig::Tank::SCALE, 0.0f, 0.0f, 0.0f, 1 };
    





}

// ===========================================================================
// HELPER FUNCTIONS
// ===========================================================================

// ===========================================================================
// COLLISION LOGIC
// ===========================================================================
void circlerectcollision() {
    for (auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive) continue;

        if (1) {
            float differenceX = player.pos_x - currentEnemy.pos.x;
            float differenceY = player.pos_y - currentEnemy.pos.y;
            float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

            float collisionRadius = (currentEnemy.scale * GameConfig::Enemy::HITBOX_RATIO) + player.scale;
            float currentdmg = calculate_max_stats(1);

            if (distanceSquared < (collisionRadius * collisionRadius)) {
                player_init.current_hp -= currentEnemy.hp / 5;
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
    }
}

// ===========================================================================
// LOAD GAME
// ===========================================================================
void LoadGame() {
    LoadDebug1();
    Animations_Load();
    boldPixelsFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);

    // Mesh: Circle
    //AEGfxMeshStart();
    //int slices = 40;
    //float angleStep = TWO_PI/ slices;
    //for (int i = 0; i < slices; i++) {
    //    AEGfxTriAdd(
    //        0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,
    //        cosf(i * angleStep), sinf(i * angleStep), 0xFFFFFFFF, 1.0f, 1.0f,
    //        cosf((i + 1) * angleStep), sinf((i + 1) * angleStep), 0xFFFFFFFF, 0.0f, 0.0f);
    //}
    //MeshCircle = AEGfxMeshEnd();
    MeshCircle = Gfx::createCircleMesh(0xFFFFFFFF);

    // Mesh: Rect
    //AEGfxMeshStart();
    //AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    //AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    //MeshRect = AEGfxMeshEnd();
    MeshRect = Gfx::createRectMesh("center", 0xFFFFFFFFF);

    MeshTriangle = Gfx::createTriangleMesh(0xFFFFFFFF);

    // Reset Player
    player.pos_x = 0;
    player.pos_y = 0;
    player.currentAngle = 0;
    player.scale = GameConfig::Tank::SCALE;
    player.barrelCount = 1; // Default to Single Barrel

    bulletFireTimer = 0;
    enemySpawnTimer = 0;

    for (int i = 0; i < GameConfig::MAX_BULLETS_COUNT; i++) bulletList[i].isActive = false;
    for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; i++) ResetEnemy(&enemyPool[i]);
    for (int i = 0; i < 5; i++) SpawnOneEnemy(false, player);
}

// ===========================================================================
// UPDATE GAME
// ===========================================================================
void UpdateGame() {
	float deltaTime = (float)AEFrameRateControllerGetFrameTime();
	UpdateDebug1();

	if (!player_init.menu_open) {


		// --- KEY '7': TOGGLE DUAL UPGRADE ---

		// Update Animations
		Animations_Update(deltaTime);

		// --- KEY '7': TOGGLE UPGRADE ---

		drawBigTank(player);

		// --- KEY 'U': TOGGLE BIG CANNON ---
		drawBigCannon(player);

		// 1. Move Player
		movePlayer(player, deltaTime);

		// 2. Rotate Player
		rotatePlayer(player);

		// 3. Shooting (UPDATED FOR MULTI-BARREL)
		ShootBullet(player, deltaTime);

		// 4. Move Bullets
		updateBullets(player, deltaTime);

		// 5. Spawn Enemies

		EnemySpawner(player, deltaTime);



		// 6. Enemy Physics
		updateEnemyPhysics(player, deltaTime);
	}

	circlerectcollision();
	AEGfxSetCamPosition(player.pos_x, player.pos_y);


}

// ===========================================================================
// DRAW GAME
// ===========================================================================

void DrawGame() {
    if (MeshRect == nullptr || MeshCircle == nullptr) return;

    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetCamPosition(player.pos_x, player.pos_y);

    // -- Draw Bullets --
    // Bullets use a simple scale and position with no rotation
    AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f);
    for (const auto& boolet : bulletList) {
        if (boolet.isActive) {
            Gfx::printMesh(MeshCircle, { boolet.posX, boolet.posY }, { boolet.size, boolet.size }, 0.0f);
        }
    }

    // -- Draw Player Tank --
    AEVec2 playerPos = { player.pos_x, player.pos_y };
    float visualScale = player.scale / GameConfig::Tank::SCALE;

    // 1. Draw Tracks
    AEGfxSetColorToMultiply(0.1f, 0.1f, 0.1f, 1.0f);
    AEVec2 trackSize = { GameConfig::Tank::TRACK_WIDTH * visualScale, GameConfig::Tank::TRACK_HEIGHT* visualScale };
    float scaledTrackOffset = GameConfig::Tank::TRACK_OFFSET_X * visualScale;

    // Left Track: Offset to the left before rotating
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { -scaledTrackOffset, 0.0f });

    // Right Track: Offset to the right before rotating
    Gfx::printMesh(MeshRect, playerPos, trackSize, player.currentAngle, { scaledTrackOffset, 0.0f });

    // 2. Draw Main Body
    AEGfxSetColorToMultiply(0.2f, 0.6f, 0.2f, 1.0f);
    Gfx::printMesh(MeshRect, playerPos, { GameConfig::Tank::BODY_WIDTH * visualScale, GameConfig::Tank::BODY_HEIGHT * visualScale }, player.currentAngle);
    float cannonwidthnow = bigcannon ? GameConfig::Tank::BARREL_WIDTH * 2.0f : GameConfig::Tank::BARREL_WIDTH;
    // 3. Draw Barrels
    DrawMultiBarrels(
        player.barrelCount,
        GameConfig::Tank::BARREL_GAP * visualScale,
        GameConfig::Tank::BARREL_PIVOT_OFFSET * visualScale,
        player.currentAngle,
        player.pos_x,
        player.pos_y,
        cannonwidthnow*visualScale,
        GameConfig::Tank::BARREL_LENGTH * visualScale,
        MeshRect
    );

    // 4. Draw Turret
    AEGfxSetColorToMultiply(0.3f, 0.7f, 0.3f, 1.0f);
    Gfx::printMesh(MeshCircle, playerPos, { GameConfig::Tank::TURRET_SIZE * visualScale, GameConfig::Tank::TURRET_SIZE * visualScale }, player.currentAngle);

    // -- Draw Enemies --
    for (const auto& currentEnemy : enemyPool) {
        if (currentEnemy.alive || currentEnemy.scale > 0) {
            // Set color based on enemy size
            if (currentEnemy.enemtype == PASSIVE) {
            if (currentEnemy.scale > (GameConfig::Enemy::SIZE_SMALL + 10.0f))
                AEGfxSetColorToMultiply(0.2f, 0.2f, 0.8f, 1);
            else
                AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1);
            }

            if (currentEnemy.enemtype == ATTACK) {
                AEGfxSetColorToMultiply(0.6f, 0.2f, 0.6f, 1);
                float rotationRad = currentEnemy.rotation * (PI / 180.0f);
                Gfx::printMesh(MeshTriangle, currentEnemy.pos, { currentEnemy.scale, currentEnemy.scale }, rotationRad);
                continue;
            }
            // Convert degrees to radians for the rotation parameter
            float rotationRad = currentEnemy.rotation * (PI / 180.0f);
            Gfx::printMesh(MeshRect, currentEnemy.pos, { currentEnemy.scale, currentEnemy.scale }, rotationRad);
        }
    }

    Animations_Draw();
    DrawDebug1();
    

}



void FreeGame() {
    AEGfxDestroyFont(boldPixelsFont);
    if (MeshRect) AEGfxMeshFree(MeshRect);
    if (MeshCircle) AEGfxMeshFree(MeshCircle);
    FreeDebug1();
    Animations_Free();
}