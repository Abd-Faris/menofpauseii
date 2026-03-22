#include "MasterHeader.h"


// =================================
//  WAVE GENERATION
// =================================


// Externs from GAME.CPP
extern int currentWave;

// Globals
float spawnTimer = 0.0f;
float const MAX_SPAWN_TIME = 5.0f;
float pendingBudget = 0.0f;
float totalWaveBudget = 0.0f;
int totalEnemiesSpawned = 0;

// Struct
struct WeightedChoice {
	EnemyTypeInfo type;
	float weight;
};

// Vector
std::vector<WeightedChoice> currentWaveChoices;

//@param waveNumber: the current wave number
//@param player: the player's shape, used to determine spawn locations and difficulty scaling
void GenerateWave(int waveNumber, shape& player) {

	//reset enemies spawned to 0 first
	totalEnemiesSpawned = 0;

	// ---- BOSS WAVE ----
	//check if wave is multiple of 5
	if (waveNumber % 5 == 0) {
		BossType type;
		//get cycle count so its repeating
		int cycle = ((waveNumber / 5) - 1) % 4;
		//wave 5, 25
		if (cycle == 0) type = BOSS1;
		//wave 10, 30
		else if (cycle == 1) type = BOSS2;
		//wave 15, 35
		else if (cycle == 2) type = BOSS3;
		//wave 20, 40
		else type = BOSS4;

		//call to spawn boss
		SpawnBoss(type, player);
		totalEnemiesSpawned = 1;

		pendingBudget = 0.0f;
		spawnTimer = 0.0f;
		return;
	}

	// ---- SET UP TIMER FOR NORMAL WAVES ----
	//calculate budget
	totalWaveBudget = 20.0f + (5.0f * powf((float)waveNumber, 1.5f));
	pendingBudget = totalWaveBudget;
	spawnTimer = MAX_SPAWN_TIME;

	// ---- ID, COST, BASE WEIGHT, GROWTH RATE, MIN WAVE, MAX WAVE - 1 ----
	// CAN CHANGE ANYTIME
	EnemyTypeInfo passive1 = { 1, 2, 70, -6.0f, 1, 3 }; //passive small
	EnemyTypeInfo passive2 = { 2, 6, 20, 2.0f, 1, 3}; //passive big
	EnemyTypeInfo kamikaze = { 3, 10, 10, 7.5f, 2, 0}; //kamikaze
	EnemyTypeInfo shooter = { 4, 10, 10, 7.5f, 4, 0 }; //shooter

	//this creates a pool of enemy types 
	std::vector<EnemyTypeInfo> pool = {
		passive1,
		passive2,
		kamikaze,
		shooter
	};

	currentWaveChoices.clear();
	for (auto& enemy : pool) {
		if (waveNumber >= enemy.minWave) {
			if (enemy.maxWave > 0 && waveNumber > enemy.maxWave) continue;

			float weight = (float)enemy.baseWeight + (enemy.growthRate * (float)(waveNumber - enemy.minWave));
			if (weight < 0.0f) weight = 0.0f;
			currentWaveChoices.push_back({ enemy, weight });
		}
	}
}

//  ---- GRADUAL SPAWN UPDATE ----
void UpdateWaveSpawning(float dt, shape& player) {
	// always tick down the timer regardless of budget
	if (spawnTimer > 0.0f) spawnTimer -= dt;

	// only spawn if there's budget and time left
	if (pendingBudget <= 0.0f || spawnTimer <= 0.0f) return;

	float targetBudget = totalWaveBudget * (spawnTimer / MAX_SPAWN_TIME);
	if (targetBudget < 0) targetBudget = 0;

	while (pendingBudget > targetBudget && pendingBudget > 0.0f) {

		float totalWeight = 0.0f;
		for (const auto& c : currentWaveChoices) totalWeight += c.weight;
		if (totalWeight <= 0.0f) break;

		float roll = ((float)rand() / (float)RAND_MAX) * totalWeight;
		float cumulativeWeight = 0.0f;
		int pickedIndex = -1;

		for (int i = 0; i < (int)currentWaveChoices.size(); ++i) {
			cumulativeWeight += currentWaveChoices[i].weight;
			if (roll <= cumulativeWeight) {
				pickedIndex = i;
				break;
			}
		}

		if (pickedIndex != -1) {
			EnemyTypeInfo& selected = currentWaveChoices[pickedIndex].type;

			if (pendingBudget >= (float)selected.cost) {
				if (selected.id == 1)      SpawnOneEnemy(false, player);
				else if (selected.id == 2) SpawnOneEnemy(true, player);
				else if (selected.id == 3) SpawnAttackEnemy(player);
				else if (selected.id == 4) SpawnShooterEnemy(player);

				pendingBudget -= (float)selected.cost;
				totalEnemiesSpawned++;
			}
			else {
				break;
			}
		}
	}
}

//checks if all enemies are dead
bool IsWaveCleared() {

	if (spawnTimer > 0.0f) {
		return false;
	}
	if (currentWave % 5 == 0 && currentboss.alive) {
		return false;
	}
	for (int i = 0; i < MAX_MINIONS_COUNT; ++i)
		if (minionPool[i].alive) {
			return false;
		}
	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i)
		if (enemyPool[i].alive) {
			return false;
		}
	return true;
}

//for debugging 
void skipWave(shape& player) {
	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i) {
		enemyPool[i].alive = false;
	}

	for (int i = 0; i < MAX_MINIONS_COUNT; ++i) {
		minionPool[i].alive = false;
	}

	currentboss.alive = false;
	pendingBudget = 0.0f;
	spawnTimer = 0.0f;
	currentWave++;
	GenerateWave(currentWave, player);
}


void printEnemyCount() {

	int activeEnemyCount = 0;

	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i) {
		if (enemyPool[i].alive) activeEnemyCount++;
	}
	if (currentboss.alive) activeEnemyCount++;

	GfxText text{ "Enemies Remaining : " + std::to_string(activeEnemyCount)};
	text.scale = 0.35f;
	text.pos = { 630, 400 };
	Gfx::printText(text, boldPixels);
}