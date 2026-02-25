#pragma once

extern BulletObj bulletList[GameConfig::MAX_BULLETS_COUNT];
extern float bulletFireTimer;
extern bool bigcannon;

void DrawMultiBarrels(int count, float gap, float pivotOffset, float tankRot, float tankX, float tankY, float barrelWidth, float barrelLength, AEGfxVertexList* MeshRect);
void drawBigTank(shape& player);
void movePlayer(shape &player, float deltaTime);
void rotatePlayer(shape& player);
void ShootBullet(shape& player, float deltaTime);
void drawBigCannon(shape& player);
void updateBullets(shape& player, float deltaTime);