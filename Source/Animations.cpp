#include "MasterHeader.h"
#include "Animations.h"

// ===========================================================================
// ANIMATION & PARTICLE SYSTEM
// ===========================================================================

namespace {
	
	// --- EXPLOSION CONFIG ---
    const int MAX_EXPLOSIONS = 50;
    const int TOTAL_FRAMES = 5;
    const float FRAME_TIME = 0.08f;

	//explosion pool and assets
    Explosion explosionPool[MAX_EXPLOSIONS];
    AEGfxTexture* pExplosionSheet = nullptr;
    AEGfxVertexList* pAnimMesh = nullptr;

	//UV steps for sprite sheet (5 frames in a row, 1 row total)
    const float UV_X_STEP = 1.0f / 5.0f;
    const float UV_Y_STEP = 1.0f;

    // --- BULLET CONFIG ---
    const int MAX_SPARKS = 2000;
    BulletSpark sparkPool[MAX_SPARKS];
}

// --- LOAD FUNCTION: Load explosion sprite sheet and create mesh for animation frames ---
void Animations_Load() {
    pExplosionSheet = AEGfxTextureLoad("Assets/explosions.png");

	//get frame width for UV mapping
    float frameWidth = 1.0f / 5.0f;

    AEGfxMeshStart();
  
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, frameWidth, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(
        0.5f, -0.5f, 0xFFFFFFFF, frameWidth, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, frameWidth, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    pAnimMesh = AEGfxMeshEnd();

	//set all explosions to inactive at the start
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        explosionPool[i].active = false;
    }
    //set all sparks to inactive at the start
    for (int i = 0; i < MAX_SPARKS; i++) {
        sparkPool[i].isActive = false;
    }
}

// --- TRIGGER FUNCTION: Activate an explosion at the given position ---
void TriggerExplosion(float x, float y, float size) {
	//find the first inactive explosion in the pool and activate it with the given position
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionPool[i].active == false) {
            //initialise x pos
            explosionPool[i].pos.x = x;
			//initialise y pos
            explosionPool[i].pos.y = y;
			//initialise size
            explosionPool[i].scale = size;
			//reset timer and frame to start animation from the beginning
            explosionPool[i].timer = 0.0f;
            explosionPool[i].currentFrame = 0;
			//activate explosion
            explosionPool[i].active = true;
            return;
        }
    }
}

void TriggerBulletImpact(float x, float y, float bulletDirX, float bulletDirY) {
    int sparksToSpawn = 20;
    int count = 0;

    float baseAngle = atan2f(bulletDirY, bulletDirX);

    for (int i = 0; i < MAX_SPARKS && count < sparksToSpawn; i++) {
        if (!sparkPool[i].isActive) {
            sparkPool[i].isActive = true;
            sparkPool[i].posX = x;
            sparkPool[i].posY = y;
            float angleOffset = (AERandFloat() * 1.0f) - 0.5f;
            float finalAngle = (baseAngle + PI) + angleOffset; 

            sparkPool[i].dirX = cosf(finalAngle);
            sparkPool[i].dirY = sinf(finalAngle);

            sparkPool[i].speed = 100.0f + (AERandFloat() * 450.0f);
            sparkPool[i].size = 3.0f + (AERandFloat() * 4.0f);
            sparkPool[i].maxLifetime = 0.12f + (AERandFloat() * 0.15f);
            sparkPool[i].lifetime = sparkPool[i].maxLifetime;

            count++;
        }
    }
}

// --- UPDATE FUNCTION ---
void Animations_Update(float dt) {

	// update active explosions
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
		//skip inactive explosions
        if (explosionPool[i].active == false) continue;

		//update timer for the explosion
        explosionPool[i].timer += dt;

		//if timer exceeds frame time, advance to the next frame and reset timer
        if (explosionPool[i].timer >= FRAME_TIME) {
            explosionPool[i].currentFrame++;
            explosionPool[i].timer = 0.0f;

			//if we've reached the end of the animation frames, deactivate the explosion
            if (explosionPool[i].currentFrame >= TOTAL_FRAMES) {
                explosionPool[i].active = false;
            }
        }
    }

	// Update active bullet sparks
    for (int i = 0; i < MAX_SPARKS; i++) {
        if (!sparkPool[i].isActive) continue;

        sparkPool[i].posX += sparkPool[i].dirX * sparkPool[i].speed * dt;
        sparkPool[i].posY += sparkPool[i].dirY * sparkPool[i].speed * dt;

        sparkPool[i].speed *= 0.92f;
        sparkPool[i].lifetime -= dt;

        if (sparkPool[i].lifetime <= 0.0f) sparkPool[i].isActive = false;
    }
}

// --- DRAW FUNCTION ---
void Animations_Draw() {
    if (pExplosionSheet == nullptr || pAnimMesh == nullptr) return;

	//set render mode, blend mode, transparency, and color for drawing explosions
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

	// --- DRAW ACTIVE EXPLOSIONS ---
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionPool[i].active == false) continue;

		//calculate UV offsets based on the current frame of the explosion animation
        float uOffset = (float)explosionPool[i].currentFrame * (1.0f / 5.0f);
        float vOffset = 0.0f;

		//set the texture and UV offsets for the current frame of the explosion animation
        AEGfxTextureSet(pExplosionSheet, uOffset, vOffset);

		//create transformation matrix to position and scale the explosion sprite correctly on the screen
        AEMtx33 mScale, mTrans, mFinal;
        AEMtx33Scale(&mScale, explosionPool[i].scale, explosionPool[i].scale);
        AEMtx33Trans(&mTrans, explosionPool[i].pos.x, explosionPool[i].pos.y);
        AEMtx33Concat(&mFinal, &mTrans, &mScale);

        AEGfxSetTransform(mFinal.m);
        AEGfxMeshDraw(pAnimMesh, AE_GFX_MDM_TRIANGLES);
    }

	// --- DRAW ACTIVE BULLET SPARKS ---
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    for (int i = 0; i < MAX_SPARKS; i++) {
        if (!sparkPool[i].isActive) continue;

        float alpha = sparkPool[i].lifetime / sparkPool[i].maxLifetime;

        AEMtx33 mScale, mTrans, mFinal;
        AEMtx33Scale(&mScale, sparkPool[i].size, sparkPool[i].size);
        AEMtx33Trans(&mTrans, sparkPool[i].posX, sparkPool[i].posY);
        AEMtx33Concat(&mFinal, &mTrans, &mScale);

        AEGfxSetTransform(mFinal.m);
        AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, alpha);
        AEGfxMeshDraw(pAnimMesh, AE_GFX_MDM_TRIANGLES); 
    }
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
}

// --- FREE FUNCTION: Unload textures and free meshes ---
void Animations_Free() {
    if (pExplosionSheet != nullptr) {
        AEGfxTextureUnload(pExplosionSheet);
        pExplosionSheet = nullptr;
    }
    if (pAnimMesh != nullptr) {
        AEGfxMeshFree(pAnimMesh);
        pAnimMesh = nullptr;
    }
}