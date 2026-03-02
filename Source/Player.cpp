#include "MasterHeader.h"

// --- NEW FUNCTION: DRAW MULTIPLE BARRELS --
namespace {  }


BulletObj bulletList[GameConfig::MAX_BULLETS_COUNT];
float bulletFireTimer = 0.0f;
bool bigcannon = false;
bool dualback = false;
bool orbitActive = false;
float orbitAngle = 0.0f;
float orbitPosX = 0.0f;
float orbitPosY = 0.0f;
SmokeParticle smokes[100];

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
            //player.scale = GameConfig::Tank::SCALE * 2.0f; // Set exact scale
        }
        else {
            // Turn Dual OFF
            player.barrelCount = 1;
            //player.scale = GameConfig::Tank::SCALE; // Back to normal
        }
    }
}

void movePlayer(shape& player, float deltaTime) {
    float playerSpeed = calculate_max_stats(2);
    float moveAmount = playerSpeed * deltaTime;

    float nextX = player.pos_x;
    float nextY = player.pos_y;

    if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP))    nextY += moveAmount;
    if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN))  nextY -= moveAmount;
    if (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT))  nextX -= moveAmount;
    if (AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT)) nextX += moveAmount;

	//collision checks for the new position before applying it
    if (!World::CheckCollision(nextX, player.pos_y, player.scale, player.currentAngle)) {
        player.pos_x = nextX;
    }
    if (!World::CheckCollision(player.pos_x, nextY, player.scale, player.currentAngle)) {
        player.pos_y = nextY;
    }
    if (player_init.current_hp <= player_init.baseHp / 2 && player_init.current_hp > 0) {
        // Give it a 15% chance to spawn a smoke puff every frame (prevents spawning too many)
        if (AERandFloat() < 0.15f) {
            SpawnSmoke(player.pos_x, player.pos_y, player.scale);
        }
    }
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
        float nextAngle = player.currentAngle + angleDifference * 0.1f;

		//check for collision at the new angle before applying it
		if (!World::CheckCollision(player.pos_x, player.pos_y, player.scale, nextAngle)) {
            player.currentAngle = nextAngle;
        }
    }
}

void SpawnBullet(shape& player, float deltaTime) {
    float fire_rate = calculate_max_stats(3);
    bulletFireTimer = fire_rate;

    // We need to find 'barrelCount' inactive bullets
    int bulletsNeeded = dualback ? 2 : player.barrelCount; player.barrelCount;

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
            float currentDirX = 0.0f;
            float currentDirY = 0.0f;

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
            boolet.directionX = currentDirX; // Forward Vector X
            boolet.directionY = currentDirY;  // Forward Vector Y

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
           // player.scale = GameConfig::Tank::SCALE;
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

void updateOrbit(shape& player, float deltaTime) {
    if (AEInputCheckTriggered(AEVK_C)) {
        orbitActive = !orbitActive;
    }

    if (orbitActive) {
        float orbitSpeed = 4.0f; // Spin speed
        float orbitRadius = 150.0f * (player.scale / GameConfig::Tank::SCALE); // Distance from tank

        orbitAngle += orbitSpeed * deltaTime;
        if (orbitAngle > TWO_PI) orbitAngle -= TWO_PI;

        orbitPosX = player.pos_x + cosf(orbitAngle) * orbitRadius;
        orbitPosY = player.pos_y + sinf(orbitAngle) * orbitRadius;
    }
}

void drawBigCannon(shape& player) {
    if (AEInputCheckTriggered(AEVK_U)) {
        bigcannon = !bigcannon;

        if (bigcannon) {
            // Turn Big Cannon ON
            player.barrelCount = 1; // Force turn off dual barrels
            //player.scale = GameConfig::Tank::SCALE * 2.0f; // Set exact scale
        }
        else {
            // Turn Big Cannon OFF
           // player.scale = GameConfig::Tank::SCALE; // Back to normal
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
    for (auto& eBullet : enemyBulletList) {
        if (eBullet.isActive) {
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

void SpawnSmoke(float x, float y, float baseSize) {
    for (auto& s : smokes) {
        if (!s.isActive) {
            s.isActive = true;
            // Spawn with a slight random offset
            s.posX = x + (AERandFloat() * 20.0f - 10.0f);
            s.posY = y + (AERandFloat() * 20.0f - 10.0f);
            s.size = (baseSize * 0.4f) + (AERandFloat() * 10.0f); // Size based on what is attached to
            break;
        }
    }
}

void updateSmoke(float deltaTime) {
    for (auto& s : smokes) {
        if (s.isActive) {
            s.posY += 60.0f * deltaTime; // float upwards
            s.posX += (AERandFloat() * 20.0f - 10.0f) * deltaTime; // goes slightly left/right
            s.size -= 15.0f * deltaTime; // shrink over time

            // if it shrinks away to nothing, despawn it
            if (s.size <= 0) {
                s.isActive = false;
            }
        }
    }
}