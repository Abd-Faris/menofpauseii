#include "MasterHeader.h"


// =================================
//  WAVE GENERATION
// =================================


// Externs from GAME.CPP
extern int currentWave;


//@param waveNumber: the current wave number
//@param player: the player's shape, used to determine spawn locations and difficulty scaling
void GenerateWave(int waveNumber, shape& player) {

	// ---- BOSS WAVE ----
	//check if wave is multiple of 5
	if (waveNumber % 5 == 0) {
		BossType type;
		//get cycle count so its repeating
		int cycle = ((waveNumber / 5) - 1) % 3; 
		//wave 5, 20, 35..
		if (cycle == 0) type = BOSS1; 
		//wave 10, 25, 40..
		else if (cycle == 1) type = BOSS2; 
		//wave 15, 30, 45..
		else                 type = BOSS3; 
		//call to spawn boss
		SpawnBoss(type, player);
		return;
	}

	// ---- ID, COST, BASE WEIGHT, GROWTH RATE, MIN WAVE ----
	// CAN CHANGE ANYTIME
	EnemyTypeInfo passive1 = { 1, 2, 70, -6.0f, 1 }; //passive small
	EnemyTypeInfo passive2 = { 2, 6, 20, 2.0f, 1 }; //passive big
	EnemyTypeInfo kamikaze = { 3, 10, 10, 7.5f, 2 }; //kamikaze
	EnemyTypeInfo shooter = { 4, 10, 10, 7.5f, 6 }; //shooter
	//can add more enemy/bosses

	//this creates a pool of enemy types 
	std::vector<EnemyTypeInfo> pool;
	//push back the enemy types into the pool
	pool.push_back(passive1);
	pool.push_back(passive2);
	pool.push_back(kamikaze);
	pool.push_back(shooter);

	//calculate budget
	float budget = 20.0f + (5.0f * powf((float)waveNumber, 1.5f)); 

	//filtering & weighting
	///create a struct for enemy type and weight for the current wave
	struct WeightedChoice {
		EnemyTypeInfo type; float weight;
	};

	//creates another list of choice that meet the wave requirement
	std::vector<WeightedChoice> activeChoices;
	float totalWeight = 0.0f;

	//this loop filters the pool for enemies that can spawn in the current wave and calculates their weights based on growth rate
	for (size_t i = 0; i < pool.size(); ++i) {
		//enemy is reference to the current enemy type in the pool
		EnemyTypeInfo& enemy = pool[i];
		//if current wave is >= the min wave for the enemy type
		if (waveNumber >= enemy.minWave) {
			//calculate weight: base weight + growth rate * (current wave - min wave)
			float weight = (float)enemy.baseWeight + (enemy.growthRate * (float)(waveNumber - enemy.minWave));
			//clamps weight to 0
			if (weight < 0.0f) weight = 0.0f;

			//creates a weightedchoice struct
			WeightedChoice choice = { enemy, weight };
			//push it to active choices
			activeChoices.push_back(choice);
			//add to total weight
			totalWeight += weight;
		}
	}
	
	//spawn enemies until budget is depleted
	while (budget > 0.0f && totalWeight > 0.0f) {
		//randomly pick an enemy type based on weights
		float roll = ((float)rand() / (float)RAND_MAX) * totalWeight;
		//keeps track of cumualtive weight
		float cumulativeWeight = 0.0f;
		//index of the picked enemy type, -1 because it is not picked yet
		int pickedIndex = -1;

		//loops through active choices to find which enemy type corresponds to the random roll
		for (int i = 0; i < (int)activeChoices.size(); ++i) {
			//skips if weight is <= 0
			if (activeChoices[i].weight <= 0.0f) continue;
			//adds the weight of the current enemy type to cumulative weight
			cumulativeWeight += activeChoices[i].weight;
			//if roll <= cumulative weight, we found our enemy type
			if (roll <= cumulativeWeight) {
				//sets picked index to id of enemy
				pickedIndex = i;
				break;
			}
		}

		//if an enemy type was picked
		if (pickedIndex != -1) {
			//reference to the selected enemy type info
			EnemyTypeInfo& selected = activeChoices[pickedIndex].type;
			//if we have enough budget to spawn this enemy type
			if (budget >= (float)selected.cost) {
				//spawn the enemy based on its id
				if (selected.id == 1) {
					SpawnOneEnemy(false, player);
				}
				else if (selected.id == 2) {
					SpawnOneEnemy(true, player);
				}
				else if (selected.id == 3) {
					SpawnAttackEnemy(player);
				}
				else if (selected.id == 4) {
					SpawnShooterEnemy(player);
				}

				//minus the cost of the spawned enemy from the budget
				budget -= (float)selected.cost;

			}
			else {
				//if cannot afford the picked enemy
				totalWeight -= activeChoices[pickedIndex].weight;
				//sets the weight of that enemy type to 0 so it won't be picked again
				activeChoices[pickedIndex].weight = 0.0f;
			}
		}
	}
}

//checks if all enemies are dead
bool IsWaveCleared() {
	//clear boss
	if (currentWave % 5 == 0 && boss.alive) return false;

	//clear minions from boss
	for (int i = 0; i < MAX_MINIONS_COUNT; ++i) {
		if (minionPool[i].alive) return false;
	}

	//clear enemies in general
	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i) {
		if (enemyPool[i].alive) return false;
	}
	return true;
}

//for debugging 
void skipWave(shape& player) {
	for (int i = 0; i < GameConfig::MAX_ENEMIES_COUNT; ++i) {
		enemyPool[i].alive = false;
	}
	boss.alive = false;
	currentWave++;
	GenerateWave(currentWave, player);
}
	