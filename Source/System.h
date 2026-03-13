#pragma once
#include "AEEngine.h"
#include "Structs.h"

void Initialise_System(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow);

void Load_Global_Assets();

void Unload_Global_Assets();
