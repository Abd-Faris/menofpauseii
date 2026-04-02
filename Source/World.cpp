// -----------------------------Gloomy's Revenge---------------------------- //
// File:	World.cpp
// Authors:	[Men of Pause II]
// Brief:	Implementation of grid-based world rendering, collisions, and decor.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// ANONYMOUS NAMESPACE 
// =============================================================================
namespace World {
    int mapGrid[WORLD_ROWS][WORLD_COLS];
    static AEGfxVertexList* pWallMesh = nullptr;
    static AEGfxTexture* pWallTex = nullptr;
    static AEGfxTexture* pGroundTex = nullptr;
    static AEGfxTexture* pTreeTex = nullptr;
    static AEGfxTexture* pOilTex = nullptr;
    static AEGfxTexture* pTwigTex = nullptr;

    // Hardcoded grid locations for large tree obstacles
    struct TreePos { int col, row; };
    const TreePos treePlacements[5] = {
        { 8,  5  },  // Q1: top-left
        { 36, 4  },  // Q2: top-right
        { 7,  20 },  // Q3: bottom-left
        { 37, 18 },  // Q4: bottom-right
        { 23, 15 },  // Middle
    };

    // Decorative objects (world positions)
    struct DecorObj {
        float x, y;
        float scale;
        float rotation;
    };

    const int NUM_OIL = 1;
    const int NUM_TWIG = 8;
    DecorObj oilSpills[NUM_OIL];
    DecorObj twigs[NUM_TWIG];

    bool decorInitialized = false;

    // =============================================================================
    // COORDINATE HELPERS
    // =============================================================================

    // ~ Brief: Converts grid col/row index to world position (centered on the tile)
    static AEVec2 GridToWorld(int col, int row) {
        AEVec2 pos;
        // Shift by half dimensions to center the 0,0 grid at the screen center
        pos.x = (col * TILE_SIZE) - HALF_WIDTH + (TILE_SIZE / 2.0f);
        pos.y = HALF_HEIGHT - (row * TILE_SIZE) - (TILE_SIZE / 2.0f);
        return pos;
    }

    // ~ Brief: Returns a random world position inside the map (avoids border and trees)
    static AEVec2 RandomInteriorPos() {
        // Pick a random spot that isn't the very edge of the map
        int col = 2 + (rand() % 31);
        int row = 2 + (rand() % 16);

        // Simple check to make sure we don't spawn decor exactly where a tree is
        for (int i = 0; i < 5; ++i) {
            if (col == treePlacements[i].col && row == treePlacements[i].row) {
                col = (col + 3) % 31 + 2;
            }
        }
        return GridToWorld(col, row);
    }

    // =============================================================================
    // LIFECYCLE FUNCTIONS
    // =============================================================================

    // ~ Brief: Load textures and create the shared unit quad mesh for tiles and decor
    void Load_World() {
        if (pWallTex != nullptr || pGroundTex != nullptr || pWallMesh != nullptr) return;
        pWallTex = AEGfxTextureLoad("./Assets/wall.png");
        pGroundTex = AEGfxTextureLoad("./Assets/sand.png");
        pTreeTex = AEGfxTextureLoad("./Assets/tree.png");
        pOilTex = AEGfxTextureLoad("./Assets/oilspill.png");
        pTwigTex = AEGfxTextureLoad("./Assets/twigs.png");

        // Build a standard 1x1 quad mesh for all world rendering
        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        pWallMesh = AEGfxMeshEnd();
    }

    // ~ Brief: Setup the grid map and randomize the placement of decorative objects
    void Init_World() {
        // Create the border walls
        for (int row = 0; row < WORLD_ROWS; row++) {
            for (int col = 0; col < WORLD_COLS; col++) {
                if (row == 0 || row == WORLD_ROWS - 1 || col == 0 || col == WORLD_COLS - 1)
                    mapGrid[row][col] = 1; // Wall
                else
                    mapGrid[row][col] = 0; // Empty
            }
        }

        // Add tree positions to the grid as wall obstacles
        for (int i = 0; i < 5; ++i) {
            mapGrid[treePlacements[i].row][treePlacements[i].col] = 1;
        }

        // Randomize decor locations only once to keep them consistent across frames
        if (!decorInitialized) {
            for (int i = 0; i < NUM_OIL; ++i) {
                AEVec2 p = RandomInteriorPos();
                oilSpills[i].x = p.x;
                oilSpills[i].y = p.y;
                oilSpills[i].scale = TILE_SIZE * 1.2f;
                oilSpills[i].rotation = (AERandFloat() * 360.0f);
            }
            for (int i = 0; i < NUM_TWIG; ++i) {
                AEVec2 p = RandomInteriorPos();
                twigs[i].x = p.x;
                twigs[i].y = p.y;
                twigs[i].scale = TILE_SIZE * 0.6f;
                twigs[i].rotation = (AERandFloat() * 360.0f);
            }
            decorInitialized = true;
        }
    }

    // =============================================================================
    // COLLISION LOGIC
    // =============================================================================

    // ~ Brief: Maps a world coordinate back to a grid cell to check if it's a wall
    bool isPointColliding(float worldX, float worldY) {
        int col = static_cast<int>((worldX + World::HALF_WIDTH) / World::TILE_SIZE);
        int row = static_cast<int>((World::HALF_HEIGHT - worldY) / World::TILE_SIZE);

        // Out of bounds is considered a collision
        if (col < 0 || col >= World::WORLD_COLS || row < 0 || row >= World::WORLD_ROWS) return true;
        return (World::mapGrid[row][col] == 1);
    }

    // ~ Brief: Samples multiple points around a circular radius to check for body collisions
    bool CheckBodyCollision(float x, float y, float playerScale) {
        float radius = playerScale * 0.8f;
        const int NUM_POINTS = 8;

        for (int i = 0; i < NUM_POINTS; i++) {
            float angle = (i / (float)NUM_POINTS) * TWO_PI;
            float worldX = x + cosf(angle) * radius;
            float worldY = y + sinf(angle) * radius;
            if (isPointColliding(worldX, worldY)) return true;
        }
        return false;
    }

    // ~ Brief: Comprehensive collision check covering both the player body and the tank barrel
    bool CheckCollision(float x, float y, float playerScale, float playerRotation) {
        if (CheckBodyCollision(x, y, playerScale)) return true;

        // Calculate specific points along the barrel to ensure it doesn't clip through walls
        float barrelLen = GameConfig::Tank::BARREL_LENGTH * (playerScale / GameConfig::Tank::SCALE);
        float halfBWidth = 11.0f;
        AEVec2 barrelPoints[4] = {
            { -halfBWidth, barrelLen        },
            {  halfBWidth, barrelLen        },
            { -halfBWidth, barrelLen * 0.5f },
            {  halfBWidth, barrelLen * 0.5f }
        };

        float cosA = cosf(playerRotation);
        float sinA = sinf(playerRotation);

        // Rotate barrel points based on current player heading
        for (int i = 0; i < 4; i++) {
            float worldX = x + (barrelPoints[i].x * cosA - barrelPoints[i].y * sinA);
            float worldY = y + (barrelPoints[i].x * sinA + barrelPoints[i].y * cosA);
            if (isPointColliding(worldX, worldY)) return true;
        }
        return false;
    }

    // =============================================================================
    // DRAWING LOGIC
    // =============================================================================

    // ~ Brief: Helper to draw a textured quad with specific scale and rotation
    static void DrawDecor(AEGfxTexture* tex, float x, float y, float scale, float rotDeg) {
        if (!tex || !pWallMesh) return;
        AEGfxTextureSet(tex, 0, 0);
        float rotRad = rotDeg * (PI / 180.0f);
        AEMtx33 s, r, t, final;
        AEMtx33Scale(&s, scale, scale);
        AEMtx33Rot(&r, rotRad);
        AEMtx33Trans(&t, x, y);

        // Combine transformations: Rotate -> Scale -> Translate
        AEMtx33Concat(&final, &r, &s);
        AEMtx33Concat(&final, &t, &final);
        AEGfxSetTransform(final.m);
        AEGfxMeshDraw(pWallMesh, AE_GFX_MDM_TRIANGLES);
    }

    // ~ Brief: Renders tiles, followed by floor decor, then wall/tree obstacles on top
    void Draw_World() {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

        // Loop through the grid to draw the base layer
        for (int row = 0; row < WORLD_ROWS; row++) {
            for (int col = 0; col < WORLD_COLS; col++) {

                // Check if this specific cell is designated for a tree
                bool isTree = false;
                for (int t = 0; t < 5; ++t) {
                    if (col == treePlacements[t].col && row == treePlacements[t].row) {
                        isTree = true; break;
                    }
                }

                // Pick texture based on grid type
                if (isTree) {
                    AEGfxTextureSet(pGroundTex, 0, 0); // Ground shows under the tree canopy
                }
                else if (mapGrid[row][col] == 1) {
                    AEGfxTextureSet(pWallTex, 0, 0);
                }
                else {
                    AEGfxTextureSet(pGroundTex, 0, 0);
                }

                AEVec2 position = GridToWorld(col, row);
                AEMtx33 scale, trans, finalTransform;
                AEMtx33Scale(&scale, TILE_SIZE, TILE_SIZE);
                AEMtx33Trans(&trans, position.x, position.y);
                AEMtx33Concat(&finalTransform, &trans, &scale);
                AEGfxSetTransform(finalTransform.m);
                AEGfxMeshDraw(pWallMesh, AE_GFX_MDM_TRIANGLES);
            }
        }

        // Draw environmental details (ordered for proper layering)
        for (int i = 0; i < NUM_OIL; ++i) {
            DrawDecor(pOilTex, oilSpills[i].x, oilSpills[i].y, oilSpills[i].scale, oilSpills[i].rotation);
        }

        for (int i = 0; i < NUM_TWIG; ++i) {
            DrawDecor(pTwigTex, twigs[i].x, twigs[i].y, twigs[i].scale, twigs[i].rotation);
        }

        // Trees are drawn last so their canopy overlaps other ground objects
        for (int i = 0; i < 5; ++i) {
            AEVec2 pos = GridToWorld(treePlacements[i].col, treePlacements[i].row);
            DrawDecor(pTreeTex, pos.x, pos.y, TILE_SIZE * 1.4f, 0.0f);
        }
    }

    // =============================================================================
    // PHYSICS RESOLUTION
    // =============================================================================

    // ~ Brief: Checks nearby wall tiles and pushes the coordinate out if it's too close
    void PushOutOfWalls(float& x, float& y, float radius) {
        int col = static_cast<int>((x + HALF_WIDTH) / TILE_SIZE);
        int row = static_cast<int>((HALF_HEIGHT - y) / TILE_SIZE);

        // Check the 3x3 neighborhood around the current grid cell
        for (int dc = -1; dc <= 1; ++dc) {
            for (int dr = -1; dr <= 1; ++dr) {
				// Skip out-of-bounds cells
                int nc = col + dc;
                int nr = row + dr;
                if (nc < 0 || nc >= WORLD_COLS || nr < 0 || nr >= WORLD_ROWS) continue;
                if (mapGrid[nr][nc] != 1) continue;

				// Calculate the world position of the center of this wall tile
                float wallX = (nc * TILE_SIZE) - HALF_WIDTH + (TILE_SIZE / 2.0f);
                float wallY = HALF_HEIGHT - (nr * TILE_SIZE) - (TILE_SIZE / 2.0f);

				// Compute the vector from the wall center to the object position
                float dx = x - wallX;
                float dy = y - wallY;
                float dist = sqrtf(dx * dx + dy * dy);
                float minDist = radius + TILE_SIZE * 0.5f;

                // If overlapping, push the object away from the wall center
                if (dist < minDist && dist > 0.01f) {
                    float push = (minDist - dist) / dist;
                    x += dx * push;
                    y += dy * push;
                }
            }
        }
    }

    // ~ Brief: Free all loaded assets and reset decor state
    void Free_World() {
        if (pWallMesh) { AEGfxMeshFree(pWallMesh);         pWallMesh = nullptr; }
        if (pWallTex) { AEGfxTextureUnload(pWallTex);     pWallTex = nullptr; }
        if (pGroundTex) { AEGfxTextureUnload(pGroundTex);  pGroundTex = nullptr; }
        if (pTreeTex) { AEGfxTextureUnload(pTreeTex);     pTreeTex = nullptr; }
        if (pOilTex) { AEGfxTextureUnload(pOilTex);      pOilTex = nullptr; }
        if (pTwigTex) { AEGfxTextureUnload(pTwigTex);     pTwigTex = nullptr; }
        decorInitialized = false;
    }
}