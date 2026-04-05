// -----------------------------Gloomy's Revenge---------------------------- //
// File:	logo.cpp
// Authors:	[Men of Pause II]
// Brief:	This file contains the Game State logic for displaying and 
//          animating the DigiPen logo splash screen before the main menu,
//          including a rotational and particle explosion transition.
// 
// ------------------------------------------------------------------------- //

// ------INCLUDE FILES------------------------------------------------------ //
#include "MasterHeader.h"
//#define PI  3.14159265f

// ------GLOBAL VARIABLES & POOLS------------------------------------------- //
static double dpLogoTimer = 0.0;        // Tracks how long the logo has been on screen
AEGfxTexture* dpLogoTexture = nullptr;  // Pointer to the loaded logo image data
AEGfxVertexList* pMesh = nullptr;       // 2D square mesh to paint the texture onto




const int EXPLOSION_COUNT = 150; // Number of particles
SplashParticle particles[EXPLOSION_COUNT];
bool explosionTriggered = false; // Ensures the explosion only triggers once


// ------GAME STATE LOGIC--------------------------------------------------- //

// ~ Brief:	Loads the DigiPen logo texture and the 1x1 square mesh.
void LoadDPLogo() {
    dpLogoTexture = AEGfxTextureLoad("./Assets/DigiPen_Singapore_WEB_RED.png");

    if (dpLogoTexture == nullptr) {
        //std::cout << "ERROR: Failed to load DP Logo texture!" << std::endl;
    }

    AEGfxMeshStart();
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pMesh = AEGfxMeshEnd();
}

// ~ Brief:	Resets the state timers and deactivates all particles.
void InitializeDPLogo() {
    dpLogoTimer = 0.0;
    explosionTriggered = false;

    // Reset the particle pool
    for (int i = 0; i < EXPLOSION_COUNT; i++) {
        particles[i].active = false;
    }
}

// ~ Brief:	Updates timers, physics, and listens for state transitions.
void UpdateDPLogo() {
    float dt = (float)AEFrameRateControllerGetFrameTime();
    dpLogoTimer += (double)dt;

    // --- EXPLOSION LOGIC ---
    // If we hit 4 seconds, trigger the explosion blast exactly once
    if (dpLogoTimer >= 4.0 && !explosionTriggered) {
        explosionTriggered = true;

        for (int i = 0; i < EXPLOSION_COUNT; i++) {
            particles[i].active = true;
            particles[i].x = 0.0f; // Start at center of screen
            particles[i].y = 0.0f;

            // Random direction (0 to 2*PI)
            float angle = AERandFloat() * 2.0f * PI;
            // Random blast speed (between 300 and 1000 pixels per second)
            float speed = 300.0f + AERandFloat() * 700.0f;

            particles[i].velX = cosf(angle) * speed;
            particles[i].velY = sinf(angle) * speed;

            // Random life span between 0.5s and 1.2s
            particles[i].maxLife = 0.5f + AERandFloat() * 0.7f;
            particles[i].life = particles[i].maxLife;

            // Random size between 4 and 12 pixels
            particles[i].scale = 30.0f + AERandFloat() * 8.0f;
        }
    }

    // Update active particles (move them and fade them out)
    if (explosionTriggered) {
        for (int i = 0; i < EXPLOSION_COUNT; i++) {
            if (particles[i].active) {
                particles[i].x += particles[i].velX * dt;
                particles[i].y += particles[i].velY * dt;

                // Add a little bit of "drag" so they slow down over time
                particles[i].velX *= 0.95f;
                particles[i].velY *= 0.95f;

                particles[i].life -= dt;
                if (particles[i].life <= 0.0f) {
                    particles[i].active = false;
                }
            }
        }
    }

    // --- SKIP LOGIC ---
    if (dpLogoTimer >= 5.0 || AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_LBUTTON)) {
        GS_next = GS_MAIN_MENU;
    }
}

// ~ Brief:	Renders the scaling logo and the particle explosion.
void DrawDPLogo() {
    // ==========================================
    // 1. DRAW THE LOGO
    // ==========================================
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxTextureSet(dpLogoTexture, 0.0f, 0.0f);

    AEMtx33 scale, rot, trans, transform;

    // Zoom in
    float grow = 1.0f + (float)(dpLogoTimer * 0.2f);
    AEMtx33Scale(&scale, 600.0f * grow, 200.0f * grow);

    // Spin in the last 1 second
    float angle = 0.0f;
    if (dpLogoTimer >= 4.0) {
        float rotationTime = (float)(dpLogoTimer - 4.0);
        angle = rotationTime * 2.0f * PI;
    }
    AEMtx33Rot(&rot, angle);
    AEMtx33Trans(&trans, 0.0f, 0.0f);

    AEMtx33Concat(&transform, &rot, &scale);
    AEMtx33Concat(&transform, &trans, &transform);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);


    // ==========================================
    // 2. DRAW THE EXPLOSION PARTICLES
    // ==========================================
    if (explosionTriggered) {
        // Switch to COLOR render mode since particles don't have textures
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        for (int i = 0; i < EXPLOSION_COUNT; i++) {
            if (particles[i].active) {

                // Calculate how faded it should be based on its remaining life
                float alpha = particles[i].life / particles[i].maxLife;

                // Set color 
                AEGfxSetColorToMultiply(1.0f, 0.6f + (alpha * 0.4f), 0.0f, alpha);

                AEMtx33 pScale, pTrans, pTransform;
                AEMtx33Scale(&pScale, particles[i].scale, particles[i].scale);
                AEMtx33Trans(&pTrans, particles[i].x, particles[i].y);
                AEMtx33Concat(&pTransform, &pTrans, &pScale);

                AEGfxSetTransform(pTransform.m);
                AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
            }
        }
    }
}

// ~ Brief:	Frees instances
void FreeDPLogo() {}

// ~ Brief:	Unloads texture/mesh to prevent leaks
void UnloadDPLogo() {
    if (dpLogoTexture != nullptr) {
        AEGfxTextureUnload(dpLogoTexture);
        dpLogoTexture = nullptr;
    }
    if (pMesh != nullptr) {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }
}