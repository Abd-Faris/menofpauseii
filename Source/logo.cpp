#include "MasterHeader.h"

// Global or static variables for this state
static double dpLogoTimer = 0.0;
AEGfxTexture* dpLogoTexture = nullptr;

// We need a mesh (a 2D square) to paint the texture onto
AEGfxVertexList* pMesh = nullptr;

void LoadDPLogo() {
    dpLogoTexture = AEGfxTextureLoad("./Assets/DigiPen_Singapore_WEB_RED.png");

    // Quick safety check: If it fails to load, the pointer will still be nullptr
    if (dpLogoTexture == nullptr) {
        std::cout << "ERROR: Failed to load DP Logo texture!" << std::endl;
    }

    // --- Create a 1x1 Square Mesh ---
    AEGfxMeshStart();

    // First Triangle
    // Arguments: X, Y, Color, Texture U, Texture V
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    // Second Triangle
    AEGfxTriAdd(
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    pMesh = AEGfxMeshEnd();
}

void InitializeDPLogo() {
    // Reset the timer every time we enter this state
    dpLogoTimer = 0.0;
}

void UpdateDPLogo() {
    // 1. Add the frame time (delta time) to your timer
    dpLogoTimer += (double)AEFrameRateControllerGetFrameTime();

    // 2. Check for Left Mouse Click (LBUTTON) OR Escape OR if 10 seconds have passed
    if (dpLogoTimer >= 10.0 || AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_LBUTTON)) {
        // 3. Trigger the state change to move to the Main Menu
        GS_next = GS_MAIN_MENU;
    }
}

void DrawDPLogo() {
    // 1. Tell the engine we want to draw a texture, not just flat colors
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

    // 2. Turn on alpha blending so the transparent parts of your PNG show the background
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);

    // 3. Keep original colors (multiply by 1) and don't artificially brighten (add 0)
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // 4. Bind our specific DigiPen logo texture
    AEGfxTextureSet(dpLogoTexture, 0.0f, 0.0f);

    // 5. Calculate Transformation (Position, Rotation, Scale)
    AEMtx33 scale, trans, transform;

    // Set how big the logo should be in pixels (Adjust these numbers to fit your screen!)
    AEMtx33Scale(&scale, 600.0f, 200.0f);

    // Set position to (0, 0), which is usually the center of the screen in Alpha Engine
    AEMtx33Trans(&trans, 0.0f, 0.0f);

    // Combine them and send to the graphics engine
    AEMtx33Concat(&transform, &trans, &scale);
    AEGfxSetTransform(transform.m);

    // 6. Draw the square mesh with the texture painted on it
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}

void FreeDPLogo() {
    // Free anything allocated in Initialize (Nothing right now)
}

void UnloadDPLogo() {
    // Unload texture
    if (dpLogoTexture != nullptr) {
        AEGfxTextureUnload(dpLogoTexture);
        dpLogoTexture = nullptr;
    }

    // Free the mesh geometry from memory
    if (pMesh != nullptr) {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }
}