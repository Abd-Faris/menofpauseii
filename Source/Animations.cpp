#include "MasterHeader.h"
#include "Animations.h"

namespace {
    const int MAX_EXPLOSIONS = 20;
    const int TOTAL_FRAMES = 5;
    const float FRAME_TIME = 0.08f;

    Explosion explosionPool[MAX_EXPLOSIONS];
    AEGfxTexture* pExplosionSheet = nullptr;

    AEGfxVertexList* pAnimMesh = nullptr;

    const float UV_X_STEP = 1.0f / 5.0f;
    const float UV_Y_STEP = 1.0f;
}

void Animations_Load() {
    pExplosionSheet = AEGfxTextureLoad("Assets/explosions.png");

    float frameWidth = 1.0 / 5.0f;

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

    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        explosionPool[i].active = false;
    }
}

void TriggerExplosion(float x, float y) {
    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionPool[i].active == false) {
            explosionPool[i].pos.x = x;
            explosionPool[i].pos.y = y;
            explosionPool[i].timer = 0.0f;
            explosionPool[i].currentFrame = 0;
            explosionPool[i].active = true;
            return;
        }
    }
}

void Animations_Update(float dt) {
    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionPool[i].active == false) continue;

        explosionPool[i].timer += dt;

        if (explosionPool[i].timer >= FRAME_TIME) {
            explosionPool[i].currentFrame++;
            explosionPool[i].timer = 0.0f;

            if (explosionPool[i].currentFrame >= TOTAL_FRAMES) {
                explosionPool[i].active = false;
            }
        }
    }
}

void Animations_Draw() {
    if (pExplosionSheet == nullptr || pAnimMesh == nullptr) return;

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionPool[i].active == false) continue;

        float uOffset = (float)explosionPool[i].currentFrame * (1.0f / 5.0f);
        float vOffset = 0.0f;

        AEGfxTextureSet(pExplosionSheet, uOffset, vOffset);

        AEMtx33 mScale, mTrans, mFinal;
        AEMtx33Scale(&mScale, 60.0f, 60.0f);
        AEMtx33Trans(&mTrans, explosionPool[i].pos.x, explosionPool[i].pos.y);
        AEMtx33Concat(&mFinal, &mTrans, &mScale);

        AEGfxSetTransform(mFinal.m);
        AEGfxMeshDraw(pAnimMesh, AE_GFX_MDM_TRIANGLES);
    }
}


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