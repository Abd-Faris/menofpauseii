#pragma once
#include "MasterHeader.h"
void SpawnBoss(BossType type, shape& player);
void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime);
void DrawBoss(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle);
void BossShootRing(Boss& boss);
extern Boss boss;