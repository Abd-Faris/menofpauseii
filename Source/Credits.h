// ----------------------------- Gloomy's Revenge ----------------------------- //
// File:    Credits.h
// Authors: [Men of Pause II]
// Brief:   Function declarations for managing the credits game state, 
//          including lifecycle functions for loading, updating, and drawing.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// FUNCTIONS
// =============================================================================

// ~ Brief: Load the background textures and build meshes required for the credits.
void LoadCredits();

// ~ Brief: Initialize the starting scroll positions for all credit text elements.
void InitializeCredits();

// ~ Brief: Handle frame-by-frame scrolling logic and input detection for skipping.
void UpdateCredits();

// ~ Brief: Render the background and credit text blocks to the screen.
void DrawCredits();

// ~ Brief: Free the meshes used by the credits system from memory.
void FreeCredits();

// ~ Brief: Unload textures and assets associated with the credits system.
void UnloadCredits();