#include "MasterHeader.h"

namespace World {
    int mapGrid[WORLD_ROWS][WORLD_COLS];
    static AEGfxVertexList* pWallMesh = nullptr;
    static AEGfxTexture* pWallTex = nullptr;
    static AEGfxTexture* pGroundTex = nullptr;
    static AEGfxTexture* pTreeTex = nullptr;
    static AEGfxTexture* pOilTex = nullptr;
    static AEGfxTexture* pTwigTex = nullptr;

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

    // Converts grid col/row to world position (center of tile)
    static AEVec2 GridToWorld(int col, int row) {
        AEVec2 pos;
        pos.x = (col * TILE_SIZE) - HALF_WIDTH + (TILE_SIZE / 2.0f);
        pos.y = HALF_HEIGHT - (row * TILE_SIZE) - (TILE_SIZE / 2.0f);
        return pos;
    }

    // Returns a random world position inside the map (not on border tiles)
    static AEVec2 RandomInteriorPos() {
        // cols 2-32, rows 2-17 (keep 2 tiles away from border)
        int col = 2 + (rand() % 31);
        int row = 2 + (rand() % 16);

        // avoid tree positions
        for (int i = 0; i < 5; ++i) {
            if (col == treePlacements[i].col && row == treePlacements[i].row) {
                col = (col + 3) % 31 + 2;
            }
        }
        return GridToWorld(col, row);
    }

    void Load_World() {
        if (pWallTex != nullptr || pGroundTex != nullptr || pWallMesh != nullptr) return;
        pWallTex = AEGfxTextureLoad("./Assets/wall.png");
        pGroundTex = AEGfxTextureLoad("./Assets/sand.png");
        pTreeTex = AEGfxTextureLoad("./Assets/tree.png");
        pOilTex = AEGfxTextureLoad("./Assets/oilspill.png");
        pTwigTex = AEGfxTextureLoad("./Assets/twigs.png");

        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        pWallMesh = AEGfxMeshEnd();
    }

    void Init_World() {
        // -- Build grid --
        for (int row = 0; row < WORLD_ROWS; row++) {
            for (int col = 0; col < WORLD_COLS; col++) {
                if (row == 0 || row == WORLD_ROWS - 1 || col == 0 || col == WORLD_COLS - 1)
                    mapGrid[row][col] = 1;
                else
                    mapGrid[row][col] = 0;
            }
        }

        // -- Mark tree cells as walls --
        for (int i = 0; i < 5; ++i) {
            mapGrid[treePlacements[i].row][treePlacements[i].col] = 1;
        }

        // -- Randomize decorative objects once --
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

    bool isPointColliding(float worldX, float worldY) {
        int col = static_cast<int>((worldX + World::HALF_WIDTH) / World::TILE_SIZE);
        int row = static_cast<int>((World::HALF_HEIGHT - worldY) / World::TILE_SIZE);
        if (col < 0 || col >= World::WORLD_COLS || row < 0 || row >= World::WORLD_ROWS) return true;
        return (World::mapGrid[row][col] == 1);
    }

    bool CheckCollision(float x, float y, float playerScale, float playerRotation) {
        float h = (playerScale * 0.9f);
        AEVec2 tankPoints[12] = {
            // corners
            { h,  h }, { h, -h }, {-h,  h }, {-h, -h },
            // midpoints of each side
            { h,  0 }, {-h,  0 },  // left and right midpoints
            { 0,  h }, { 0, -h },  // top and bottom midpoints
            // extra points along sides for wider coverage
            { h,  h * 0.5f }, { h, -h * 0.5f },
            {-h,  h * 0.5f }, {-h, -h * 0.5f }
        };

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

        for (int i = 0; i < 12; i++) {
            float worldX = x + (tankPoints[i].x * cosA - tankPoints[i].y * sinA);
            float worldY = y + (tankPoints[i].x * sinA + tankPoints[i].y * cosA);
            if (isPointColliding(worldX, worldY)) return true;
        }
        for (int i = 0; i < 4; i++) {
            float worldX = x + (barrelPoints[i].x * cosA - barrelPoints[i].y * sinA);
            float worldY = y + (barrelPoints[i].x * sinA + barrelPoints[i].y * cosA);
            if (isPointColliding(worldX, worldY)) return true;
        }
        return false;
    }

    // Helper: draw a textured quad at world position
    static void DrawDecor(AEGfxTexture* tex, float x, float y, float scale, float rotDeg) {
        if (!tex || !pWallMesh) return;
        AEGfxTextureSet(tex, 0, 0);
        float rotRad = rotDeg * (PI / 180.0f);
        AEMtx33 s, r, t, final;
        AEMtx33Scale(&s, scale, scale);
        AEMtx33Rot(&r, rotRad);
        AEMtx33Trans(&t, x, y);
        AEMtx33Concat(&final, &r, &s);
        AEMtx33Concat(&final, &t, &final);
        AEGfxSetTransform(final.m);
        AEGfxMeshDraw(pWallMesh, AE_GFX_MDM_TRIANGLES);
    }

    void Draw_World() {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        // -- Draw ground/wall tiles --
        for (int row = 0; row < WORLD_ROWS; row++) {
            for (int col = 0; col < WORLD_COLS; col++) {

                // skip tree cells — drawn separately on top
                bool isTree = false;
                for (int t = 0; t < 5; ++t) {
                    if (col == treePlacements[t].col && row == treePlacements[t].row) {
                        isTree = true; break;
                    }
                }

                if (isTree) {
                    AEGfxTextureSet(pGroundTex, 0, 0); // draw ground under tree
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

        // -- Draw oil spills (under twigs, under trees) --
        for (int i = 0; i < NUM_OIL; ++i) {
            DrawDecor(pOilTex, oilSpills[i].x, oilSpills[i].y, oilSpills[i].scale, oilSpills[i].rotation);
        }

        // -- Draw twigs --
        for (int i = 0; i < NUM_TWIG; ++i) {
            DrawDecor(pTwigTex, twigs[i].x, twigs[i].y, twigs[i].scale, twigs[i].rotation);
        }

        // -- Draw trees on top --
        for (int i = 0; i < 5; ++i) {
            AEVec2 pos = GridToWorld(treePlacements[i].col, treePlacements[i].row);
            DrawDecor(pTreeTex, pos.x, pos.y, TILE_SIZE * 1.4f, 0.0f);
        }
    }

    void PushOutOfWalls(float& x, float& y, float radius) {
        int col = static_cast<int>((x + HALF_WIDTH) / TILE_SIZE);
        int row = static_cast<int>((HALF_HEIGHT - y) / TILE_SIZE);

        for (int dc = -1; dc <= 1; ++dc) {
            for (int dr = -1; dr <= 1; ++dr) {
                int nc = col + dc;
                int nr = row + dr;
                if (nc < 0 || nc >= WORLD_COLS || nr < 0 || nr >= WORLD_ROWS) continue;
                if (mapGrid[nr][nc] != 1) continue;

                float wallX = (nc * TILE_SIZE) - HALF_WIDTH + (TILE_SIZE / 2.0f);
                float wallY = HALF_HEIGHT - (nr * TILE_SIZE) - (TILE_SIZE / 2.0f);

                float dx = x - wallX;
                float dy = y - wallY;
                float dist = sqrtf(dx * dx + dy * dy);
                float minDist = radius + TILE_SIZE * 0.5f;

                if (dist < minDist && dist > 0.01f) {
                    float push = (minDist - dist) / dist;
                    x += dx * push;
                    y += dy * push;
                }
            }
        }
    }

    void Free_World() {
        if (pWallMesh) { AEGfxMeshFree(pWallMesh);        pWallMesh = nullptr; }
        if (pWallTex) { AEGfxTextureUnload(pWallTex);    pWallTex = nullptr; }
        if (pGroundTex) { AEGfxTextureUnload(pGroundTex);  pGroundTex = nullptr; }
        if (pTreeTex) { AEGfxTextureUnload(pTreeTex);    pTreeTex = nullptr; }
        if (pOilTex) { AEGfxTextureUnload(pOilTex);     pOilTex = nullptr; }
        if (pTwigTex) { AEGfxTextureUnload(pTwigTex);    pTwigTex = nullptr; }
        decorInitialized = false;
    }
}