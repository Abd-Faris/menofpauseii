#include "MasterHeader.h"

// --- NEW FUNCTION: DRAW MULTIPLE BARRELS --
namespace {  }


BulletObj bulletList[GameConfig::MAX_BULLETS_COUNT];
float bulletFireTimer = 0.0f;
bool bigcannon = false;
bool dualback = false;

void DrawMultiBarrels(int count, float gap, float pivotOffset, float tankRot, float tankX, float tankY, float barrelWidth, float barrelLength, AEGfxVertexList* MeshRect) {
    if (count <= 0) return;

    // 1. Calculate the starting X offset to keep the barrels centered on the turret
    float formationWidth = (float)(count - 1) * gap;
    float startX = -formationWidth / 2.0f;

    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);

    for (int i = 0; i < count; i++) {
        // 2. Calculate local position relative to the tank's center
        // Spread them along the X-axis and push them forward along Y
        float currentLocalX = startX + (i * gap);
        AEVec2 barrelOffset = { currentLocalX, pivotOffset };

        // 3. Define the size of each barrel
        AEVec2 barrelSize = { barrelWidth, barrelLength };

        // 4. Use the generalized printMesh to handle the TRS math
        Gfx::printMesh(
            MeshRect,            // The barrel shape
            { tankX, tankY },    // Tank Position (World Coords)
            barrelSize,          // Scale
            tankRot,             // Rotation (Radians)
            barrelOffset         // Local Offset (Position relative to tank center)
        );
    }
}

void drawBigTank(shape &player) {
    if (AEInputCheckTriggered(AEVK_7)) {
        if (player.barrelCount == 1) {
            // Turn Dual ON
            player.barrelCount = 2;
            bigcannon = false; // Force turn off big cannon to prevent overlapping states
            player.scale = GameConfig::Tank::SCALE * 2.0f; // Set exact scale
        }
        else {
            // Turn Dual OFF
            player.barrelCount = 1;
            player.scale = GameConfig::Tank::SCALE; // Back to normal
        }
    }
}

void movePlayer(shape &player, float deltaTime) {
    float playerSpeed = calculate_max_stats(2);
    if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP)) player.pos_y += playerSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN)) player.pos_y -= playerSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT)) player.pos_x -= playerSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT)) player.pos_x += playerSpeed * deltaTime;
}

void rotatePlayer(shape &player) {
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);
    float windowWidth = (float)AEGfxGetWindowWidth();
    float windowHeight = (float)AEGfxGetWindowHeight();
    float mouseRelativeX = (float)mouseX - windowWidth / 2.0f;
    float mouseRelativeY = windowHeight / 2.0f - (float)mouseY;

    if ((mouseRelativeX * mouseRelativeX) + (mouseRelativeY * mouseRelativeY) > GameConfig::MOUSE_JITTER_THRESHOLD) {
        float targetAngle = atan2f(mouseRelativeY, mouseRelativeX) - HALF_PI;
        float angleDifference = targetAngle - player.currentAngle;
        while (angleDifference > PI) angleDifference -= TWO_PI;
        while (angleDifference < -PI) angleDifference += TWO_PI;
        player.currentAngle += angleDifference * 0.1f;
    }
}

void SpawnBullet(shape& player, float deltaTime) {
    float fire_rate = calculate_max_stats(3);
    bulletFireTimer = fire_rate;

    // We need to find 'barrelCount' inactive bullets
    int bulletsNeeded = player.barrelCount;

    // Calculate Start Offset for bullets (Same math as drawing)
    float visualScale = player.scale / GameConfig::Tank::SCALE;

    float scaledgap = visualScale * GameConfig::Tank::BARREL_GAP;
    float totalWidth = (float)(player.barrelCount - 1) * scaledgap;
    float startOffset = -totalWidth / 2.0f;

    int bulletsFound = 0;
    for (auto& boolet : bulletList) {
        if (!boolet.isActive) {
            boolet.isActive = true;

            // -- Calculate Multi-Barrel Spawn Position --
            float offsetLocalX = startOffset + (bulletsFound * scaledgap);
            float offsetLocalY = GameConfig::Tank::BARREL_LENGTH * visualScale; // Spawn at tip

            // Rotate Local Offset by Tank Angle
            // Formula: x' = x*cos - y*sin, y' = x*sin + y*cos
            // Note: AE Engine Rotation is usually Standard Math Rotation
            float cosA = cosf(player.currentAngle);
            float sinA = sinf(player.currentAngle);

            if (dualback) {
                offsetLocalX = 0.0f; // Keep it centered
                if (bulletsFound == 0) {
                    // Bullet 1: Forward
                    offsetLocalY = GameConfig::Tank::BARREL_LENGTH * visualScale;
                    currentDirX = -sinA;
                    currentDirY = cosA;
                }
                else {
                    // Bullet 2: Backward (Negative Length & Flipped Direction)
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

            // Because Tank "Forward" is Angle + 90deg (Y-Axis), we adjust coordinate rotation:
            // Tank Right Vector (X axis) is (cos(a), sin(a))
            // Tank Forward Vector (Y axis) is (cos(a+90), sin(a+90)) -> (-sin(a), cos(a))

            float spawnOffsetX = (offsetLocalX * cosA) - (offsetLocalY * sinA);
            float spawnOffsetY = (offsetLocalX * sinA) + (offsetLocalY * cosA);

            boolet.posX = player.pos_x + spawnOffsetX;
            boolet.posY = player.pos_y + spawnOffsetY;

            // Direction is same for all parallel barrels
            boolet.directionX = -sinA; // Forward Vector X
            boolet.directionY = cosA;  // Forward Vector Y

            boolet.speed = GameConfig::Bullet::BASE_SPEED;
            boolet.size = GameConfig::Bullet::DEFAULT_SIZE;
            if (bigcannon) {
                boolet.size = GameConfig::Bullet::DEFAULT_SIZE * 2.0f; // Twice as big
                boolet.damagemul = 2.5f; // 2.5x damage
            }
            else {
                boolet.size = GameConfig::Bullet::DEFAULT_SIZE; // Normal size
                boolet.damagemul = 1.0f; // Normal damage
            }
            bulletsFound++;
            if (bulletsFound >= bulletsNeeded) break;
        }
    }
}
void DualBack(shape& player) {
    if (AEInputCheckTriggered(AEVK_8)) {
        dualback = !dualback;

        if (dualback) {
            // Turn off other modes to prevent visual/math glitches
            player.barrelCount = 1;
            bigcannon = false;
            player.scale = GameConfig::Tank::SCALE;
        }
    }
}

void ShootBullet(shape &player, float deltaTime) {
    if (AEInputCheckCurr(AEVK_SPACE) || AEInputCheckCurr(AEVK_LBUTTON)) {
        bulletFireTimer -= deltaTime;

        if (bulletFireTimer <= 0) {
            SpawnBullet(player, deltaTime);
        }
    }
}

void drawBigCannon(shape& player) {
    if (AEInputCheckTriggered(AEVK_U)) {
        bigcannon = !bigcannon;

        if (bigcannon) {
            // Turn Big Cannon ON
            player.barrelCount = 1; // Force turn off dual barrels
            player.scale = GameConfig::Tank::SCALE * 2.0f; // Set exact scale
        }
        else {
            // Turn Big Cannon OFF
            player.scale = GameConfig::Tank::SCALE; // Back to normal
        }
    }
}

void updateBullets(shape& player, float deltaTime) {
    for (auto& boolet : bulletList) {
        if (boolet.isActive) {
            boolet.posX += boolet.directionX * boolet.speed * deltaTime;
            boolet.posY += boolet.directionY * boolet.speed * deltaTime;

            float diffX = boolet.posX - player.pos_x;
            float diffY = boolet.posY - player.pos_y;
            if ((diffX * diffX + diffY * diffY) > GameConfig::DESPAWN_DISTANCE_SQ) {
                boolet.isActive = false;
            }
        }
    }
}