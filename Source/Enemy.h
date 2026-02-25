#pragma once

extern std::array<Enemies, GameConfig::MAX_ENEMIES_COUNT> enemyPool;
extern f64 enemySpawnTimer;

void ResetEnemy(Enemies* enemyToReset);
void SpawnOneEnemy(bool isBigEnemy, shape player);
void SpawnAttackEnemy(shape player);
void SpawnShooterEnemy(shape player);
void EnemySpawner(shape& player, float deltaTime);
void updateEnemyPhysics(shape &player, float deltaTime);