#include "MasterHeader.h"

void GenerateWave(int waveNumber, shape& player);
void UpdateWaveSpawning(float dt, shape& player);
bool IsWaveCleared();
void skipWave(shape& player);
void printEnemyCount();
