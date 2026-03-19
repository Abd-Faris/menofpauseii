
#pragma once

// ===========================================================================
// PAUSE MENU HEADER
// ===========================================================================
namespace PauseScreen {
    // ---> NEW: A dedicated variable just for pausing! <---
    extern bool isPaused;

    void LoadPause();
    void UpdatePause();
    void DrawPause();
    void FreePause();
}