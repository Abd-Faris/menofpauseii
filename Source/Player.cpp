// -----------------------------Gloomy's Revenge---------------------------- //
// File:	Player.cpp
// Authors:	[Men of Pause II]
// Brief:	This file contains functions for player functionalities and 
//          weapon upgrade states.
// 
// ------------------------------------------------------------------------- //

// ------INCLUDE FILES------------------------------------------------------ //
#include "MasterHeader.h"

// ------GLOBAL VARIABLES & POOLS------------------------------------------- //
namespace {}

BulletObj bulletList[GameConfig::MAX_BULLETS_COUNT];
float bulletFireTimer = 0.0f; // Cooldown timer for the player's gun
bool bigcannon = false;       // State tracker for '1' upgrade
bool dualback = false;        // State tracker for '3' upgrade
bool orbitActive = false;     // State tracker for '4' spinning shield
float orbitAngle = 0.0f;      // Current rotation angle of the shield
float orbitPosX = 0.0f;       // World X position of the shield
float orbitPosY = 0.0f;       // World Y position of the shield
SmokeParticle smokes[100];    // Object pool for damage smoke effect
AEGfxTexture* pBulletTex = nullptr;
AEGfxTexture* pOrbitTex = nullptr;
AEGfxVertexList* pBulletMesh = nullptr;

// ------WEAPON DRAWING LOGIC----------------------------------------------- //

// ~ Brief:	Loads the texture and generates the mesh data for player bullets.
void LoadBullets() {
    pBulletTex = AEGfxTextureLoad("Assets/bulletgreen.png");
    pOrbitTex = AEGfxTextureLoad("./Assets/orbit.png");

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBulletMesh = AEGfxMeshEnd();
}

// ~ Brief:	Draws multiple tank barrels symmetrically relative to the tank's center.
void DrawMultiBarrels(int count, float gap, float pivotOffset, float tankRot, float tankX, float tankY, float barrelWidth, float barrelLength, AEGfxVertexList* MeshRect) {
    if (count <= 0) return;

    // 1. Calculate the starting X offset to keep the entire formation of barrels centered on the turret
    float formationWidth = (float)(count - 1) * gap;
    float startX = -formationWidth / 2.0f;

    if (playerFlashTimer > 0.0f) {
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f); // Flash Red
    }
    else {
        AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f); // Normal Dark Grey
    }

    for (int i = 0; i < count; i++) {
        // 2. Calculate local position relative to the tank's center
        // Spread them along the X-axis and push them forward along Y based on pivot
        float currentLocalX = startX + (i * gap);
        AEVec2 barrelOffset = { currentLocalX, pivotOffset };

        // 3. Define the size of each individual barrel
        AEVec2 barrelSize = { barrelWidth, barrelLength };

        // 4. Use the generalized printMesh to handle the Translation/Rotation/Scale matrix math
        Gfx::printMesh(
            MeshRect,            // The barrel shape
            { tankX, tankY },    // Tank Position (World Coords)
            barrelSize,          // Scale
            tankRot,             // Rotation (Radians)
            barrelOffset         // Local Offset (Position relative to tank center)
        );
    }
}

// ------PLAYER INPUT & UPGRADES-------------------------------------------- //

// ~ Brief:	Handles input and state tracking to toggle the dual-barrel upgrade.
void drawBigTank(shape& player) {

    // LEGIT: force dual cannon on if upgrade is active
    if (upgradeFlag & UPGRADE_DUAL_CANNON) {
        player.barrelCount = 2;
        bigcannon = false; // force off big cannon to prevent clashing
        dualback = false; // Add this!
        return;
    }
    else if (!cheatsOn) {
        player.barrelCount = 1; // reset if upgrade not active
    }

    // CHEATS: Toggle double-barrel mode on/off when '4' is pressed
    if (cheatsOn && AEInputCheckTriggered(AEVK_4)) {
        if (player.barrelCount == 1) {
            // Turn Dual ON
            player.barrelCount = 2;
            bigcannon = false; // Force turn off big cannon to prevent overlapping visual states
            dualback = false;
        }
        else {
            // Turn Dual OFF
            player.barrelCount = 1;
        }
    }
}

// ~ Brief:	Processes WASD input to move the player, handles split-axis wall 
//          collisions, and triggers low-HP smoke effects.
void movePlayer(shape& player, float deltaTime) {
    // 1. Calculate how far the player should move this frame based on stats
    float playerSpeed = calculate_max_stats(2);
    float moveAmount = playerSpeed * deltaTime;

    // 2. Create temporary variables to test the new position
    //float nextX = player.pos_x;
    //float nextY = player.pos_y;

    // 3. Apply WASD / Arrow Key inputs
    float dirX = 0.f;
    float dirY = 0.f;

    if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP))    dirY += 1.f;
    if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN))  dirY -= 1.f;
    if (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT))  dirX -= 1.f;
    if (AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT)) dirX += 1.f;

    // Normalize so diagonal isn't sqrt(2) faster
    float len = sqrt(dirX * dirX + dirY * dirY);
    if (len > 0.f) {
        dirX /= len;
        dirY /= len;
    }

    float nextX = player.pos_x + dirX * moveAmount;
    float nextY = player.pos_y + dirY * moveAmount;

    // 4. Split-Axis Collision Check
    // We check X and Y separately so the tank can "slide" against walls instead of getting stuck completely
    if (!World::CheckCollision(nextX, player.pos_y, player.scale, player.currentAngle)) {
        player.pos_x = nextX; // Apply X movement if no wall is hit
    }
    if (!World::CheckCollision(player.pos_x, nextY, player.scale, player.currentAngle)) {
        player.pos_y = nextY; // Apply Y movement if no wall is hit
    }

    // 5. Low HP Smoke Effect
    // If player HP is 50% or less, start smoking
    if (player_init.current_hp <= player_init.baseHp / 2 && player_init.current_hp > 0) {
        // 15% chance to spawn a puff every frame (prevents spawning too many stacked particles)
        if (AERandFloat() < 0.15f) {
            SpawnSmoke(player.pos_x, player.pos_y, player.scale);
        }
    }
}

// ~ Brief:	Calculates mouse position relative to the screen center and smoothly 
//          rotates the player's turret to face it.
void rotatePlayer(shape& player) {
    // 1. Get mouse cursor coordinates
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    // 2. Translate screen coordinates so (0,0) is perfectly in the center of the window
    float windowWidth = (float)AEGfxGetWindowWidth();
    float windowHeight = (float)AEGfxGetWindowHeight();
    float mouseRelativeX = (float)mouseX - windowWidth / 2.0f;
    float mouseRelativeY = windowHeight / 2.0f - (float)mouseY;

    // 3. Ignore tiny mouse jitters to keep the tank from vibrating
    if ((mouseRelativeX * mouseRelativeX) + (mouseRelativeY * mouseRelativeY) > GameConfig::MOUSE_JITTER_THRESHOLD) {

        // 4. Calculate the angle from the center of the screen to the mouse (subtracting half pi to align sprite "forward" with "up")
        float targetAngle = atan2f(mouseRelativeY, mouseRelativeX) - HALF_PI;

        // 5. Find the shortest rotational path to the target angle
        float angleDifference = targetAngle - player.currentAngle;
        while (angleDifference > PI) angleDifference -= TWO_PI;
        while (angleDifference < -PI) angleDifference += TWO_PI;

        // 6. Calculate the new angle, applying a 0.1f lerp (smoothing) factor
        float nextAngle = player.currentAngle + angleDifference * 0.1f;

        // 7. Check for collision at the new angle before applying it (prevents turret from clipping into walls)
        if (!World::CheckCollision(player.pos_x, player.pos_y, player.scale, nextAngle)) {
            player.currentAngle = nextAngle;
        }
    }
}

// ~ Brief:	Fetches inactive bullets from the object pool and calculates their 
//          spawn positions/vectors based on active weapon upgrades.
void SpawnBullet(shape& player) {
    // 1. Get fire rate stat and reset the cooldown timer
    bulletFireTimer = 0.0f;

    // 2. Determine how many bullets to shoot this frame
    int bulletsNeeded = dualback ? 2 : static_cast<int>(player.barrelCount);

    // 3. Calculate starting offsets to align bullets with the barrels perfectly
    float visualScale = player.scale / GameConfig::Tank::SCALE;
    float scaledgap = visualScale * GameConfig::Tank::BARREL_GAP;
    float totalWidth = (float)(player.barrelCount - 1) * scaledgap;
    float startOffset = -totalWidth / 2.0f;

    int bulletsFound = 0;

    // 4. Search the bullet pool for inactive bullets
    for (auto& boolet : bulletList) {
        if (!boolet.isActive) {
            boolet.isActive = true;

            // -- Calculate Multi-Barrel Spawn Position --
            float offsetLocalX = startOffset + (bulletsFound * scaledgap);
            float offsetLocalY = GameConfig::Tank::BARREL_LENGTH * visualScale; // Spawn at tip of barrel
            float currentDirX = 0.0f;
            float currentDirY = 0.0f;

            // Precalculate sin and cos of the tank's angle for efficiency
            float cosA = cosf(player.currentAngle);
            float sinA = sinf(player.currentAngle);

            // -- Handle Custom Weapon Fire Patterns --
            if (dualback) {
                offsetLocalX = 0.0f; // Keep it centered
                if (bulletsFound == 0) {
                    // Bullet 1: Forward
                    offsetLocalY = GameConfig::Tank::BARREL_LENGTH * visualScale;
                    currentDirX = -sinA;
                    currentDirY = cosA;
                }
                else {
                    // Bullet 2: Backward (Negative Y Offset & Flipped Direction vector)
                    offsetLocalY = -GameConfig::Tank::BARREL_LENGTH * visualScale;
                    currentDirX = sinA;
                    currentDirY = -cosA;
                }
            }
            else {
                // Normal Side-by-Side Math
                offsetLocalX = startOffset + (bulletsFound * scaledgap);
                offsetLocalY = GameConfig::Tank::BARREL_LENGTH * visualScale;
                currentDirX = -sinA;
                currentDirY = cosA;
            }

            // -- Apply 2D Coordinate Rotation --
            // Because Tank "Forward" is Angle + 90deg (Y-Axis), we adjust coordinate rotation:
            // Tank Right Vector (X axis) is (cos(a), sin(a))
            // Tank Forward Vector (Y axis) is (cos(a+90), sin(a+90)) -> (-sin(a), cos(a))
            float spawnOffsetX = (offsetLocalX * cosA) - (offsetLocalY * sinA);
            float spawnOffsetY = (offsetLocalX * sinA) + (offsetLocalY * cosA);

            // Finalize bullet start position in the world
            boolet.posX = player.pos_x + spawnOffsetX;
            boolet.posY = player.pos_y + spawnOffsetY;

            // Finalize bullet flight direction
            boolet.directionX = currentDirX;
            boolet.directionY = currentDirY;

            // Apply weapon stats
            boolet.speed = GameConfig::Bullet::BASE_SPEED;
            if (bigcannon) {
                boolet.size = GameConfig::Bullet::DEFAULT_SIZE * 2.0f; // Twice as big
                boolet.damagemul = 2.5f; // 2.5x damage
            }
            else {
                boolet.size = GameConfig::Bullet::DEFAULT_SIZE; // Normal size
                boolet.damagemul = 1.0f; // Normal damage
            }

            // Stop spawning if we have generated enough bullets for this shot
            bulletsFound++;
            if (bulletsFound >= bulletsNeeded) break;
        }
    }
}

// ~ Brief:	Handles input and state tracking to toggle the front-and-back 
//          shooting upgrade.
void DualBack(shape& player) {

    // LEGIT: Enable if card is present in passive deck
    if (upgradeFlag & UPGRADE_CANNON_180) {
        dualback = true;
        // Turn off other modes to prevent visual/math glitches
        player.barrelCount = 1;
        bigcannon = false;
        return;
    }
    else if (!cheatsOn) { dualback = false; }

    // CHEATS: Toggle front-and-back shooting mode when '6' is pressed
    if (cheatsOn && AEInputCheckTriggered(AEVK_6)) {
        dualback = !dualback;

        if (dualback) {
            // Turn off other modes to prevent visual/math glitches
            player.barrelCount = 1;
            bigcannon = false;
        }
    }
}

// ~ Brief:	Monitors left-click/spacebar input and fire rate cooldowns to 
//          trigger bullet spawning.
void ShootBullet(shape& player, float deltaTime) {
	//increment the fire timer by the time elapsed since last frame
    bulletFireTimer += deltaTime;
	// Check if the mouse button was released to prevent auto-firing when holding down the button
    if (!mousereleased) {
        if (!AEInputCheckCurr(AEVK_LBUTTON)) {
            mousereleased = true;
        }
        else {
            return;
        }
    }
	// If spacebar or left mouse button is currently held down, attempt to fire
    if (AEInputCheckCurr(AEVK_SPACE) || AEInputCheckCurr(AEVK_LBUTTON)) {
        float fire_rate = calculate_max_stats(3);

        // Fire only when the timer has reached or exceeded the fire_rate
        if (bulletFireTimer >= fire_rate) {
            SFX::playSFX(SFX_PLAYER_SHOOT);
            SpawnBullet(player);
        }
    }
}

// ~ Brief:	Calculates the continuous rotation and positional offsets for the 
//          orbital shield upgrade.
void updateOrbit(shape& player, float deltaTime) {
    // LEGIT Check
    if (upgradeFlag & UPGRADE_ORBIT) {
        orbitActive = true;
    }
    // If no card, and cheats are OFF, kill the orbit
    else if (!cheatsOn) {
        orbitActive = false;
    }

    // CHEAT Toggle
    if (cheatsOn && AEInputCheckTriggered(AEVK_7)) {
        orbitActive = !orbitActive;
    }

    // ACTIVE STATE MATH (Runs every frame if active)
    if (orbitActive) {
        float orbitSpeed = 4.0f;
        float orbitRadius = 180.0f * (player.scale / GameConfig::Tank::SCALE);

        orbitAngle += orbitSpeed * deltaTime;
        if (orbitAngle > TWO_PI) orbitAngle -= TWO_PI;

        // Calculate world position based on player center
        orbitPosX = player.pos_x + cosf(orbitAngle) * orbitRadius;
        orbitPosY = player.pos_y + sinf(orbitAngle) * orbitRadius;
    }
}
// ~ Brief:	Draws the orbiting shield sprite at its calculated position
void DrawOrbit(float playerScale) {
    if (!orbitActive || pOrbitTex == nullptr || pBulletMesh == nullptr) {
        return;
    }

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pOrbitTex, 0, 0);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    float orbitSize = 100.0f * (playerScale / GameConfig::Tank::SCALE);

    Gfx::printMesh(pBulletMesh, { orbitPosX, orbitPosY }, { orbitSize, orbitSize }, orbitAngle, { 0.f, 0.f }, true);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
}

// ~ Brief:	Handles input and state tracking to toggle the Big Cannon upgrade.
void drawBigCannon(shape& player) {
    // LEGIT: Enable Big Cannon weapon mode when card is present in passive desk
    if (upgradeFlag & UPGRADE_BIG_CANNON) {
        bigcannon = true;
        // Turn Big Cannon ON and force off dual barrels to prevent stat clashing
        player.barrelCount = 1;
        return;
    } // If no card, and cheats are OFF, reset to normal
    else if (!cheatsOn) {
        bigcannon = false;
    }

    // CHEATS: Toggle Big Cannon weapon mode when '5' is pressed
    if (cheatsOn && AEInputCheckTriggered(AEVK_5)) {
        bigcannon = !bigcannon;

        if (bigcannon) {
            // Turn Big Cannon ON and force off dual barrels to prevent stat clashing
            player.barrelCount = 1;
        }
        else {
            // Turn Big Cannon OFF
        }
    }
}

// ------BULLET & PARTICLE UPDATES------------------------------------------ //

// ~ Brief:	Updates movement physics, handles wall/border collisions, and 
//          despawns out-of-bounds player and enemy bullets.
void updateBullets(shape& player, float deltaTime) {
    // --- 1. UPDATE PLAYER BULLETS ---
    for (auto& boolet : bulletList) {
        if (boolet.isActive) {
            // Move the bullet along its vector
            boolet.posX += boolet.directionX * boolet.speed * deltaTime;
            boolet.posY += boolet.directionY * boolet.speed * deltaTime;

            // Check for the leading edge of the bullet for more accurate wall collision
            float edgeX = boolet.posX + (boolet.size * boolet.directionX);
            float edgeY = boolet.posY + (boolet.size * boolet.directionY);

            // Bullet to environment border collision check
            if (World::isPointColliding(edgeX, edgeY)) {
                TriggerBulletImpact(boolet.posX, boolet.posY, boolet.directionX, boolet.directionY);
                boolet.isActive = false;
                continue;
            }

            // Despawn bullet if it flies too far away from the player to save memory
            float diffX = boolet.posX - player.pos_x;
            float diffY = boolet.posY - player.pos_y;
            if ((diffX * diffX + diffY * diffY) > GameConfig::DESPAWN_DISTANCE_SQ) {
                boolet.isActive = false;
            }
        }
    }

    // --- 2. UPDATE ENEMY BULLETS ---
    for (auto& eBullet : enemyBulletList) {
        if (eBullet.isActive) {
            // Move the enemy bullet
            eBullet.posX += eBullet.directionX * eBullet.speed * deltaTime;
            eBullet.posY += eBullet.directionY * eBullet.speed * deltaTime;

            // Despawn if they get too far from the player
            float diffX = eBullet.posX - player.pos_x;
            float diffY = eBullet.posY - player.pos_y;
            if ((diffX * diffX + diffY * diffY) > GameConfig::DESPAWN_DISTANCE_SQ) {
                eBullet.isActive = false;
            }
        }
    }
}

// ~ Brief:	Fetches an inactive smoke particle from the pool and initializes 
//          it with a randomized position offset and size.
void SpawnSmoke(float x, float y, float baseSize) {
    // Find the first inactive smoke particle in the memory pool
    for (auto& s : smokes) {
        if (!s.isActive) {
            s.isActive = true;

            // Spawn with a slight random offset so multiple smokes don't perfectly stack on top of each other
            s.posX = x + (AERandFloat() * 20.0f - 10.0f);
            s.posY = y + (AERandFloat() * 20.0f - 10.0f);

            // Randomize size, scaling it roughly to the size of the object that spawned it
            s.size = (baseSize * 0.4f) + (AERandFloat() * 10.0f);
            break;
        }
    }
}

// ~ Brief:	Updates physics, applies wind drift, and shrinks active smoke 
//          particles over time until they despawn.
void updateSmoke(float deltaTime) {
    // Process physics for all active smoke clouds
    for (auto& s : smokes) {
        if (s.isActive) {
            s.posY += 60.0f * deltaTime; // Float steadily upwards
            s.posX += (AERandFloat() * 20.0f - 10.0f) * deltaTime; // Jitter slightly left/right like blowing wind
            s.size -= 15.0f * deltaTime; // Shrink slowly over time

            // If it shrinks away to nothing, kill it so it can be recycled by the pool
            if (s.size <= 0) {
                s.isActive = false;
            }
        }
    }
}

// ~ Brief:	Unloads bullet textures and frees mesh memory to prevent memory leaks.
void FreeBullets() {
    if (pBulletTex) { AEGfxTextureUnload(pBulletTex);  pBulletTex = nullptr; }
    if (pOrbitTex) { AEGfxTextureUnload(pOrbitTex);   pOrbitTex = nullptr; }
    if (pBulletMesh) { AEGfxMeshFree(pBulletMesh);      pBulletMesh = nullptr; }
}