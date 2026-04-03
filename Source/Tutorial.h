// -----------------------------Gloomy's Revenge---------------------------- //
// File:    Tutorial.cpp
// Authors: [Men of Pause II]
// Brief:   Tutorial overlay — displays context-sensitive prompt text during
//          Wave 1 (movement and shooting hints) and Wave 2 (card shop guide).
//          Steps advance on player input and persist across frames via a
//          shared index.
// ------------------------------------------------------------------------- //

#pragma once

#include "MasterHeader.h"

// =============================================================================
// TUTORIAL FUNCTIONS
// =============================================================================

// ~ Brief: Reset the tutorial step counter to the beginning.
//          Call this whenever a new game session starts.
void resetTutorial();

// ~ Brief: Draw the tutorial overlay for the current wave and tutorial step.
//          Wave 1 steps (0–2) cover movement and shooting controls.
//          Wave 2 steps (2–6) walk the player through the card shop.
//          Each step advances on Space or Left Click, except the final hint
//          of each wave which waits for the player to act in-world.
void printtutorial();