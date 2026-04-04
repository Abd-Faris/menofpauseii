// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Results.h
// Authors: [Men of Pause II]
// Brief:   Declares the results screen state functions and the gameWon flag
//          used to determine whether to display the victory or defeat screen.
// ------------------------------------------------------------------------- //

#pragma once

// ~ Brief: Set to true if the player cleared the final boss wave, false if they died.
//          Must be assigned before transitioning to GS_RESULTS.
extern bool gameWon;

// ~ Brief: Load win/loss background textures and build button and background meshes.
void LoadResults();

// ~ Brief: Assign meshes to all result screen buttons. Called each time the state is entered.
void InitializeResults();

// ~ Brief: Poll for button clicks and handle transitions to the next game state.
void UpdateResults();

// ~ Brief: Draw the win or loss background and all result screen buttons and text.
void DrawResults();

// ~ Brief: Free all meshes and unload all textures used by the results screen.
void FreeResults();