#pragma once
#include "MasterHeader.h"
void SpawnBoss(BossType type, shape& player);
void UpdateBossPhysics(Boss& boss, shape& player, float deltaTime);
void DrawBoss(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle);
void BossShootRing(Boss& boss);
void BossCollision(Boss& boss, shape& player, bool orbitActive, float orbitPosX, float orbitPosY);
void DrawBossHP(Boss& boss, AEGfxVertexList* MeshRect, AEGfxVertexList* MeshCircle, shape& player);
void BossSpawnMinion(Boss& boss, shape& player);
void updateMinionPhysics(shape& player, float deltaTime);
extern Boss boss;
constexpr int MAX_MINIONS_COUNT = 30;
extern std::array<Enemies, MAX_MINIONS_COUNT> minionPool;
void Boss3Spiral(Boss& boss, float deltaTime);
void Boss3AimedShot(Boss& boss, shape& player);

AEVec2 GetGunPosition(Boss& boss, bool leftGun);
void   Boss4ShootGuns(Boss& boss, shape& player, float deltaTime);
void   DrawBossLaser(Boss& boss, AEGfxVertexList* MeshRect);