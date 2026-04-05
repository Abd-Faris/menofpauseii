// -----------------------------Gloomy's Revenge---------------------------- //
// File:    System.cpp
// Authors: [Men of Pause II]
// Brief:   Engine initialisation, global asset management, and the SFX/BGM
//          audio system.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// GLOBALS
// =============================================================================

s8   boldPixels{};          // handle to the shared bold pixel font
bool fullscreen = false;    // tracks current fullscreen state (toggled by F11 or settings)

// =============================================================================
// SYSTEM INITIALISATION
// =============================================================================

// ~ Brief: Initialise the Alpha Engine, configure the window, and enable the
//          CRT debug heap for leak detection in debug builds.
void Initialise_System(_In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow) {
    // Enable CRT heap diagnostics — reports memory leaks on exit in debug builds
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialise the Alpha Engine: 1600×900, vsync on, 60 Hz target, windowed
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);

    AESysSetWindowTitle("Men of Pause II: Gloomy's Revenge");

    AESysReset();

    //printf("Hello World\n");
}

// ~ Brief: Apply the current fullscreen state to the Alpha Engine window.
//          Called whenever fullscreen is toggled in settings or via F11.
void updateFullScreen() {
    AESysSetFullScreen(fullscreen ? 1 : 0);
}

// =============================================================================
// GLOBAL ASSET MANAGEMENT
// =============================================================================

// ~ Brief: Load all assets shared across every game state: card data, font, and SFX.
//          Called once at startup before the GSM loop begins.
void Load_Global_Assets() {
    Cards::Load_Cards("../../Data/Cards.json");
    boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
    SFX::load();
}

// ~ Brief: Release all globally shared assets on shutdown to prevent memory leaks.
void Unload_Global_Assets() {
    // Release card pool memory for each rarity bucket
    for (int i{}; i < NUM_OF_RARITIES; i++) {
        cardPool[i].clear();
        cardPool[i].shrink_to_fit();
    }

    AEGfxDestroyFont(boldPixels);
    SFX::unload();
}

// =============================================================================
// SFX NAMESPACE — audio loading, unloading, BGM routing, and SFX playback
// =============================================================================

namespace SFX {

    // -------------------------------------------------------------------------
    // AUDIO HANDLES
    // -------------------------------------------------------------------------

    // BGM tracks — one per game state
    AEAudio gamebgm{ nullptr };  // in-game BGM
    AEAudio mainbgm{ nullptr };  // main menu BGM
    AEAudio shopbgm{ nullptr };  // card shop BGM
    AEAudio credbgm{ nullptr };  // credits BGM
    AEAudioGroup bgm{ nullptr }; // audio group for all BGM (allows stopping all BGM at once)

    // SFX pool
    std::vector<std::vector<AEAudio>> sfxLibrary;  // [sfx type][variant index]
    AEAudioGroup sfx{ nullptr };                    // audio group for all SFX

    // -------------------------------------------------------------------------
    // SFX FILE PATHS  (*** ADD NEW SFX VARIANTS HERE ***)
    // -------------------------------------------------------------------------

    // One row per SFX type; multiple paths in a row are played at random for variety
    const std::vector<std::vector<std::string>> sfxPaths = {
        { "Assets/audio/sfx/Enemy_Death1.mp3",      "Assets/audio/sfx/Enemy_Death2.mp3"      },  // SFX_ENEMY_DEATH
        { "Assets/audio/sfx/Enemy_Shoot.mp3"                                                  },  // SFX_ENEMY_SHOOT
        { "Assets/audio/sfx/Enemy_TakeDamage1.mp3", "Assets/audio/sfx/Enemy_TakeDamage2.mp3" },  // SFX_ENEMY_HIT
        { "Assets/audio/sfx/Player_Shoot.mp3"                                                 },  // SFX_PLAYER_SHOOT
        { "Assets/audio/sfx/Player_TakeDamage1.mp3","Assets/audio/sfx/Player_TakeDamage2.mp3"},  // SFX_PLAYER_HIT
        { "Assets/audio/sfx/UI_Button_Hover.mp3"                                              },  // SFX_UI_BUTTON_HOVER
        { "Assets/audio/sfx/UI_Button_Select.mp3"                                             },  // SFX_UI_BUTTON_SELECT
        { "Assets/audio/sfx/UI_Card_Hover.mp3"                                                },  // SFX_UI_CARD_HOVER
        { "Assets/audio/sfx/UI_Card_Pickup.mp3"                                               },  // SFX_UI_CARD_PICKUP
        { "Assets/audio/sfx/UI_Card_Place.mp3"                                                },  // SFX_UI_CARD_PLACE
        { "Assets/audio/sfx/UI_Card_Trash.mp3"                                                },  // SFX_UI_CARD_TRASH
    };

    // -------------------------------------------------------------------------
    // PER-TYPE VOLUME NORMALISATION  (*** TWEAK LOUD SFX HERE ***)
    // -------------------------------------------------------------------------

    // Multiplied against sfxVolume at playback time — use to balance individual SFX
    // without affecting the global volume slider.
    const std::vector<float> sfxNorm = {
        1.0f,  // SFX_ENEMY_DEATH
        0.8f,  // SFX_ENEMY_SHOOT
        1.0f,  // SFX_ENEMY_HIT
        0.6f,  // SFX_PLAYER_SHOOT
        0.6f,  // SFX_PLAYER_HIT
        1.0f,  // SFX_UI_BUTTON_HOVER
        1.5f,  // SFX_UI_BUTTON_SELECT
        1.0f,  // SFX_UI_CARD_HOVER
        1.0f,  // SFX_UI_CARD_PICKUP
        1.0f,  // SFX_UI_CARD_PLACE
        1.0f,  // SFX_UI_CARD_TRASH
    };

    // -------------------------------------------------------------------------
    // LOAD / UNLOAD
    // -------------------------------------------------------------------------

    // ~ Brief: Create audio groups and load all BGM tracks and SFX variants into memory.
    //          sfxLibrary is indexed as [sfx type][variant], mirroring sfxPaths.
    void load() {
        // Create separate groups so BGM and SFX can be stopped independently
        bgm = AEAudioCreateGroup();
        sfx = AEAudioCreateGroup();

        // BGM tracks
        mainbgm = AEAudioLoadMusic("Assets/audio/bgm/main_bgm.mp3");
        gamebgm = AEAudioLoadMusic("Assets/audio/bgm/game_bgm.mp3");
        shopbgm = AEAudioLoadMusic("Assets/audio/bgm/shop_bgm.mp3");
        credbgm = AEAudioLoadMusic("Assets/audio/bgm/cred_bgm.mp3");

        // SFX — resize library to match the number of SFX types, then load each variant
        sfxLibrary.resize(SFX_COUNT);
        for (int i = 0; i < SFX_COUNT; ++i) {
            for (const std::string& path : sfxPaths[i]) {
                AEAudio indivsfx = AEAudioLoadSound(path.c_str());
                sfxLibrary[i].push_back(indivsfx);
            }
        }
    }

    // ~ Brief: Unload all audio groups, BGM tracks, and SFX variants to prevent memory leaks.
    void unload() {
        AEAudioUnloadAudioGroup(bgm);
        AEAudioUnloadAudioGroup(sfx);

        AEAudioUnloadAudio(mainbgm);
        AEAudioUnloadAudio(gamebgm);
        AEAudioUnloadAudio(shopbgm);
        AEAudioUnloadAudio(credbgm);

        // Unload every SFX variant across all types
        for (auto& sfxtype : sfxLibrary)
            for (AEAudio& indivsfx : sfxtype)
                AEAudioUnloadAudio(indivsfx);

        sfxLibrary.clear();
    }

    // -------------------------------------------------------------------------
    // PLAYBACK
    // -------------------------------------------------------------------------

    // ~ Brief: Stop any currently playing BGM and start the track appropriate for
    //          the current game state. Looping is enabled (repeat = -1).
    void playBGM() {
        switch (GS_current) {
        case GS_MAIN_MENU:  AEAudioStopGroup(bgm); AEAudioPlay(mainbgm, bgm, bgmVolume, 1.f, -1); break;
        case GS_GAME:       AEAudioStopGroup(bgm); AEAudioPlay(gamebgm, bgm, bgmVolume, 1.f, -1); break;
        case GS_CARD_SHOP:  AEAudioStopGroup(bgm); AEAudioPlay(shopbgm, bgm, bgmVolume, 1.f, -1); break;
        case GS_CREDITS:    AEAudioStopGroup(bgm); AEAudioPlay(credbgm, bgm, bgmVolume, 1.f, -1); break;
        default: break;
        }
    }

    // ~ Brief: Play a random variant of the requested SFX type at the current sfxVolume,
    //          scaled by the per-type normalisation factor. Silently ignores invalid IDs
    //          or types with no loaded variants.
    void playSFX(int id) {
        if (id >= SFX_COUNT || sfxLibrary[id].empty()) return;

        // Pick a random variant from the SFX type's variant list
        int idx = (int)(AERandFloat() * sfxLibrary[id].size());

        // Final volume = global slider * per-type normalisation
        float volume = sfxVolume * sfxNorm[id];

        AEAudioPlay(sfxLibrary[id][idx], sfx, volume, 1.0f, 0);
    }

}  // namespace SFX