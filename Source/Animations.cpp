#include "MasterHeader.h"
#include "Animations.h"

// ===========================================================================
// ANIMATION SYSTEM
// ===========================================================================

namespace {
	//initialise explosion pool frames
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

// --- UPDATE FUNCTION: Update active explosions ---
void Animations_Update(float dt) {
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
}

// --- DRAW FUNCTION: Draw active explosions ---
void Animations_Draw() {
    if (pExplosionSheet == nullptr || pAnimMesh == nullptr) return;

	//set render mode, blend mode, transparency, and color for drawing explosions
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

	//loop through explosion pool and draw active explosions with correct frame from sprite sheet
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