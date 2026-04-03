// -----------------------------Gloomy's Revenge---------------------------- //
// File:    LevelSelect.h
// Authors: [Men of Pause II]
// Brief:   Level select screen — New Game / Continue buttons, tutorial and
//          cheat toggles, and back-to-menu navigation.
//          Continue is hidden when no save file exists.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// EXTERNS
// =============================================================================
extern bool tutorialOn;
extern bool cheatsOn;

// ~ Brief: Load all textures and build meshes needed for the level-select screen.
void LoadLevelSelect();

// ~ Brief: Check for an existing save file, sync toggle labels to current state,
//          and assign the shared rect mesh to all buttons.
void InitializeLevelSelect();

// ~ Brief: Render the level-select screen in layer order:
//          background, then the button set and text labels.
void DrawLevelSelect();

// ~ Brief: Poll for button clicks and handle game-state transitions.
void UpdateLevelSelect();

// ~ Brief: Nothing to free between game-state transitions for this screen.
void FreeLevelSelect();

// ~ Brief: Free all meshes and textures owned by the level-select screen to prevent memory leaks.
void UnloadLevelSelect();
