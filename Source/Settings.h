// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Settings.h
// Authors: [Men of Pause II]
// Brief:   Settings screen — volume sliders (SFX / BGM), fullscreen toggle,
//          and back-to-menu navigation.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// EXTERNS
// =============================================================================
extern float sfxVolume;
extern float bgmVolume;


// ~ Brief: Load all textures and build meshes needed for the settings screen.
void LoadSettings();

// ~ Brief: Reset drag state, sync the fullscreen label to the current setting,
//          and assign the shared rect mesh to every button.
void InitializeSettings();

// ~ Brief: Render the settings screen in layer order:
//          background, sliders, buttons, text labels, volume percentages.
void DrawSettings();

// ~ Brief: Poll cursor position, handle slider drag input, and process button clicks.
void UpdateSettings();

// ~ Brief: Nothing to free between game-state transitions for this screen.
void FreeSettings();

// ~ Brief: Free all meshes and textures owned by the settings screen to prevent memory leaks.
void UnloadSettings();

