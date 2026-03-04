#pragma once
#include "MasterHeader.h"
void SpawnBoss(BossType type, shape& player);
void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime);
void DrawBoss(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle);
void BossShootRing(Boss& boss);
void BossCollision(Boss& boss, shape& player, bool orbitActive, float orbitPosX, float orbitPosY);
void DrawBossHP(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle, shape& player);
extern Boss boss;