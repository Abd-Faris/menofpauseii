// -----------------------------Gloomy's Revenge---------------------------- //
// File:    GameStateManager.cpp
// Authors: [Men of Pause II]
// Brief:   Defines the Game State Manager (GSM) — maintains current, previous,
//          and next game states, maps each state to its function pointers, and
//          drives state transitions each frame.
// ------------------------------------------------------------------------- //

#include <iostream>
#include "MasterHeader.h"

// =============================================================================
// GLOBALS
// =============================================================================

int GS_current = 0;  // currently executing game state
int GS_previous = 0;  // game state that was active last frame
int GS_next = 0;  // game state to transition to at end of frame

GSFunctions GS_Functions;  // function pointer set for the current game state

// =============================================================================
// GAME STATE TABLE
// =============================================================================

// Each entry maps a game state (by enum index) to its six lifecycle functions:
// { Load, Initialize, Update, Draw, Free, Unload }
// nullptr is used where a lifecycle function is not needed for that state.
// ENSURE THE ORDER MATCHES THE ENUM IN GameStateManager.h!
std::vector<GSFunctions> gamestates{

    // ---- Standard game states ----
    {LoadDPLogo,      InitializeDPLogo,      UpdateDPLogo,      DrawDPLogo,      FreeDPLogo,      UnloadDPLogo     },
    {LoadMainMenu,    InitializeMainMenu,    UpdateMainMenu,    DrawMainMenu,    nullptr,          UnloadMainMenu   },
    {LoadGame,        InitializeGame,        UpdateGame,        DrawGame,        FreeGame,        UnloadGame       },
    {LoadResults,     nullptr,               UpdateResults,     DrawResults,     FreeResults,     nullptr          },
    {LoadLevelSelect, InitializeLevelSelect, UpdateLevelSelect, DrawLevelSelect, FreeLevelSelect, UnloadLevelSelect},
    {LoadCredits,     InitializeCredits,     UpdateCredits,     DrawCredits,     FreeCredits,     UnloadCredits    },
    {LoadSettings,    InitializeSettings,    UpdateSettings,    DrawSettings,    FreeSettings,    UnloadSettings   },
    {LoadControls,    InitializeControls,    UpdateControls,    DrawControls,    FreeControls,    UnloadControls   },

    // ---- Debugging game states ----
    {LoadDebug1,      nullptr,               UpdateDebug1,      DrawDebug1,      FreeDebug1,      nullptr          },
    {LoadCardShop,    InitializeCardShop,    UpdateCardShop,    DrawCardShop,    FreeCardShop,    UnloadCardShop   },
};

// =============================================================================
// GSM FUNCTIONS
// =============================================================================

// ~ Brief: Initialise the GSM with a starting game state.
//          Sets current, previous, and next to the same state so the first
//          frame loads and enters cleanly without a spurious transition.
void GSM_Initialize(int GS_startingState) {
    GS_current = GS_previous = GS_next = GS_startingState;
    printf("GSM:Initialize\n");
}

// ~ Brief: Resolve the function pointer set for the current game state.
//          Called once per frame before the lifecycle functions are dispatched.
void GSM_Update() {
    GS_Functions = gamestates[GS_current];  // bind lifecycle functions for this state
}