
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
	//int gGameRunning = 1;

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
	// Load SFX
	SFX::load();
}

void Unload_Global_Assets() {
	// Unload Cards
	for (int i{}; i < NUM_OF_RARITIES; i++) {
		cardPool[i].clear();
		cardPool[i].shrink_to_fit();
	}
	// Unload Font
	AEGfxDestroyFont(boldPixels);
	// Unload SFX
	SFX::unload();
}

namespace SFX {
	// namespace scope variables
	AEAudio gamebgm{ nullptr };
	AEAudio mainbgm{ nullptr };
	AEAudio shopbgm{ nullptr };
	AEAudio credbgm{ nullptr };
	AEAudioGroup bgm{ nullptr };

	void load() {
		// loads audio groups
		bgm = AEAudioCreateGroup();

		// loads bgm
		mainbgm = AEAudioLoadMusic("Assets/audio/bgm/main_bgm.mp3");
		gamebgm = AEAudioLoadMusic("Assets/audio/bgm/game_bgm.mp3");
		shopbgm = AEAudioLoadMusic("Assets/audio/bgm/shop_bgm.mp3");
		credbgm = AEAudioLoadMusic("Assets/audio/bgm/cred_bgm.mp3");
	}

	void unload() {
		// unload audio groups
		AEAudioUnloadAudioGroup(bgm);

		// unload bgms
		AEAudioUnloadAudio(mainbgm);
		AEAudioUnloadAudio(gamebgm);
		AEAudioUnloadAudio(shopbgm);
		AEAudioUnloadAudio(credbgm);
	}

	void playBGM() {
		switch (GS_current) {
		case GS_MAIN_MENU:
			AEAudioStopGroup(bgm); AEAudioPlay(mainbgm, bgm, 1.f, 1.f, -1); break;
		case GS_GAME:
			AEAudioStopGroup(bgm); AEAudioPlay(gamebgm, bgm, 2.f, 1.f, -1); break;
		case GS_CARD_SHOP:
			AEAudioStopGroup(bgm); AEAudioPlay(shopbgm, bgm, 2.f, 1.f, -1); break;
		case GS_CREDITS:
			AEAudioStopGroup(bgm); AEAudioPlay(credbgm, bgm, 2.f, 1.f, -1); break;
		default: break;
		}
	}

	void playSFX(int sfx) {
		switch (sfx) {
			//
		}
	}
}

