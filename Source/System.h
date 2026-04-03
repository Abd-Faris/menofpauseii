// -----------------------------Gloomy's Revenge---------------------------- //
// File:    System.h
// Authors: [Men of Pause II]
// Brief:   Engine initialisation, global asset management, and the SFX/BGM
//          audio system.
// ------------------------------------------------------------------------- //

#pragma once
#include "AEEngine.h"
#include "Structs.h"

// =============================================================================
// EXTERNS
// =============================================================================
extern s8 boldPixels;
extern bool fullscreen;

// =============================================================================
// SYSTEM INITIALISATION
// =============================================================================

// ~ Brief: Initialise the Alpha Engine, configure the window, and enable the
//          CRT debug heap for leak detection in debug builds.
void Initialise_System(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow);

// ~ Brief: Apply the current fullscreen state to the Alpha Engine window.
//          Called whenever fullscreen is toggled in settings or via F11.
void updateFullScreen();

// ~ Brief: Load all assets shared across every game state: card data, font, and SFX.
//          Called once at startup before the GSM loop begins.
void Load_Global_Assets();

// ~ Brief: Release all globally shared assets on shutdown to prevent memory leaks.
void Unload_Global_Assets();

// =============================================================================
// SFX NAMESPACE — audio loading, unloading, BGM routing, and SFX playback
// =============================================================================

namespace SFX {

	// ~ Brief: Create audio groups and load all BGM tracks and SFX variants into memory.
	//          sfxLibrary is indexed as [sfx type][variant], mirroring sfxPaths.
	void load();

	// ~ Brief: Unload all audio groups, BGM tracks, and SFX variants to prevent memory leaks.
	void unload();

	// ~ Brief: Stop any currently playing BGM and start the track appropriate for
	//          the current game state. Looping is enabled (repeat = -1).
	void playBGM();

	// ~ Brief: Play a random variant of the requested SFX type at the current sfxVolume,
	//          scaled by the per-type normalisation factor. Silently ignores invalid IDs
	//          or types with no loaded variants.
	void playSFX(int);
}