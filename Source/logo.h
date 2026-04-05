// -----------------------------Gloomy's Revenge---------------------------- //
// File:	DigiPenLogo.h
// Authors:	[Men of Pause II]
// Brief:	Header for the DigiPen logo screen and opening credits.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// SPLASH SCREEN FUNCTIONS
// =============================================================================

// ~ Brief: Loads the DigiPen logo texture and startup audio assets.
void LoadDPLogo();

// ~ Brief: Initializes timers and alpha transparency for the fade-in effect.
void InitializeDPLogo();

// ~ Brief: Handles the splash screen duration and transition to the Main Menu.
void UpdateDPLogo();

// ~ Brief: Renders the logo with current transparency settings to the screen.
void DrawDPLogo();

// ~ Brief: Cleans up transient memory used during the logo sequence.
void FreeDPLogo();

// ~ Brief: Fully unloads the logo textures from the GPU.
void UnloadDPLogo();