// -----------------------------Gloomy's Revenge---------------------------- //
// File:    MainMenu.h
// Authors: [Men of Pause II]
// Brief:   Main menu screen — navigation buttons (Play, Credits, Controls,
//          Settings, Exit) and exit-confirmation overlay.
// ------------------------------------------------------------------------- //

#pragma once

// ~ Brief: Load all textures and build meshes needed for the main menu screen.
void LoadMainMenu();

// ~ Brief: Reset the exit-confirmation flag, assign the shared rect mesh to all buttons,
//          and begin BGM playback.
void InitializeMainMenu();

// ~ Brief: Render the main menu in layer order:
//          background (normal or exit overlay), then the active button set and labels.
void DrawMainMenu();

// ~ Brief: Poll for button clicks and handle game-state transitions.
void UpdateMainMenu();

// ~ Brief: Free all meshes and textures owned by the main menu screen to prevent memory leaks.
void UnloadMainMenu();
