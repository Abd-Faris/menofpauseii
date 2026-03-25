
#include "MasterHeader.h"

// Define Externs
s8 boldPixels{};
bool fullscreen = false;
void Initialise_System(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// set gamestate to "Running"
	int gGameRunning = 1;

	// Initialization of your own variables go here

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);

	// Changing the window title
	AESysSetWindowTitle("Men of Pause II: Gloomy's Revenge");

	// reset the system modules
	AESysReset();

	printf("Hello World\n");
}

void Load_Global_Assets() {
	// Load Cards from JSON
	Cards::Load_Cards("../../Data/Cards.json");
	// Load Font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
}

void Unload_Global_Assets() {
	// Unload Cards
	for (int i{}; i < NUM_OF_RARITIES; i++) {
		cardPool[i].clear();
		cardPool[i].shrink_to_fit();
	}
	// Unload Font
	AEGfxDestroyFont(boldPixels);
}

