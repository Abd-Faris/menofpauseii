#include "MasterHeader.h"
#include "AEEngine.h"
#include "AEGraphics.h"
#include "AEMath.h"
#include <cmath>
#include <array>
#include <vector>

// ===========================================================================
// GLOBALS & CONFIGURATION
// ===========================================================================
namespace {
    // -- Assets --
    // We hold onto these pointers so we can clean them up in FreeGame()
    s8 boldPixelsFont;
    AEGfxVertexList* MeshRect = nullptr;
    AEGfxVertexList* MeshCircle = nullptr;

    // -- Player State --
    // Tracks position, scale, and rotation (angle in radians)
    shape player = { 50.0f, 0.0f, 0.0f, 0.0f };

    // -- Bullet System (Object Pool) --
    // We pre-allocate 1000 bullets so we don't lag creating them during gameplay.
    struct Bullet {
        float posX, posY;
        float directionX, directionY; // Normalized vector for direction
        float speed;
        float size;     // Allows us to have different sized bullets later
        bool isActive;  // If false, the game ignores this bullet
    };

    const int MAX_BULLETS = 1000;
    Bullet bulletList[MAX_BULLETS];
    float bulletFireTimer = 0.0f; // Cooldown timer for shooting

    // -- Enemy System --
    // Similar to bullets, we pool enemies to avoid memory allocation lag.
    enum { MAX_ENEMIES = 50 };
    std::array<Enemies, MAX_ENEMIES> enemyPool;
    f64 enemySpawnTimer = 0;

    // -- Visual Settings --
    // Tweaking these changes how the Tank looks without changing logic
    const float TANK_BARREL_LENGTH = 150.0f;
    const float TANK_BARREL_WIDTH = 30.0f;
}

// ===========================================================================
// HELPER FUNCTIONS
// These handle the boring logic to keep the main loop clean.
// ===========================================================================

// Moves an enemy off-screen and marks it as dead so it can be reused later.
void ResetEnemy(Enemies* enemyToReset) {
    enemyToReset->alive = false;
    enemyToReset->pos.x = 10000.0f; // Yeet them far away
    enemyToReset->pos.y = 10000.0f;
    enemyToReset->scale = 0;
    enemyToReset->hp = 0;
}

// Finds the first "dead" enemy in the pool and brings them back to life.
void SpawnOneEnemy(bool isBigEnemy) {
    for (auto& newEnemy : enemyPool) {
        // We only want to recycle an enemy that isn't currently being used
        if (!newEnemy.alive) {
            float windowWidth = (float)AEGfxGetWindowWidth();
            float windowHeight = (float)AEGfxGetWindowHeight();

            // Pick a random spot on the screen
            float spawnX = (AERandFloat() * windowWidth) - windowWidth / 2.0f;
            float spawnY = (AERandFloat() * windowHeight) - windowHeight / 2.0f;

            // Don't let them spawn right on top of the player (it feels unfair)
            float differenceX = spawnX - player.pos_x;
            float differenceY = spawnY - player.pos_y;
            float distanceToPlayer = sqrt((differenceX * differenceX) + (differenceY * differenceY));

            // If too close, shove them 400 pixels away
            if (distanceToPlayer < 300.0f) {
                spawnX += 400.0f;
            }

            // Initialize stats
            newEnemy.pos = { spawnX, spawnY };
            newEnemy.velocity = { 0, 0 };
            newEnemy.alive = true;
            newEnemy.rotation = AERandFloat() * 360.0f;

            // Big enemies are tankier (100hp) but same speed for now
            newEnemy.scale = isBigEnemy ? 67.0f : 30.0f;
            newEnemy.hp = isBigEnemy ? 100 : 15;

            // Found one, so stop looping
            break;
        }
    }
}

// ===========================================================================
// COLLISION LOGIC
// ===========================================================================
void circlerectcollision() {
    // Loop through every living enemy...
    for (auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive) continue;

        // ...and check it against every active bullet
        for (auto& boolet : bulletList) {
            if (!boolet.isActive) continue;

            // Get distance between bullet center and enemy center
            float differenceX = boolet.posX - currentEnemy.pos.x;
            float differenceY = boolet.posY - currentEnemy.pos.y;

            // Use Distance Squared (A^2 + B^2) to avoid the slow square root calculation
            float distanceSquared = (differenceX * differenceX) + (differenceY * differenceY);

            // HITBOX MATH:
            // Enemies are squares, but we check collision as circles because it's easier.
            // 0.6f is the "Goldilocks" number: 0.5 misses corners, 0.7 hits empty air.
            float collisionRadius = (currentEnemy.scale * 0.6f) + boolet.size;

            // If closer than radius, it's a hit
            if (distanceSquared < (collisionRadius * collisionRadius)) {
                currentEnemy.hp -= 15;    // Ouch
                boolet.isActive = false;  // Bullet disappears on impact
            }
        }
    }
}

// ===========================================================================
// LOAD GAME
// Initializes assets and resets the game state (so restarting works).
// ===========================================================================
void LoadGame() {
    // 1. Load the font
    boldPixelsFont = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);

    // 2. Build the Mesh for Circles (Player body, bullets)
    AEGfxMeshStart();
    int slices = 40; // Higher number = smoother circle
    float angleStep = 2.0f * 3.1415f / slices;
    for (int i = 0; i < slices; i++) {
        AEGfxTriAdd(
            0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,
            cosf(i * angleStep), sinf(i * angleStep), 0xFFFFFFFF, 1.0f, 1.0f,
            cosf((i + 1) * angleStep), sinf((i + 1) * angleStep), 0xFFFFFFFF, 0.0f, 0.0f);
    }
    MeshCircle = AEGfxMeshEnd();

    // 3. Build the Mesh for Rectangles (Tank tracks, barrel, enemies)
    AEGfxMeshStart();
    // Two triangles make a square
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    MeshRect = AEGfxMeshEnd();

    // 4. Reset Variables
    // Important: We must reset these manually, or reloading the level will be broken
    player.pos_x = 0;
    player.pos_y = 0;
    player.currentAngle = 0;
    player.scale = 50.0f;

    bulletFireTimer = 0;
    enemySpawnTimer = 0;

    // Clear the object pools
    for (int i = 0; i < MAX_BULLETS; i++) bulletList[i].isActive = false;
    for (int i = 0; i < MAX_ENEMIES; i++) ResetEnemy(&enemyPool[i]);

    // Spawn the first wave immediately
    for (int i = 0; i < 5; i++) SpawnOneEnemy(false);
}

// ===========================================================================
// DRAW GAME
// This is the Game Loop. It runs Inputs -> Logic -> Physics -> Rendering.
// ===========================================================================
void DrawGame() {
    float deltaTime = (float)AEFrameRateControllerGetFrameTime();

    // -----------------------------------------------------------------------
    // PART A: LOGIC & PHYSICS
    // -----------------------------------------------------------------------

    // 1. Move Player (Standard WASD)
    float playerSpeed = 500.0f;
    if (AEInputCheckCurr(AEVK_W)) player.pos_y += playerSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_S)) player.pos_y -= playerSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_A)) player.pos_x -= playerSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_D)) player.pos_x += playerSpeed * deltaTime;

    // 2. Rotate Player (Aim at Mouse)
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    // Convert screen coordinates (Top-Left 0,0) to World Coordinates (Center 0,0)
    float windowWidth = (float)AEGfxGetWindowWidth();
    float windowHeight = (float)AEGfxGetWindowHeight();
    float mouseRelativeX = (float)mouseX - windowWidth / 2.0f;
    float mouseRelativeY = windowHeight / 2.0f - (float)mouseY;

    // Logic: Only rotate if the mouse is slightly away from center (prevents jittering)
    if ((mouseRelativeX * mouseRelativeX) + (mouseRelativeY * mouseRelativeY) > 1.0f) {
        float targetAngle = atan2f(mouseRelativeY, mouseRelativeX) - 1.5708f; // -90 deg offset because art points Up
        float angleDifference = targetAngle - player.currentAngle;

        // Math magic to make sure we turn the shortest direction (left vs right)
        while (angleDifference > 3.1415f) angleDifference -= 6.2831f;
        while (angleDifference < -3.1415f) angleDifference += 6.2831f;

        // 0.1f creates a smooth turning delay (lag)
        player.currentAngle += angleDifference * 0.1f;
    }

    // 3. Shooting
    if (AEInputCheckCurr(AEVK_SPACE)) {
        bulletFireTimer -= deltaTime;

        // Only fire if cooldown is finished
        if (bulletFireTimer <= 0) {
            // Find the first "sleeping" bullet in our list and wake it up
            for (auto& boolet : bulletList) {
                if (!boolet.isActive) {
                    boolet.isActive = true;
                    // Calculate vector based on player angle (+90 deg offset)
                    boolet.directionX = cosf(player.currentAngle + 1.5708f);
                    boolet.directionY = sinf(player.currentAngle + 1.5708f);

                    // Spawn it at the tip of the barrel, not inside the tank
                    boolet.posX = player.pos_x + boolet.directionX * TANK_BARREL_LENGTH;
                    boolet.posY = player.pos_y + boolet.directionY * TANK_BARREL_LENGTH;
                    boolet.speed = 800.0f;
                    boolet.size = 15.0f;

                    bulletFireTimer = 0.15f; // Set cooldown
                    break; // We fired one, so stop looking
                }
            }
        }
    }

    // 4. Move Bullets
    for (auto& boolet : bulletList) {
        if (boolet.isActive) {
            boolet.posX += boolet.directionX * boolet.speed * deltaTime;
            boolet.posY += boolet.directionY * boolet.speed * deltaTime;

            // Clean up bullets that fly too far off screen
            float diffX = boolet.posX - player.pos_x;
            float diffY = boolet.posY - player.pos_y;
            // 2500^2 is a massive range, basically keeps them until well off screen
            if ((diffX * diffX + diffY * diffY) > 2500 * 2500) {
                boolet.isActive = false;
            }
        }
    }

    // 5. Spawn Enemies periodically
    enemySpawnTimer += deltaTime;
    if (enemySpawnTimer >= 3.0) {
        // Roll dice: 40% chance for a big enemy
        bool spawnBig = (AERandFloat() * 10.0f) < 4.0f;
        SpawnOneEnemy(spawnBig);
        enemySpawnTimer = 0;
    }

    // 6. Enemy Physics (The "Boids" Logic)
    for (auto& currentEnemy : enemyPool) {
        if (!currentEnemy.alive) continue;

        // -- Separation Force --
        // This loop ensures enemies push away from each other so they don't stack
        AEVec2 separationForce = { 0,0 };
        for (auto& otherEnemy : enemyPool) {
            if (&currentEnemy == &otherEnemy || !otherEnemy.alive) continue;

            float diffX = currentEnemy.pos.x - otherEnemy.pos.x;
            float diffY = currentEnemy.pos.y - otherEnemy.pos.y;
            float distance = sqrt(diffX * diffX + diffY * diffY);

            float minDistance = (currentEnemy.scale + otherEnemy.scale) * 0.6f;

            // If they are touching...
            if (distance < minDistance && distance > 0.1f) {
                // Calculate how hard to push (closer = harder push)
                float pushStrength = (minDistance - distance) / minDistance * 500.0f;
                separationForce.x += (diffX / distance) * pushStrength;
                separationForce.y += (diffY / distance) * pushStrength;
            }
        }

        // Apply the separation push
        currentEnemy.velocity.x += separationForce.x * deltaTime;
        currentEnemy.velocity.y += separationForce.y * deltaTime;

        // Apply friction so they don't slide forever like on ice
        currentEnemy.velocity.x *= 0.92f;
        currentEnemy.velocity.y *= 0.92f;

        // Actually move them
        currentEnemy.pos.x += currentEnemy.velocity.x * deltaTime;
        currentEnemy.pos.y += currentEnemy.velocity.y * deltaTime;

        // -- Death Animation --
        // If HP is 0, shrink them until they vanish
        if (currentEnemy.hp <= 0) {
            currentEnemy.scale -= 100 * deltaTime;
            if (currentEnemy.scale <= 0) {
                ResetEnemy(&currentEnemy);
            }
        }
    }

    // 7. Check hits
    circlerectcollision();


    // -----------------------------------------------------------------------
    // PART B: RENDERING
    // -----------------------------------------------------------------------

    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f); // Dark Grey Ground
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetCamPosition(player.pos_x, player.pos_y); // Camera follows player

    // -- Draw Bullets --
    AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
    for (const auto& boolet : bulletList) {
        if (boolet.isActive) {
            AEMtx33 scaleMatrix, transMatrix, finalMatrix;

            AEMtx33Scale(&scaleMatrix, boolet.size, boolet.size);
            AEMtx33Trans(&transMatrix, boolet.posX, boolet.posY);
            AEMtx33Concat(&finalMatrix, &transMatrix, &scaleMatrix);

            AEGfxSetTransform(finalMatrix.m);
            AEGfxMeshDraw(MeshCircle, AE_GFX_MDM_TRIANGLES);
        }
    }

    // -- Draw Player Tank (Composite Drawing) --
    // We are building the tank out of 4 separate shapes.
    // They all share the same Rotation and Position matrices so they move together.
    AEMtx33 scaleMatrix, rotationMatrix, transMatrix, offsetMatrix, finalMatrix;

    AEMtx33Rot(&rotationMatrix, player.currentAngle);
    AEMtx33Trans(&transMatrix, player.pos_x, player.pos_y);

    // 1. Draw Tracks (Black/Grey)
    AEGfxSetColorToMultiply(0.1f, 0.1f, 0.1f, 1.0f);

    // Left Track (Offset -35)
    AEMtx33Scale(&scaleMatrix, 20.0f, 90.0f);
    AEMtx33Trans(&offsetMatrix, -35.0f, 0.0f);
    AEMtx33Concat(&finalMatrix, &offsetMatrix, &scaleMatrix);
    AEMtx33Concat(&finalMatrix, &rotationMatrix, &finalMatrix);
    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
    AEGfxSetTransform(finalMatrix.m);
    AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);

    // Right Track (Offset +35)
    AEMtx33Scale(&scaleMatrix, 20.0f, 90.0f);
    AEMtx33Trans(&offsetMatrix, 35.0f, 0.0f);
    AEMtx33Concat(&finalMatrix, &offsetMatrix, &scaleMatrix);
    AEMtx33Concat(&finalMatrix, &rotationMatrix, &finalMatrix);
    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
    AEGfxSetTransform(finalMatrix.m);
    AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);

    // 2. Draw Main Body (Green Box)
    AEGfxSetColorToMultiply(0.2f, 0.6f, 0.2f, 1.0f);
    AEMtx33Scale(&scaleMatrix, 60.0f, 80.0f);
    AEMtx33Concat(&finalMatrix, &rotationMatrix, &scaleMatrix);
    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
    AEGfxSetTransform(finalMatrix.m);
    AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);

    // 3. Draw Barrel (Gun)
    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);
    AEMtx33Scale(&scaleMatrix, TANK_BARREL_WIDTH, TANK_BARREL_LENGTH);
    // Offset forward so it pivots at the base, not the center of the barrel
    AEMtx33Trans(&offsetMatrix, 0.0f, TANK_BARREL_LENGTH / 2.0f);
    AEMtx33Concat(&finalMatrix, &offsetMatrix, &scaleMatrix);
    AEMtx33Concat(&finalMatrix, &rotationMatrix, &finalMatrix);
    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
    AEGfxSetTransform(finalMatrix.m);
    AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);

    // 4. Draw Turret (Circle on top)
    AEGfxSetColorToMultiply(0.3f, 0.7f, 0.3f, 1.0f);
    AEMtx33Scale(&scaleMatrix, 40.0f, 40.0f);
    AEMtx33Concat(&finalMatrix, &rotationMatrix, &scaleMatrix);
    AEMtx33Concat(&finalMatrix, &transMatrix, &finalMatrix);
    AEGfxSetTransform(finalMatrix.m);
    AEGfxMeshDraw(MeshCircle, AE_GFX_MDM_TRIANGLES);

    // -- Draw Enemies --
    for (const auto& currentEnemy : enemyPool) {
        // Draw if they are alive OR if they are currently dying (scale > 0)
        if (currentEnemy.alive || currentEnemy.scale > 0) {

            // Big enemies are Blue, small are Red
            if (currentEnemy.scale > 50) AEGfxSetColorToMultiply(0.2f, 0.2f, 0.8f, 1);
            else AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1);

            AEMtx33Scale(&scaleMatrix, currentEnemy.scale, currentEnemy.scale);
            AEMtx33RotDeg(&rotationMatrix, currentEnemy.rotation);
            AEMtx33Trans(&transMatrix, currentEnemy.pos.x, currentEnemy.pos.y);

            AEMtx33Concat(&finalMatrix, &transMatrix, &rotationMatrix);
            AEMtx33Concat(&finalMatrix, &finalMatrix, &scaleMatrix);

            AEGfxSetTransform(finalMatrix.m);
            AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
        }
    }
}

// ===========================================================================
// CLEANUP
// ===========================================================================
void FreeGame() {
    AEGfxDestroyFont(boldPixelsFont);
    // Always check if pointer is valid before freeing to prevent crashes
    if (MeshRect) AEGfxMeshFree(MeshRect);
    if (MeshCircle) AEGfxMeshFree(MeshCircle);
}