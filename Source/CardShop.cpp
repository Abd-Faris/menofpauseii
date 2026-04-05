// -----------------------------Gloomy's Revenge---------------------------- //
// File:    CardShop.cpp
// Authors: [Men of Pause II]
// Brief:   Card shop screen — card loading, generation, deck management,
//          drag-and-drop interaction, stat modifier computation, and drawing.
// ------------------------------------------------------------------------- //

#pragma once
#include "MasterHeader.h"
#include "../Extern/rapidjson/document.h"
#include "../Extern/rapidjson/error/en.h"
#include <fstream>
#include <sstream>

// =============================================================================
// GLOBALS
// =============================================================================

// All card arrays indexed by deck: [0] = shop, [1] = active, [2] = inventory
std::array<std::vector<Card>, 3>                    allCards{};

// Card pool bucketed by rarity — populated by Cards::Load_Cards()
std::array<std::vector<CardStats>, NUM_OF_RARITIES> cardPool;

// Additive and multiplicative stat modifiers applied by equipped cards
PlayerStatsModifier cardBaseMod{ 0.f, 0.f, 0.f, 0.f, 0.f };   // flat bonuses
PlayerStatsModifier cardMultMod{ 1.f, 1.f, 1.f, 1.f, 1.f };   // multipliers

// Bitmask tracking which passive weapon upgrades are currently active
u32 upgradeFlag{ UPGRADE_NONE };

// =============================================================================
// EXTERNS
// =============================================================================

extern int currentWave;  // current wave number (used for boss countdown text)

// =============================================================================
// ANONYMOUS NAMESPACE — file-private data and helpers (Load / Init)
// =============================================================================

namespace {

    // -------------------------------------------------------------------------
    // MESHES & TEXTURES
    // -------------------------------------------------------------------------

    AEGfxVertexList* rectMesh{};        // UV-mapped mesh for card sprites (shared across all cards)

    // Solid-colour panel meshes
    AEGfxVertexList* bag{};             // bag panel background
    AEGfxVertexList* shop{};            // shop panel background
    AEGfxVertexList* desc{};            // description panel background
    AEGfxVertexList* cardSlots{};       // active-card slots background
    AEGfxVertexList* trash{};           // trash zone background

    // Panel fill colours (ARGB)
    u32 cbag{ 0xFFC77014 };
    u32 cshop{ 0xFFcccc00 };
    u32 cdesc{ 0xFF000000 };
    u32 ccardSlots{ 0xFFcccc00 };
    u32 ctrash{ 0xFFe62e00 };

    GfxText cardCount{ "", 0.5f };  // reused for active and inventory card-count labels

    // Panel textures
    AEGfxTexture* pBgTex = nullptr;  // full-screen background
    AEGfxTexture* pShopTex = nullptr;  // shoporange.png — bag panel
    AEGfxTexture* pSlotsTex = nullptr;  // shopyellow.png — shop + active slots
    AEGfxTexture* pTrashTex = nullptr;  // trash.png — trash zone
    AEGfxTexture* pBtnNormalTex = nullptr;  // button idle state
    AEGfxTexture* pBtnHoverTex = nullptr;  // button hovered state

    AEGfxVertexList* pBtnMesh = nullptr;  // UV quad for buttons
    AEGfxVertexList* pPanelMesh = nullptr;  // shared UV quad for textured panels

    // -------------------------------------------------------------------------
    // SPRITESHEET
    // -------------------------------------------------------------------------

    AEGfxTexture* cardSpriteSheet{ nullptr };  // cards.png — all card art packed into one sheet
    const int NUM_COLS{ 4 };                   // columns in the spritesheet grid
    const int NUM_ROWS{ 7 };                   // rows in the spritesheet grid

    // -------------------------------------------------------------------------
    // CONSTANTS
    // -------------------------------------------------------------------------

    // ---- Card render scales per deck ----
    f32 CARD_SHOP_SCALE{ 7 };  // shop cards render larger to draw attention
    f32 ACTIVE_CARD_SCALE{ 3 };  // active-slot cards
    f32 INVENTORY_CARD_SCALE{ 3 };  // bag cards

    // ---- Base slot counts (before passive card bonuses) ----
    const int base_shopCards{ 3 };   // cards offered in the shop each round
    const int base_activeCards{ 5 };   // max cards in the active deck
    const int base_inventoryCards{ 15 };  // max cards in the bag
    const int base_buyable{ 1 };   // purchases allowed per visit
    const int base_rolls{ 0 };   // rerolls allowed per visit

    // ---- Hard caps on slot counts (cannot be exceeded by card effects) ----
    const int max_shopCards{ 7 };
    const int max_activeCards{ 10 };
    const int max_inventoryCards{ 15 };
    const int buyable_max{ 3 };

    // ---- Runtime slot counts (modified by passive card effects each round) ----
    int num_shopCards{ 3 };
    int num_activeCards{ 5 };
    int num_inventoryCards{ 15 };

    // ---- Runtime purchase / reroll budget ----
    int num_buyable{ 1 };
    int num_rolls{ 0 };
    int buyable_left{ 0 };  // purchases remaining this visit
    int rolls_left{ 0 };  // rerolls remaining this visit

    // ---- Rarity weights (must sum to 100) ----
    const float RARITY_WEIGHTS[] = {
        50.0f,  // COMMON
        30.0f,  // RARE
        15.0f,  // EPIC
         5.0f,  // UNIQUE
    };

    // -------------------------------------------------------------------------
    // DECK REFERENCES & SELECTION STATE
    // -------------------------------------------------------------------------

    // Convenience references into allCards[] to avoid magic indices elsewhere
    std::vector<Card>& shopCards = allCards[0];  // cards currently offered in the shop
    std::vector<Card>& activeCards = allCards[1];  // cards in the active deck
    std::vector<Card>& inventoryCards = allCards[2];  // cards stored in the bag

    Card* pSelectedCard{ nullptr };  // card currently held by the player
    Card* pHoveredCard{ nullptr };  // card the cursor is currently over (drives description display)

    // -------------------------------------------------------------------------
    // UI DATA
    // -------------------------------------------------------------------------

    // Button list: { position, size, mesh (assigned in Init), nextGS }
    std::vector<GfxButton> shopButtons{
        {{-700, -230}, {100, 100}, nullptr, GS_GAME}  // Next Round button
    };

    // Static text labels rendered every frame
    std::vector<GfxText> shopTexts{
        {"BAG",         0.5f, 0, 0, 0, 255, { 575, 370}},
        {"ACTIVE CARDS",0.5f, 0, 0, 0, 255, {-125,-230}},
        {"TRASH",       0.5f, 0, 0, 0, 255, {-700,-300}},
        {"NEXT\nROUND", 0.35f,0, 0, 0, 255, {-700,-220}}
    };

    // -------------------------------------------------------------------------
    // LAYOUT HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Populate the shop card array with freshly generated cards.
    void initCardShop(std::vector<Card>& cardShop) {
        Card card{};
        for (int i{ 0 }; i < num_shopCards; ++i) {
            Cards::generateCard(card);      // randomise card stats and art
            card.from = DECK::SHOP;
            cardShop.push_back(card);
        }
    }

    // ~ Brief: Distribute cards evenly along the X axis within [start, end],
    //          capped at maxGap pixels between centres. Single-card arrays are centred.
    void computeXCardPositions(std::vector<Card>& arr, f32 start, f32 end,
        f32 y, f32 scale = 10, f32 maxGap = 100.f) {
        // Edge case: nothing to position
        if (arr.empty()) return;

        int displayCards = static_cast<int>(arr.size());

        // Inset start and end by half a card width so cards don't clip the panel edges
        start += (arr[0].size.x / 2) * scale;
        end -= (arr[0].size.x / 2) * scale;
        f32 totalRange = end - start;

        f32 contentWidth = (displayCards - 1) * maxGap;

        // If natural spacing is narrower than the panel, centre the group inside it
        if (contentWidth < totalRange && displayCards > 1) {
            f32 offset = (totalRange - contentWidth) / 2.0f;
            start += offset;
            end -= offset;
        }

        f32 range = end - start;

        for (int i{ 0 }; i < displayCards; ++i) {
            // Edge case: centre a lone card
            if (arr.size() <= 1) {
                arr[i].pos.x = start + (0.5f * range);
                arr[i].pos.y = y;
                arr[i].homepos = arr[i].pos;
                continue;
            }

            // Normalise index to [0, 1] then map into world range
            f32 normalized = static_cast<f32>(i) / (displayCards - 1);
            arr[i].pos.x = start + (normalized * range);
            arr[i].pos.y = y;
            arr[i].homepos = arr[i].pos;
        }
    }

    // ~ Brief: Assign fixed grid positions to inventory (bag) cards.
    //          The grid is 3 columns × 5 rows; any cards beyond 15 are ignored.
    void computeBagCardPositions() {
        // Hardcoded pixel positions for the 3×5 bag grid
        std::vector<float> xpos{ 487.5f, 575.f, 662.5f };
        std::vector<float> ypos{ 287.5f, 145.f, 2.5f, -140.f, -282.5f };

        for (int i = 0; i < (int)inventoryCards.size(); ++i) {
            int row = i / 3;
            int col = i % 3;

            // Stop if we've run out of defined grid rows
            if (row >= (int)ypos.size()) break;

            inventoryCards[i].homepos = { xpos[col], ypos[row] };
            inventoryCards[i].pos = inventoryCards[i].homepos;
        }
    }

    // ~ Brief: Recalculate and cache the home position and bounding box
    //          for every card in all three decks.
    void computeCardHomePos() {
        if (!activeCards.empty()) {
            computeXCardPositions(activeCards, -550.f, 300.f, -300.f, ACTIVE_CARD_SCALE, 150.f);
            for (Card& card : activeCards) {
                card.homepos = card.pos;
                Comp::computeBoundingBox(card.boundingBox, card.homepos, card.size, ACTIVE_CARD_SCALE * 10);
            }
        }
        if (!shopCards.empty()) {
            computeXCardPositions(shopCards, -700.f, 300.f, 20.f, CARD_SHOP_SCALE, 300.f);
            for (Card& card : shopCards) {
                card.homepos = card.pos;
                Comp::computeBoundingBox(card.boundingBox, card.homepos, card.size, CARD_SHOP_SCALE * 10);
            }
        }
        if (!inventoryCards.empty()) {
            computeBagCardPositions();
            for (Card& card : inventoryCards) {
                card.homepos = card.pos;
                Comp::computeBoundingBox(card.boundingBox, card.homepos, card.size, INVENTORY_CARD_SCALE * 10);
            }
        }
    }

}  // namespace

// =============================================================================
// LOAD / UNLOAD
// =============================================================================

// ~ Brief: Load all textures and build meshes needed for the card shop screen.
void LoadCardShop() {
    // Card spritesheet
    cardSpriteSheet = AEGfxTextureLoad("Assets/cards.png");
    //if (!cardSpriteSheet) std::cout << "[ ERROR ] cards.png failed to load!\n";

    // Panel and button textures
    pBgTex = AEGfxTextureLoad("./Assets/sandbg.png");
    pShopTex = AEGfxTextureLoad("./Assets/shoporange.png");
    pSlotsTex = AEGfxTextureLoad("./Assets/shopyellow.png");
    pTrashTex = AEGfxTextureLoad("./Assets/trash.png");
    pBtnNormalTex = AEGfxTextureLoad("./Assets/mainmenubutton1.png");
    pBtnHoverTex = AEGfxTextureLoad("./Assets/mainmenubutton2.png");

    // Shared UV quad used for all textured panels
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pPanelMesh = AEGfxMeshEnd();

    // UV quad for buttons (identical layout to panel mesh)
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pBtnMesh = AEGfxMeshEnd();

    PauseScreen::LoadPause();
}

// ~ Brief: Free all textures and meshes owned by the card shop screen to prevent memory leaks.
void UnloadCardShop() {
    // Spritesheet (no null-check — guaranteed loaded or an error was already printed)
    AEGfxTextureUnload(cardSpriteSheet);

    // Panel textures
    if (pBgTex) { AEGfxTextureUnload(pBgTex);        pBgTex = nullptr; }
    if (pShopTex) { AEGfxTextureUnload(pShopTex);      pShopTex = nullptr; }
    if (pSlotsTex) { AEGfxTextureUnload(pSlotsTex);     pSlotsTex = nullptr; }
    if (pBtnNormalTex) { AEGfxTextureUnload(pBtnNormalTex); pBtnNormalTex = nullptr; }
    if (pBtnHoverTex) { AEGfxTextureUnload(pBtnHoverTex);  pBtnHoverTex = nullptr; }
    if (pTrashTex) { AEGfxTextureUnload(pTrashTex);     pTrashTex = nullptr; }
    if (pPanelMesh) { AEGfxMeshFree(pPanelMesh);         pPanelMesh = nullptr; }

    // Solid-colour panel meshes
    AEGfxMeshFree(rectMesh);
    AEGfxMeshFree(bag);
    AEGfxMeshFree(shop);
    AEGfxMeshFree(desc);
    AEGfxMeshFree(cardSlots);
    AEGfxMeshFree(trash);
    AEGfxMeshFree(pBtnMesh);
}

// =============================================================================
// INIT / FREE
// =============================================================================

// ~ Brief: Build all panel meshes, reserve deck memory, populate the shop,
//          compute card home positions, and reset per-round budgets.
void InitializeCardShop() {
    // Card spritesheet UV tile size
    f32 uSize = 1.0f / NUM_COLS;
    f32 vSize = 1.0f / NUM_ROWS;
    rectMesh = Gfx::createRectMesh(0xFFFFFFFF, 0.f, 0.f, uSize, vSize);

    // Solid-colour panel meshes
    bag = Gfx::createRectMesh(cbag);
    shop = Gfx::createRectMesh(cshop);
    desc = Gfx::createRectMesh(cdesc);
    cardSlots = Gfx::createRectMesh(ccardSlots);
    trash = Gfx::createRectMesh(ctrash);

    // [ DO NOT TOUCH ] Reserve capacity to avoid reallocation invalidating Card pointers
    shopCards.reserve(5);
    activeCards.reserve(10);
    inventoryCards.reserve(15);

    // Populate shop only if entering fresh (not returning from pause)
    if (shopCards.empty()) initCardShop(shopCards);
    computeCardHomePos();

    // Reset per-round purchase and reroll budgets
    buyable_left = num_buyable;
    rolls_left = num_rolls;

    Cards::computeCardEffects();
    SFX::playBGM();
}

// ~ Brief: Clear shop cards, reset selection pointers, and turn off the tutorial.
//          Stat modifier console dump is intentionally commented out; remove if no longer needed.
void FreeCardShop() {
    PauseScreen::FreePause();

    shopCards.clear();  // discard unsold shop cards for next round

    // Stat calculations kept for potential future debug output — suppress unused warnings
    f32 hp = calculate_max_stats(0); hp = hp;
    f32 dmg = calculate_max_stats(1); dmg = dmg;
    f32 speed = calculate_max_stats(2); speed = speed;
    f32 fire_rate = calculate_max_stats(3); fire_rate = fire_rate;
    f32 xp_mult = calculate_max_stats(4); xp_mult = xp_mult;

    // Disable tutorial after the first shop visit
    tutorialOn = false;

    // Clear dangling pointers — no card is selected or hovered between states
    pSelectedCard = nullptr;
    pHoveredCard = nullptr;
}

// =============================================================================
// ANONYMOUS NAMESPACE — file-private helpers (Update)
// =============================================================================

namespace {

    // -------------------------------------------------------------------------
    // DECK MANAGEMENT HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Remove a card from its source deck by matching home position (unique per card).
    //          Also decrements buyable_left when a shop card is purchased.
    void removeFromSource(Card& card) {
        // Default to shop deck; override for active and bag decks
        std::vector<Card>* sourceDeck = &shopCards;
        switch (card.from) {
        case DECK::ACTIVE: sourceDeck = &activeCards;    break;
        case DECK::BAG:    sourceDeck = &inventoryCards; break;
        }

        // Buying from the shop costs one purchase token
        if (card.from == DECK::SHOP) --buyable_left;

        // Find and erase by home position (home positions are unique after computeCardHomePos)
        for (size_t i = 0; i < sourceDeck->size(); ++i) {
            if (Comp::AEVec2Equal((*sourceDeck)[i].homepos, card.homepos)) {
                sourceDeck->erase(sourceDeck->begin() + i);
                break;
            }
        }
    }

    // ~ Brief: Resolve where a released card should land (bag, active deck, or trash).
    //          Cards dropped outside any valid zone snap back to their home position.
    void handleCardDrop() {
        Card& card = *pSelectedCard;

        // ---- Drop into bag ----
        if (Comp::collisionPointRect(card.pos, { 575, 0 }, { 350, 800 })) {
            // Reject if already in the bag or the bag is full
            if ((card.from == DECK::BAG) || ((int)inventoryCards.size() >= num_inventoryCards)) return;

            pSelectedCard = nullptr;
            Card newCard = card;
            newCard.from = DECK::BAG;
            inventoryCards.push_back(newCard);
            removeFromSource(card);
            computeCardHomePos();
            SFX::playSFX(SFX_UI_CARD_PLACE);
        }
        // ---- Drop into active deck ----
        else if (Comp::collisionPointRect(card.pos, { -125, -300 }, { 950, 200 })) {
            // Reject if already active, active deck is full, or the card has no active effect
            if ((card.from == DECK::ACTIVE) || ((int)activeCards.size() >= num_activeCards)) return;
            if ((card.info.active.empty()) && (!card.info.passive.empty())) return;

            pSelectedCard = nullptr;
            Card newCard = card;
            newCard.from = DECK::ACTIVE;
            activeCards.push_back(newCard);
            removeFromSource(card);
            computeCardHomePos();
            SFX::playSFX(SFX_UI_CARD_PLACE);
        }
        // ---- Drop into trash ----
        else if (Comp::collisionPointRect(card.pos, { -700, -370 }, { 100, 100 })) {
            // Cannot trash shop cards — player hasn't paid for them yet
            if (card.from == DECK::SHOP) return;

            pSelectedCard = nullptr;
            removeFromSource(card);
            computeCardHomePos();
            SFX::playSFX(SFX_UI_CARD_TRASH);
        }
    }

    // ~ Brief: While the player holds left-click, move the selected card with the cursor.
    //          On release, attempt a drop and snap the card back to its home position if the drop fails.
    void updateCardPosition() {
        Card& card = *pSelectedCard;

        // Follow the cursor delta each frame
        AEVec2 deltacursorpos{};
        Comp::getDeltaCursorPos(deltacursorpos);
        card.pos.x += deltacursorpos.x;
        card.pos.y -= deltacursorpos.y;

        // Left-click released — attempt drop, then reset regardless of outcome
        if (!AEInputCheckCurr(AEVK_LBUTTON)) {
            handleCardDrop();
            card.pos = card.homepos;   // snap back if drop was rejected
            pSelectedCard = nullptr;
            Cards::computeCardEffects();    // recompute modifiers after any deck change
        }
    }

    // ~ Brief: Hit-test the cursor against every card in all decks.
    //          Sets pHoveredCard for description display and pSelectedCard on click.
    //          Shop cards are only interactive while the player has purchases remaining.
    void checkCardCollision() {
        AEVec2 cursorpos{};
        Comp::getCursorPos(cursorpos);

        // Check active deck first (drawn on top, so hit priority matches visual order)
        for (Card& card : activeCards) {
            bool hovered = Comp::collisionPointRect(cursorpos, card.boundingBox);
            if (hovered) {
                if (!card.hovered) SFX::playSFX(SFX_UI_BUTTON_HOVER);
                pHoveredCard = &card;
                if (AEInputCheckTriggered(AEVK_LBUTTON)) {
                    SFX::playSFX(SFX_UI_CARD_PICKUP);
                    pSelectedCard = &card;
                }
            }
            card.hovered = hovered;
            if (hovered) return;  // early-out: only one card can be hovered at a time
        }

        // Check inventory (bag) deck
        for (Card& card : inventoryCards) {
            bool hovered = Comp::collisionPointRect(cursorpos, card.boundingBox);
            if (hovered) {
                if (!card.hovered) SFX::playSFX(SFX_UI_BUTTON_HOVER);
                pHoveredCard = &card;
                if (AEInputCheckTriggered(AEVK_LBUTTON)) {
                    SFX::playSFX(SFX_UI_CARD_PICKUP);
                    pSelectedCard = &card;
                }
            }
            card.hovered = hovered;
            if (hovered) return;
        }

        // Skip shop cards if the player has no purchases left
        if (buyable_left <= 0) return;

        // Check shop deck
        for (Card& card : shopCards) {
            bool hovered = Comp::collisionPointRect(cursorpos, card.boundingBox);
            if (hovered) {
                if (!card.hovered) SFX::playSFX(SFX_UI_BUTTON_HOVER);
                pHoveredCard = &card;
                if (AEInputCheckTriggered(AEVK_LBUTTON)) {
                    SFX::playSFX(SFX_UI_CARD_PICKUP);
                    pSelectedCard = &card;
                }
            }
            card.hovered = hovered;
            if (hovered) return;
        }

        // No card was hit this frame
        pHoveredCard = nullptr;
    }

    // ~ Brief: If the player has rerolls remaining and clicks the description panel,
    //          regenerate all shop cards and decrement rolls_left.
    void checkReroll() {
        if (rolls_left <= 0) return;
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);

        // Description panel doubles as the reroll click target
        if (Comp::collisionPointRect(mousepos, { -200, 325 }, { 1100, 150 })) {
            SFX::playSFX(SFX_UI_BUTTON_SELECT);
            shopCards.clear();
            initCardShop(shopCards);
            computeCardHomePos();
            --rolls_left;
        }
    }

}  // namespace

// =============================================================================
// UPDATE
// =============================================================================

// ~ Brief: Handle pause, Next Round button clicks, card drag-and-drop, and reroll input.
void UpdateCardShop() {
    PauseScreen::UpdatePause();

    // Check Next Round button
    if (AEInputCheckTriggered(AEVK_LBUTTON)) {
        SFX::playSFX(SFX_UI_BUTTON_SELECT);
        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);
        for (GfxButton& btn : shopButtons) {
            if (Comp::collisionPointRect(mousepos, btn.pos, btn.size)) {
                GS_next = btn.nextGS;
                break;
            }
        }
    }

    if (pSelectedCard) {
        // A card is held — update its position and use it as the description source
        updateCardPosition();
        pHoveredCard = pSelectedCard;
    }
    else {
        // No card held — check hover / pick-up collisions and reroll
        checkCardCollision();
        if (rolls_left > 0) checkReroll();
    }
}

// =============================================================================
// ANONYMOUS NAMESPACE — file-private helpers (Draw)
// =============================================================================

namespace {

    // -------------------------------------------------------------------------
    // DRAW HELPERS
    // -------------------------------------------------------------------------

    // ~ Brief: Draw all background and panel textures that make up the shop layout.
    void drawShopUI() {
        // Full-screen background
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(pBgTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        Gfx::printMesh(pPanelMesh, { 0.f, 0.f },
            { (float)AEGfxGetWindowWidth(), (float)AEGfxGetWindowHeight() }, 0.f, { 0.f, 0.f }, true);

        // Textured panels — bag, shop, active slots, trash
        AEGfxTextureSet(pShopTex, 0, 0);
        Gfx::printMesh(pPanelMesh, { 575,  0 }, { 350, 800 }, 0.f, { 0.f, 0.f }, true);  // bag

        AEGfxTextureSet(pSlotsTex, 0, 0);
        Gfx::printMesh(pPanelMesh, { -200,  40 }, { 1100, 380 }, 0.f, { 0.f, 0.f }, true);  // shop

        AEGfxTextureSet(pSlotsTex, 0, 0);
        Gfx::printMesh(pPanelMesh, { -125, -300 }, { 950, 200 }, 0.f, { 0.f, 0.f }, true);  // active slots

        AEGfxTextureSet(pTrashTex, 0, 0);
        Gfx::printMesh(pPanelMesh, { -700, -370 }, { 100, 100 }, 0.f, { 0.f, 0.f }, true);  // trash

        // Solid-colour description panel
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetTransparency(1.f);
        Gfx::printMesh(desc, { -200, 325 }, { 1100, 150 });
    }

    // ~ Brief: Calculate the UV bounding box for a card in the spritesheet and
    //          return a new mesh with those coordinates. Used for debugging UV layout.
    AEGfxVertexList* createCardMesh(const Card& card) {
        f32 uMin = card.info.col * (1.0f / NUM_COLS);
        f32 vMin = card.info.row * (1.0f / NUM_ROWS);
        f32 uMax = uMin + (1.0f / NUM_COLS);
        f32 vMax = vMin + (1.0f / NUM_ROWS);

        /*std::cout << "Card: " << card.info.ID
            << " row:" << card.info.row
            << " col:" << card.info.col
            << " UV: (" << uMin << "," << vMin << ") -> (" << uMax << "," << vMax << ")\n";*/

        return Gfx::createRectMesh(0xFFFFFFFF, uMin, vMin, uMax, vMax);
    }

    // ~ Brief: Draw every card in all three decks using the shared spritesheet mesh,
    //          offsetting the UV each draw call to select the correct card art.
    void drawCards() {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

        // Draw shop, active, and inventory cards — UV offset selects art from spritesheet
        for (Card& indivCard : shopCards) {
            f32 uOffset = indivCard.info.col * (1.0f / NUM_COLS);
            f32 vOffset = indivCard.info.row * (1.0f / NUM_ROWS);
            AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
            Gfx::printMesh(rectMesh, indivCard, CARD_SHOP_SCALE);
        }
        for (Card& indivCard : activeCards) {
            f32 uOffset = indivCard.info.col * (1.0f / NUM_COLS);
            f32 vOffset = indivCard.info.row * (1.0f / NUM_ROWS);
            AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
            Gfx::printMesh(rectMesh, indivCard, ACTIVE_CARD_SCALE);
        }
        for (Card& indivCard : inventoryCards) {
            f32 uOffset = indivCard.info.col * (1.0f / NUM_COLS);
            f32 vOffset = indivCard.info.row * (1.0f / NUM_ROWS);
            AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
            Gfx::printMesh(rectMesh, indivCard, INVENTORY_CARD_SCALE);
        }

        // Restore colour render mode
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetTransparency(1.f);
    }

    // ~ Brief: Draw a textured button, switching to the hover texture when the cursor is over it.
    //          Triggers a hover SFX on the first frame the cursor enters the button region.
    void drawTexturedButton(GfxButton& btn, AEVec2& mousepos) {
        bool hovered = Comp::collisionPointRect(mousepos, btn.pos, btn.size);

        // Play hover SFX on the leading edge of a hover (first frame only)
        if (hovered && !btn.hovered)
            SFX::playSFX(SFX_UI_BUTTON_HOVER);

        btn.hovered = hovered;

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(hovered ? pBtnHoverTex : pBtnNormalTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        {
            // Anon scope — matrix locals (scale, trans, final) are discarded after the draw call
            AEMtx33 scale, trans, final;
            AEMtx33Scale(&scale, btn.size.x, btn.size.y);
            AEMtx33Trans(&trans, btn.pos.x, btn.pos.y);
            AEMtx33Concat(&final, &trans, &scale);
            AEGfxSetTransform(final.m);
            AEGfxMeshDraw(pBtnMesh, AE_GFX_MDM_TRIANGLES);
        }
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);  // restore colour mode
    }

    // ~ Brief: Draw all static buttons and text labels, plus dynamic card-count labels
    //          for the active deck and inventory. Hides the "SHOP" label when no purchases remain.
    void drawTexts() {
        // Static buttons
        AEVec2 mousepos{};
        Comp::getCursorPos(mousepos);
        for (GfxButton& btn : shopButtons)
            drawTexturedButton(btn, mousepos);

        // Static text labels
        for (GfxText& text : shopTexts)
            Gfx::printMultiline(text, boldPixels);

        // Hide the "SHOP" label once all purchases are spent to avoid confusion
        GfxText shoptxt{ "SHOP", 0.8f, 0, 0, 0, 255, { -200, 190 } };
        if (buyable_left > 0)
            Gfx::printText(shoptxt, boldPixels);

        // Dynamic card-count labels (current / max) for active deck and inventory
        cardCount.pos = { -125, -375 };
        cardCount.text = std::to_string(activeCards.size()) + "/" + std::to_string(num_activeCards);
        Gfx::printText(cardCount, boldPixels);

        cardCount.pos = { 575, -375 };
        cardCount.text = std::to_string(inventoryCards.size()) + "/" + std::to_string(num_inventoryCards);
        Gfx::printText(cardCount, boldPixels);
    }

    // ~ Brief: While a card is being dragged, highlight the bag and active-deck panels
    //          and show colour-coded prompts indicating whether the card is valid in each zone.
    void drawPrompts() {
        Card& card = *pSelectedCard;

        {
            // Active deck prompt — green if the card has an active effect, red otherwise
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxTextureSet(pSlotsTex, 0, 0);
            AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
            AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetTransparency(1.f);
            Gfx::printMesh(pPanelMesh, { -125, -300 }, { 950, 200 }, 0.f, { 0.f, 0.f }, true);
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);

            GfxText text{ "", 0.6f };
            if (!card.info.active.empty()) {
                text.text = "Place card HERE to activate card effects";
                text.r = 0.f; text.g = 200.f; text.b = 0.f; text.a = 255.f;   // green = valid
            }
            else {
                text.text = "Card has no effect if placed here";
                text.r = 255.f; text.g = 0.f; text.b = 0.f; text.a = 255.f;   // red = invalid
            }
            text.pos = { -125, -300 };
            Gfx::printMultiline(text, boldPixels);
        }
        {
            // Bag prompt — green if the card has a passive effect, red otherwise
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxTextureSet(pShopTex, 0, 0);
            AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
            AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetTransparency(1.f);
            Gfx::printMesh(pPanelMesh, { 575, 0 }, { 350, 800 }, 0.f, { 0.f, 0.f }, true);
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);

            GfxText text{ "", 0.6f };
            if (!card.info.passive.empty()) {
                text.text = "Place card\nHERE to\nactivate card\neffects";
                text.r = 0.f; text.g = 200.f; text.b = 0.f; text.a = 255.f;   // green = valid
            }
            else {
                text.text = "Card has no\neffect if placed\nhere";
                text.r = 255.f; text.g = 0.f; text.b = 0.f; text.a = 255.f;   // red = invalid
            }
            text.pos = { 575, 30 };
            Gfx::printMultiline(text, boldPixels);
        }

        // Drop-zone confirmation labels shown when the card overlaps the zone
        if (Comp::collisionPointRect(card.pos, { 575, 0 }, { 350, 800 })) {
            if (card.from == DECK::BAG) return;  // already in bag — no label needed
            GfxText text{ "Add to Bag", 0.8f };
            text.pos = { 575, 120 };
            Gfx::printText(text, boldPixels);
        }
        else if (Comp::collisionPointRect(card.pos, { -125, -300 }, { 950, 200 })) {
            if (card.from == DECK::ACTIVE) return;  // already active — no label needed
            GfxText text{ "Add to Active Cards", 0.6f };
            text.pos = { -125, -260 };
            Gfx::printText(text, boldPixels);
        }
    }

    // ~ Brief: Draw the currently dragged card at its cursor-following position,
    //          always on top of all other card draws.
    void drawSelectedCard() {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

        // Scale matches whichever deck the card originated from
        Card& card = *pSelectedCard;
        f32 scale{};
        switch (card.from) {
        case DECK::SHOP:   scale = CARD_SHOP_SCALE;      break;
        case DECK::ACTIVE: scale = ACTIVE_CARD_SCALE;    break;
        case DECK::BAG:    scale = INVENTORY_CARD_SCALE; break;
        }

        // Sample the correct sprite from the shared spritesheet
        f32 uOffset = card.info.col * (1.0f / NUM_COLS);
        f32 vOffset = card.info.row * (1.0f / NUM_ROWS);
        AEGfxTextureSet(cardSpriteSheet, uOffset, vOffset);
        Gfx::printMesh(rectMesh, card, scale);

        // Restore colour mode
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetTransparency(1.f);
    }

    // ~ Brief: Print the hovered card's active and passive effect descriptions in the
    //          description panel. If no card is hovered, show a reroll prompt or a
    //          default "select a card" message depending on whether rolls remain.
    void drawCardDescription(bool printDefault = false) {
        if (printDefault) {
            GfxText text{ "", 0.f, 255, 255, 255, 255 };
            text.pos = { -200, 325 };
            if (rolls_left <= 0) {
                // No rerolls — prompt player to select a card
                text.scale = 1.f;
                text.text = "Select a card to continue!";
            }
            else {
                // Rerolls available — invite the player to click the description panel
                text.scale = 0.8f;
                text.text = "Click here to re-roll cards! (";
                text.text += std::to_string(rolls_left) + " left)";
            }
            Gfx::printText(text, boldPixels);
            return;
        }

        // Concatenate active and passive effect descriptions for the hovered card
        GfxText cardDesc{ "", 0.5f, 255, 255, 255, 255 };
        cardDesc.pos = { -200, 360 };

        Card& card = *pHoveredCard;
        if (!card.info.active.empty()) {
            for (CardEffect& effect : card.info.active)
                cardDesc.text += effect.desc + '\n';
        }
        if (!card.info.passive.empty()) {
            cardDesc.text += "[ PASSIVE ABILITY ]\n";
            for (CardEffect& effect : card.info.passive)
                cardDesc.text += effect.desc + '\n';
        }

        Gfx::printMultiline(cardDesc, boldPixels);
    }

    // ~ Brief: Cover the shop panel with a textured overlay once the player has
    //          spent all purchases, and display the upcoming boss countdown.
    void blockShop() {
        // Re-draw the shop panel texture on top to visually hide the cards
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(pSlotsTex, 0, 0);
        AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.f);
        Gfx::printMesh(pPanelMesh, { -200, 40 }, { 1100, 380 }, 0.f, { 0.f, 0.f }, true);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

        // Build informational text: current wave, call-to-action, and boss countdown
        GfxText text{ "", 0.5f };
        text.pos = { -200, 180 };
        text.text += "Current Wave: " + std::to_string(currentWave) + "\n\n\n\n\n";
        text.text += "CLICK NEXT ROUND TO CONTINUE\n\n\n\n\n";

        int nextBoss{ 6 - (currentWave % 5) };
        if (nextBoss == 6) {
            // Boss spawns this wave
            text.text += "BOSS SPAWNING THIS WAVE\nGood Luck :)";
        }
        else {
            // Pluralise "WAVE" correctly based on count
            text.text += std::to_string(nextBoss)
                + " MORE WAVE" + (nextBoss > 1 ? "S" : "")
                + " TO BOSS SPAWN";
        }
        Gfx::printMultiline(text, boldPixels);
    }

}  // namespace

// =============================================================================
// DRAW
// =============================================================================

// ~ Brief: Render the full card shop screen in layer order:
//          UI panels, cards, texts, card description, tutorial,
//          shop blocker (if no purchases remain), selected card and prompts (always on top).
void DrawCardShop() {
    // Draw background panels and layout
    drawShopUI();

    // Draw all cards
    drawCards();

    // Draw buttons and text labels
    drawTexts();
    PauseScreen::DrawPauseButton();

    // Draw card description (or default prompt if nothing is hovered)
    drawCardDescription(pHoveredCard == nullptr);

    // Draw tutorial overlay on wave 2 if tutorial is still active
    if (currentWave == 2 && tutorialOn) printtutorial();

    // Block shop access once all purchases are spent
    if (buyable_left <= 0) blockShop();

    // Draw drag prompts and selected card always on top
    if (pSelectedCard) {
        drawPrompts();       // highlight valid drop zones
        drawTexts();         // keep text readable over the dragged card
        drawSelectedCard();  // selected card renders above everything else
    }

    PauseScreen::DrawPause();
}

// =============================================================================
// CARDS NAMESPACE — JSON loading, card generation, and modifier computation
// =============================================================================

namespace Cards {

    // ~ Brief: Parse a JSON array of card effects into a vector of CardEffect structs.
    //          Missing fields are defaulted to empty string / 0.0f rather than erroring.
    std::vector<CardEffect> parseEffects(const rapidjson::Value& effectArray) {
        std::vector<CardEffect> effects;

        for (const auto& e : effectArray.GetArray()) {
            CardEffect effect;
            effect.id = e.HasMember("id") ? e["id"].GetString() : "";
            effect.type = e.HasMember("type") ? e["type"].GetString() : "";
            effect.desc = e.HasMember("desc") ? e["desc"].GetString() : "";
            effect.valuetype = e.HasMember("valuetype") ? e["valuetype"].GetString() : "";
            effect.value = e.HasMember("value") ? e["value"].GetFloat() : 0.f;
            effects.push_back(effect);
        }

        return effects;
    }

    // ~ Brief: Parse cards.json and populate cardPool, bucketed by rarity.
    //          Logs errors to stdout if the file cannot be opened or the JSON is malformed.
    //          Reference: https://rapidjson.org/md_doc_stream.html#FileStreams
    void Load_Cards(std::string const& filename) {
        // Clear existing pool before repopulating
        for (int i{}; i < NUM_OF_RARITIES; i++)
            cardPool[i].clear();

        std::ifstream file{ filename };
        if (!file.is_open()) {
            //std::cout << "[ ERROR ] " << filename << " failed to open!!\n";
            return;
        }

        // Read file into string, then parse into a rapidjson Document
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();

        rapidjson::Document doc;
        rapidjson::ParseResult ok = doc.Parse(json.c_str());
        if (ok.IsError()) {
            /*std::cout << "[ ERROR ] JSON Parse Error: " << GetParseError_En(ok.Code())
                << " at offset " << ok.Offset() << '\n';*/
            return;
        }

        // Translate each JSON card object into a CardStats and push to the correct rarity bucket
        for (const auto& cardJson : doc["cards"].GetArray()) {
            CardStats card;
            card.ID = cardJson["id"].GetString();
            card.rarity = cardJson["rarity"].GetInt();
            card.row = cardJson["sprite_row"].GetInt();
            card.col = cardJson["sprite_col"].GetInt();
            card.active = parseEffects(cardJson["active"]);
            card.passive = parseEffects(cardJson["passive"]);
            ::cardPool[card.rarity].push_back(card);
        }
    }

    // ~ Brief: Roll a rarity index using weighted random selection.
    //          Weights are defined in RARITY_WEIGHTS[] and must sum to 100.
    int randomiseRarity() {
        float rng = AERandFloat() * 100.0f;
        float total = 0.0f;

        for (int i = 0; i < NUM_OF_RARITIES; i++) {
            total += RARITY_WEIGHTS[i];
            if (rng < total) return static_cast<Rarity>(i);
        }
        return COMMON;  // fallback — should never be reached if weights sum to 100
    }

    // ~ Brief: Assign a randomly generated card of a randomly selected rarity to card.
    //          Rerolls the rarity if the chosen bucket is empty. Clamps rarity to valid range.
    void generateCard(Card& card) {
        int cardRarity;

        // Reroll until a non-empty rarity bucket is found
        do {
            cardRarity = randomiseRarity();
        } while (::cardPool[cardRarity].empty());

        // Clamp rarity to valid index range
        if (cardRarity >= 4) cardRarity = 3;

        std::vector<CardStats>& rarityPool = ::cardPool[cardRarity];

        // Pick a random card from the rarity bucket
        int index = (int)(AERandFloat() * rarityPool.size());
        if (index == (int)rarityPool.size()) --index;  // clamp to last valid index

        card.info = ::cardPool[cardRarity][index];
    }

    // ~ Brief: Reset all card-driven stat modifiers and passive upgrade flags to their
    //          default values, ready for recomputation after a deck change.
    void resetModifiers() {
        // Flat bonuses reset to zero
        cardBaseMod.hp = 0.f;
        cardBaseMod.dmg = 0.f;
        cardBaseMod.fireRate = 0.f;
        cardBaseMod.moveSpeed = 0.f;
        cardBaseMod.xp = 0.f;

        // Multipliers reset to identity (1.0)
        cardMultMod.hp = 1.f;
        cardMultMod.dmg = 1.f;
        cardMultMod.fireRate = 1.f;
        cardMultMod.moveSpeed = 1.f;
        cardMultMod.xp = 1.f;

        // Passive slot counts and flags reset to base values
        upgradeFlag = UPGRADE_NONE;
        num_activeCards = base_activeCards;
        num_shopCards = base_shopCards;
        num_buyable = base_buyable;
        num_rolls = base_rolls;
    }

    // ~ Brief: Clear all card decks and reset all modifiers for a fresh game session.
    void resetCards() {
        if (!shopCards.empty())      shopCards.clear();
        if (!inventoryCards.empty()) inventoryCards.clear();
        if (!activeCards.empty())    activeCards.clear();
        resetModifiers();
    }

    // ~ Brief: Recompute all stat modifiers and passive upgrade flags from the current decks.
    //          Active cards contribute active effects; inventory cards contribute passive effects.
    //          Called whenever the deck composition changes (card placed, bought, or trashed).
    void computeCardEffects() {
        float oldMaxHp = get_max_hp();  // snapshot HP max before modifiers change
        resetModifiers();

        // ---- Active deck: apply active effects to stat modifiers ----
        for (Card& card : activeCards) {
            for (CardEffect& effect : card.info.active) {
                // Route to the correct modifier struct (additive vs multiplicative)
                PlayerStatsModifier& mod = (effect.valuetype == "mult") ? cardMultMod : cardBaseMod;

                if (effect.type == "HP")         mod.hp += effect.value;
                else if (effect.type == "DMG")        mod.dmg += effect.value;
                else if (effect.type == "FIRE_RATE")  mod.fireRate += effect.value;
                else if (effect.type == "MOVE_SPEED") mod.moveSpeed += effect.value;
                else if (effect.type == "XP")         mod.xp += effect.value;
            }
        }

        // Fire rate multiplier floor — prevent the fire rate from being reduced by more than 90%
        if (cardMultMod.fireRate <= 0.1f) cardMultMod.fireRate = 0.1f;

        // ---- Inventory deck: apply passive effects to slot counts and upgrade flags ----
        for (Card& card : inventoryCards) {
            for (CardEffect& effect : card.info.passive) {
                if (effect.id == "active_cards_up") num_activeCards++;
                else if (effect.id == "shop_cards_up")   num_shopCards++;
                else if (effect.id == "shop_buys_up")    num_buyable++;
                else if (effect.id == "rerolls_up") {
                    // Higher rarities grant more rerolls per card
                    if (card.info.rarity == COMMON) num_rolls += 1;
                    else if (card.info.rarity == RARE)   num_rolls += 2;
                    else if (card.info.rarity == EPIC)   num_rolls += 3;
                }
                // Flip the corresponding weapon upgrade bit
                else if (effect.id == "big_cannon")  upgradeFlag |= UPGRADE_BIG_CANNON;
                else if (effect.id == "cannon_180")  upgradeFlag |= UPGRADE_CANNON_180;
                else if (effect.id == "dual_cannon") upgradeFlag |= UPGRADE_DUAL_CANNON;
                else if (effect.id == "orbit")       upgradeFlag |= UPGRADE_ORBIT;
            }
        }

        // ---- Clamp slot counts to hard caps ----
        if (num_shopCards > max_shopCards)   num_shopCards = max_shopCards;
        if (num_activeCards > max_activeCards) num_activeCards = max_activeCards;
        if (num_buyable > buyable_max)     num_buyable = buyable_max;

        // Dual cannon is mutually exclusive with big cannon and cannon_180
        if (upgradeFlag & UPGRADE_DUAL_CANNON) {
            if (upgradeFlag & UPGRADE_BIG_CANNON) upgradeFlag &= ~UPGRADE_BIG_CANNON;
            if (upgradeFlag & UPGRADE_CANNON_180) upgradeFlag &= ~UPGRADE_CANNON_180;
        }

        // Propagate any HP max change to current HP (handled by DebugXP)
        UpdateCurrentHpAfterCards(oldMaxHp);
    }

    // ~ Brief: Search the full card pool for a card whose ID matches search,
    //          and assign its stats to card. Falls back to the first common card if not found.
    void GetCardByID(std::string& search, Card& card) {
        for (int i{}; i < NUM_OF_RARITIES; i++) {
            for (CardStats stat : cardPool[i]) {
                if (stat.ID == search) {
                    card.info = stat;
                    return;
                }
            }
        }
        // Fallback — assign first card in the common pool
        card.info = cardPool[0][0];
    }

}  // namespace Cards