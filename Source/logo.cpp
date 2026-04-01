// -----------------------------Gloomy's Revenge---------------------------- //
// File:	logo.cpp
// Authors:	[Men of Pause II]
// Brief:	This file contains the Game State logic for displaying and 
//          animating the DigiPen logo splash screen before the main menu.
// 
// ------------------------------------------------------------------------- //

// ------INCLUDE FILES------------------------------------------------------ //
#include "MasterHeader.h"
//#define PI  3.14159265f

// ------GLOBAL VARIABLES & POOLS------------------------------------------- //
static double dpLogoTimer = 0.0;        // Tracks how long the logo has been on screen
AEGfxTexture* dpLogoTexture = nullptr;  // Pointer to the loaded logo image data
AEGfxVertexList* pMesh = nullptr;       // 2D square mesh to paint the texture onto


// ------GAME STATE LOGIC--------------------------------------------------- //

// ~ Brief:	Loads the DigiPen logo texture from the Assets folder and 
//          generates a 1x1 square mesh to render the image onto.
void LoadDPLogo() {
    // 1. Load the texture from the Assets directory
    dpLogoTexture = AEGfxTextureLoad("./Assets/DigiPen_Singapore_WEB_RED.png");

    // 2. Quick safety check: If it fails to load, alert the console
    if (dpLogoTexture == nullptr) {
        std::cout << "ERROR: Failed to load DP Logo texture!" << std::endl;
    }

    // 3. Create a 1x1 Square Mesh for the texture to map to
    AEGfxMeshStart();

    // First Triangle (X, Y, Color, Texture U, Texture V)
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

// ~ Brief:	Resets the state variables each time the splash screen is entered.
void InitializeDPLogo() {
    // Reset the timer so it always starts fresh at 0 seconds
    dpLogoTimer = 0.0;
}

// ~ Brief:	Updates the state timer and listens for user input to skip the 
//          splash screen, transitioning to the main menu.
void UpdateDPLogo() {
    // 1. Add the frame time (delta time) to the state timer
    dpLogoTimer += (double)AEFrameRateControllerGetFrameTime();

    // 2. Check for skip conditions: 5 seconds passed OR Left Click OR Escape Key
    if (dpLogoTimer >= 5.0 || AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_LBUTTON)) {

        // 3. Trigger the Game State Manager to move to the Main Menu
        GS_next = GS_MAIN_MENU;
    }
}

// ~ Brief:	Renders the DigiPen logo to the center of the screen, slowly 
//          scaling it up over time to create a smooth zoom-in effect.
void DrawDPLogo() {
    // 1. Configure engine render modes for textured drawing with transparency
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // 2. Keep original texture colors (multiply by 1, add 0)
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // 3. Bind the DigiPen logo texture to the engine
    AEGfxTextureSet(dpLogoTexture, 0.0f, 0.0f);

    // 4. Calculate Transformation (Position, Rotation, Scale)
    // ADDED 'rot' to our list of matrices
    AEMtx33 scale, rot, trans, transform;

    // --- EXPANSION MATH ---
    float grow = 1.0f + (float)(dpLogoTimer * 0.2f);
    AEMtx33Scale(&scale, 600.0f * grow, 200.0f * grow);

    // --- ROTATION MATH ---
    float angle = 0.0f;
    // If we are in the last 1 second of the 5-second timer...
    if (dpLogoTimer >= 4.0) {
        // Calculate how much time has passed since the 4-second mark (goes from 0.0 to 1.0)
        float rotationTime = (float)(dpLogoTimer - 4.0);

        // Multiply time by 2*PI (a full circle in radians)
        angle = rotationTime * 2.0f * PI;
    }
    // Apply the angle to the rotation matrix
    AEMtx33Rot(&rot, angle);
    // ----------------------

    // 5. Set position to (0, 0) - Center of the screen
    AEMtx33Trans(&trans, 0.0f, 0.0f);

    // 6. Combine matrices and send to the graphics engine
    // Scale -> Rotate -> Translate
    AEMtx33Concat(&transform, &rot, &scale);        // 1. Rotate the Scaled logo
    AEMtx33Concat(&transform, &trans, &transform);  // 2. Translate it to its final position
    AEGfxSetTransform(transform.m);

    // 7. Draw the square mesh with the texture painted on it
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}

// ~ Brief:	Frees any dynamically allocated instances used in Initialize.
void FreeDPLogo() {
    // Free anything allocated in Initialize (Nothing right now)
}

// ~ Brief:	Unloads the texture and frees the mesh geometry from memory 
//          to prevent memory leaks before changing states.
void UnloadDPLogo() {
    // 1. Unload texture from memory
    if (dpLogoTexture != nullptr) {
        AEGfxTextureUnload(dpLogoTexture);
        dpLogoTexture = nullptr;
    }

    // 2. Free the mesh geometry from memory
    if (pMesh != nullptr) {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }
}