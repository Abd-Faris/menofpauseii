// -----------------------------Gloomy's Revenge---------------------------- //
// File:	Pause.h
// Authors:	[Men of Pause II]
// Brief:	Header for the mid-game pause menu and overlay logic.
// ------------------------------------------------------------------------- //

#pragma once

// =============================================================================
// PAUSE MENU SYSTEM
// =============================================================================

namespace PauseScreen {

	// Global toggle to halt game simulation and input
	extern bool isPaused;

	// ~ Brief: Loads UI textures and button assets for the pause menu.
	void LoadPause();

	// ~ Brief: Handles menu navigation, resume, and quit button logic.
	void UpdatePause();

	// ~ Brief: Renders the dimming overlay and pause menu buttons.
	void DrawPause();

	// ~ Brief: Unloads pause-specific UI assets from memory.
	void FreePause();

	// ~ Brief: Renders the small pause icon/button during active gameplay.
	void DrawPauseButton();
}