// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Controls.h
// Authors: [Men of Pause II]
// Brief:   Controls screen — displays key bindings and provides a
//          back-to-menu button.
// ------------------------------------------------------------------------- //

#pragma once

// ~ Brief: Load all textures and build meshes needed for the controls screen.
void LoadControls();

// ~ Brief: Assign the shared rect mesh to all controls buttons.
void InitializeControls();

// ~ Brief: Nothing to free between game-state transitions for this screen.
void DrawControls();

// ~ Brief: Poll for button clicks and handle game-state transitions.
void UpdateControls();

// ~ Brief: Nothing to free between game-state transitions for this screen.
void FreeControls();

// ~ Brief: Free all meshes and textures owned by the controls screen to prevent memory leaks.
void UnloadControls();