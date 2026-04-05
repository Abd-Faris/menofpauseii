// -----------------------------Gloomy's Revenge---------------------------- //
// File:	Wave.cpp
// Authors:	[Men of Pause II]
// Brief:	Handles procedural wave scaling, boss spawning, and enemy weighting.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

// =============================================================================
// GLOBALS & EXTERNS
// =============================================================================

extern int currentWave;

float spawnTimer = 0.0f;
float const MAX_SPAWN_TIME = 5.0f;
float pendingBudget = 0.0f;
float totalWaveBudget = 0.0f;
int totalEnemiesSpawned = 0;

struct WeightedChoice {
	EnemyTypeInfo type;
	float weight;
};

std::vector<WeightedChoice> currentWaveChoices;

// =============================================================================
// WAVE GENERATION LOGIC
// =============================================================================

// ~ Brief: Prepares the enemy pool and difficulty budget for a new wave.
//          Handles special boss cycles every 5 waves.
void GenerateWave(int waveNumber, shape& player) {

	// Reset spawn counter for the new wave
	totalEnemiesSpawned = 0;

	// ---- BOSS WAVE CHECK ----
	if (waveNumber % 5 == 0) {
		BossType type;

		// Cycle through the 4 available bosses based on wave progression
		int cycle = ((waveNumber / 5) - 1) % 4;
		if (cycle == 0)      type = BOSS1;
		else if (cycle == 1) type = BOSS2;
		else if (cycle == 2) type = BOSS3;
		else                 type = BOSS4;

		// Spawn the boss and clear the budget since it's a solo encounter
		SpawnBoss(type, player);
		totalEnemiesSpawned = 1;
		pendingBudget = 0.0f;
		spawnTimer = 0.0f;
		return;
	}

	// ---- NORMAL WAVE SETUP ----
	// Scaled budget formula: 20 + (5 * wave^1.5)
	totalWaveBudget = 20.0f + (5.0f * powf((float)waveNumber, 1.5f));
	pendingBudget = totalWaveBudget;
	spawnTimer = MAX_SPAWN_TIME;

	// Define enemy archetypes: ID, COST, BASE WEIGHT, GROWTH RATE, MIN WAVE, MAX WAVE
	EnemyTypeInfo passive1 = { 1, 2,  60, 0.0f, 1, 1 };
	EnemyTypeInfo passive2 = { 2, 6,  40, 0.0f, 1, 1 };
	EnemyTypeInfo kamikaze = { 3, 10, 10, 8.0f, 1, 0 };
	EnemyTypeInfo shooter = { 4, 10, 5,  8.0f, 3, 0 };

	// Ensure Wave 1 always starts with a threat
	if (waveNumber == 1) {
		SpawnAttackEnemy(player);
		pendingBudget -= (float)kamikaze.cost;
		totalEnemiesSpawned++;
	}

	std::vector<EnemyTypeInfo> pool = { passive1, passive2, kamikaze, shooter };

	// Filter the pool based on the current wave number and calculate dynamic weights
	currentWaveChoices.clear();
	for (auto& enemy : pool) {
		if (waveNumber >= enemy.minWave) {
			if (enemy.maxWave > 0 && waveNumber > enemy.maxWave) continue;

			// Increase weight over time based on growth rate
			float weight = (float)enemy.baseWeight + (enemy.growthRate * (float)(waveNumber - enemy.minWave));
			if (weight < 0.0f) weight = 0.0f;
			currentWaveChoices.push_back({ enemy, weight });
		}
	}
}

// =============================================================================
// SPAWNING UPDATES
// =============================================================================

// ~ Brief: Gradually spends the wave budget over the spawn duration.
//          Uses a weighted random system to pick which enemies to spawn.
void UpdateWaveSpawning(float dt, shape& player) {
	if (spawnTimer > 0.0f) spawnTimer -= dt;

	// Stop spawning if out of points or the timer has finished
	if (pendingBudget <= 0.0f || spawnTimer <= 0.0f) return;

	// Determine how much budget should have been spent based on time elapsed
	float targetBudget = totalWaveBudget * (spawnTimer / MAX_SPAWN_TIME);
	if (targetBudget < 0) targetBudget = 0;

	// Keep spawning until we catch up to the target budget
	while (pendingBudget > targetBudget && pendingBudget > 0.0f) {

		float totalWeight = 0.0f;
		for (const auto& c : currentWaveChoices) totalWeight += c.weight;
		if (totalWeight <= 0.0f) break;

		// Weighted random roll
		float roll = ((float)rand() / (float)RAND_MAX) * totalWeight;
		float cumulativeWeight = 0.0f;
		int pickedIndex = -1;

		// Iterate through the choices to find which enemy type corresponds to the roll
		for (int i = 0; i < (int)currentWaveChoices.size(); ++i) {
			cumulativeWeight += currentWaveChoices[i].weight;
			if (roll <= cumulativeWeight) {
				pickedIndex = i;
				break;
			}
		}

		// Spawn the picked enemy if we can afford it, otherwise break out of the loop
		if (pickedIndex != -1) {
			EnemyTypeInfo& selected = currentWaveChoices[pickedIndex].type;

			// Check if we can afford the picked enemy
			if (pendingBudget >= (float)selected.cost) {
				if (selected.id == 1)      SpawnOneEnemy(false, player);
				else if (selected.id == 2) SpawnOneEnemy(true, player);
				else if (selected.id == 3) SpawnAttackEnemy(player);
				else if (selected.id == 4) SpawnShooterEnemy(player);

				// Subtract the cost from the pending budget and increment the spawn count
				pendingBudget -= (float)selected.cost;
				totalEnemiesSpawned++;
			}
			else {
				break; // Cannot afford anything else in the current weight roll
			}
		}
	}
}

// =============================================================================
// UTILITY & DEBUG
// =============================================================================

// ~ Brief: Verifies if all enemies, minions, and bosses are dead and spawning is finished.
bool IsWaveCleared() {
	if (spawnTimer > 0.0f) return false;
	if (currentWave % 5 == 0 && currentboss.alive) return false;

	// Check minion pool
	for (int i = 0; i < MAX_MINIONS_COUNT; ++i)
		if (minionPool[i].alive) return false;

	// Check main enemy pool
	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i)
		if (enemyPool[i].alive) return false;

	return true;
}

// ~ Brief: Clears all active enemies and immediately prepares the next wave.
void skipWave(shape& player) {
	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i) {
		enemyPool[i].alive = false;
	}

	// Clear minions and boss as well to be safe
	for (int i = 0; i < MAX_MINIONS_COUNT; ++i) {
		minionPool[i].alive = false;
	}

	// Clear the boss if it's a boss wave
	currentboss.alive = false;
	// Reset wave spawning state
	pendingBudget = 0.0f;
	spawnTimer = 0.0f;

	currentWave++;
	GenerateWave(currentWave, player);
}

// ~ Brief: Tallies up all alive entities and displays the count on the HUD.
void printEnemyCount() {
	int activeEnemyCount = 0;

	// Count alive minions
	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i) {
		if (enemyPool[i].alive) activeEnemyCount++;
	}
	if (currentboss.alive) activeEnemyCount++;

	GfxText text{ "Enemies Remaining : " + std::to_string(activeEnemyCount) };
	text.scale = 0.35f;
	text.pos = { 630, 400 };
	Gfx::printText(text, boldPixels);
}