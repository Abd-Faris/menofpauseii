#pragma once
#include "AEEngine.h"
#include "Structs.h"

// Declare Externs
extern s8 boldPixels;
extern bool fullscreen;

void Initialise_System(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow);

void updateFullScreen();

void Load_Global_Assets();

void Unload_Global_Assets();

namespace SFX {
	void load();
	void unload();
	void playBGM();
	void playSFX(int);
}