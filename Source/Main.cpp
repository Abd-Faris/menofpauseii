// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Main.cpp
// Authors: [Men of Pause II]
// Brief:   Entry point of the program — initialises all systems and runs the
//          main game-state loop.
// ------------------------------------------------------------------------- //

#include <iostream>
#include "MasterHeader.h"
#include <direct.h>

// =============================================================================
// ENTRY POINT
// =============================================================================

// ~ Brief: Program entry point. Initialises engine systems and the game-state
//          manager (GSM), then drives the outer game-state loop and the inner
//          per-frame loop until the player quits.
int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    // ---- Startup ----
    Initialise_System(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    Load_Global_Assets();

    // Seed the GSM at the logo screen
    GSM_Initialize(GS_LOGO);

    // =========================================================================
    // OUTER LOOP — runs once per game-state transition
    // =========================================================================
    while (GS_current != GS_QUIT)
    {
        if (GS_current != GS_RESTART)
        {
            // Normal transition: resolve the new state's function pointers and load its assets
            GSM_Update();
            if (GS_Functions.Load) GS_Functions.Load();
        }
        else
        {
            // Restart: reuse the previous game state without reloading assets
            GS_next = GS_previous;
            GS_current = GS_previous;
        }

        // Initialise the incoming game state
        if (GS_Functions.Initialize) GS_Functions.Initialize();

        // =====================================================================
        // INNER LOOP — runs once per frame while the game state is unchanged
        // =====================================================================
        while (GS_next == GS_current)
        {
            AESysFrameStart();  // begin frame (clears buffers, polls input)

            // Global keybind — toggle fullscreen regardless of current game state
            if (AEInputCheckTriggered(AEVK_F11)) {
                fullscreen = !fullscreen;
                updateFullScreen();
            }

            // Per-frame update and draw
            if (GS_Functions.Update) GS_Functions.Update();
            if (GS_Functions.Draw)   GS_Functions.Draw();

            AESysFrameEnd();    // end frame (present, cap frame rate)
        }

        // Free per-state runtime data
        if (GS_Functions.Free) GS_Functions.Free();

        // Unload per-state assets — skipped on restart so assets can be reused
        if (GS_next != GS_RESTART) {
            if (GS_Functions.Unload) GS_Functions.Unload();
        }

        // Advance the game-state history by one step
        GS_previous = GS_current;
        GS_current = GS_next;
    }

    // ---- Shutdown ----
    Unload_Global_Assets();
    return 0;
}