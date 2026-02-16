#include "MasterHeader.h"

f64 dt{};
f64 timer{}, timer2{};
enum { MAX_ENEMIES = 50 };
std::array <Enemies, MAX_ENEMIES> maxenemy = {};


f32 maxspeed = 50;
f32 friction = 0.92;
namespace {
	s8 boldPixels;
	AEGfxVertexList* MeshRect{};
	
}

void DrawEnemyMesh() {

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	AEGfxTriAdd(
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	// Saving the mesh (list of triangles) in MeshRect
	MeshRect = AEGfxMeshEnd();

}

void InitEnemies(Enemies *enemy) {
	enemy->pos.x = 67676767.f; // spawn way outside player boundary
	enemy->pos.y = 67676767.f; // spawn way outside player boundary
	enemy->scale = 0;
	enemy->xp = 0;
	enemy->rotation = 0;//static_cast<float>(AERandFloat() * 360);
	enemy->alive = false;
	enemy->hp = 0;
	enemy->velocity.x = 0.f;
	enemy->velocity.y = 0.f;
}

void separateEnemies() {
	const float separationForce = 0.5f;

	for (size_t i = 0; i < 20; i++) {
		if (!maxenemy[i].alive) continue;

		for (size_t j = i + 1; j < 20; j++) {
			if (!maxenemy[j].alive) continue;

			AEVec2 delta;
			AEVec2Sub(&delta, &maxenemy[j].pos, &maxenemy[i].pos);
			
			float dist = AEVec2Length (&delta);
			float minDist = maxenemy[i].scale + maxenemy[j].scale;

			if (dist < minDist && dist > 0) {
				AEVec2 pushDir;
				AEVec2Normalize(&pushDir, &delta);
				float overlap = minDist - dist;
				AEVec2 pushforce;
				AEVec2Scale(&pushforce, &pushDir, (overlap * 0.5f));
				
				AEVec2Sub(&maxenemy[i].pos, &maxenemy[i].pos, &pushforce);
				AEVec2Add(&maxenemy[j].pos, &maxenemy[j].pos, &pushforce);
		
			}
		}
	}
}
void applyPhysicsSeparation(float dt, std::array<Enemies, MAX_ENEMIES>& maxenemy, int size) {
	const float separationStrength = 500.0f;
	const float separationRange = 1.5f; // Multiplier for when to start separating

	for (size_t i = 0; i < size; i++) {
		if (!maxenemy[i].alive) continue;

		AEVec2 separationForce = { 0,0 };

		for (size_t j = 0; j < size; j++) {
			if (i == j || !maxenemy[j].alive) continue;

			AEVec2 delta;
			AEVec2Sub (&delta, &maxenemy[i].pos, &maxenemy[j].pos);
			float dist = AEVec2Length(&delta);
			float minDist = (maxenemy[i].scale/2 + maxenemy[j].scale/2) * separationRange;
			


			if (dist < minDist && dist > 0.01f) {
				AEVec2 pushDir;
				AEVec2Normalize(&pushDir, &delta);
				float overlap = minDist - dist;

				// Force increases as enemies get closer
				float forceMagnitude = (overlap / minDist) * separationStrength;
				AEVec2 pushforce;
				AEVec2Scale(&pushforce, &pushDir, forceMagnitude);
				//separationForce = separationForce + pushDir * forceMagnitude;
				AEVec2Add(&separationForce, &separationForce, &pushforce);
			}
		}

		// Apply acceleration (F = ma, so a = F/m)
		AEVec2 acceleration;
		AEVec2Scale(&acceleration, &separationForce, dt);

		AEVec2Add(&maxenemy[i].velocity, &maxenemy[i].velocity, &acceleration);
		//maxenemy[i].velocity = maxenemy[i].velocity + acceleration;
	}
}

void PlayerPhysicsSeparation(float dt, std::array <Enemies, MAX_ENEMIES> &maxenemy, int size, shape Player) {
	const float separationStrength = 500.0f;
	const float separationRange = 1.5f; // Multiplier for when to start separating

	for (size_t i = 0; i < size; i++) {
		if (!maxenemy[i].alive) continue;

		AEVec2 separationForce = { 0,0 };

		AEVec2 PlayerPos = { Player.pos_x, Player.pos_y };

			AEVec2 delta;
			AEVec2Sub(&delta, &maxenemy[i].pos, &PlayerPos);
			float dist = AEVec2Length(&delta);
			float minDist = (maxenemy[i].scale / 2 + Player.scale / 2) * separationRange;



			if (dist < minDist && dist > 0.01f) {
				AEVec2 pushDir;
				AEVec2Normalize(&pushDir, &delta);
				float overlap = minDist - dist;

				// Force increases as enemies get closer
				float forceMagnitude = (overlap / minDist) * separationStrength;
				AEVec2 pushforce;
				AEVec2Scale(&pushforce, &pushDir, forceMagnitude);
				//separationForce = separationForce + pushDir * forceMagnitude;
				AEVec2Add(&separationForce, &separationForce, &pushforce);
			}
		

		// Apply acceleration (F = ma, so a = F/m)
		AEVec2 acceleration;
		AEVec2Scale(&acceleration, &separationForce, dt);

		AEVec2Add(&maxenemy[i].velocity, &maxenemy[i].velocity, &acceleration);
		//maxenemy[i].velocity = maxenemy[i].velocity + acceleration;
	}
}
void updateEnemyPhysics(float dt, std::array <Enemies, MAX_ENEMIES> &maxenemy, int size) {
	for (int i{}; i < size; i++) {
		if (!maxenemy[i].alive) continue;

		// Apply friction/damping for natural deceleration

		AEVec2Scale(&maxenemy[i].velocity, &maxenemy[i].velocity, friction);

		// Clamp to max speed
		float speed = AEVec2Length(&maxenemy[i].velocity);
		if (speed > maxspeed) {
			AEVec2 normal;
			AEVec2Normalize(&normal, &maxenemy[i].velocity);
			AEVec2Scale(&maxenemy[i].velocity, &normal, maxspeed);
			
		}

		// Update position

		AEVec2 currentvelo;
		AEVec2Scale(&currentvelo, &maxenemy[i].velocity, dt);
		AEVec2Add(&maxenemy[i].pos, &maxenemy[i].pos, &currentvelo);
		

		// Keep enemies on screen (with bounce)
		/*if (enemy.pos.x < enemy.scale) {
			enemy.pos.x = enemy.scale;
			enemy.velocity.x *= -0.5f;
		}
		else if (enemy.pos.x > AEGfxGetWindowWidth() - enemy.scale) {
			enemy.pos.x = AEGfxGetWindowWidth() - enemy.scale;
			enemy.velocity.x *= -0.5f;
		}

		if (enemy.pos.y < enemy.scale) {
			enemy.pos.y = enemy.scale;
			enemy.velocity.y *= -0.5f;
		}
		else if (enemy.pos.y > AEGfxGetWindowHeight() - enemy.scale) {
			enemy.pos.y = AEGfxGetWindowHeight() - enemy.scale;
			enemy.velocity.y *= -0.5f;
		}*/
	}
}

void SpawnEnemies(Enemies* enemy) {
	
	float setscale = 30;

	float posx = static_cast<float>((AERandFloat() * AEGfxGetWindowWidth()) - AEGfxGetWindowWidth() / 2) - setscale;
	if (posx < (-AEGfxGetWindowWidth() / 2) + setscale) {
		posx = (-AEGfxGetWindowWidth() / 2) + setscale;
	}

	float posy = static_cast<float>((AERandFloat() * AEGfxGetWindowHeight()) - AEGfxGetWindowHeight() / 2) - setscale;
	if (posy < (-AEGfxGetWindowHeight() / 2) + setscale) {
		posy = (-AEGfxGetWindowHeight() / 2) + setscale;
	}

	enemy->pos.x = posx;
	enemy->pos.y = posy;
	enemy->scale = setscale;
	enemy->xp = 10;
	enemy->rotation = static_cast<float>(AERandFloat() * 360);
	enemy->alive = true;
	enemy->hp = 15;
	enemy->velocity.x = 0;
	enemy->velocity.y = 0;
}

void SpawnBigEnemies(Enemies* enemy) {

	float setscale = 67;

	float posx = static_cast<float>((AERandFloat() * AEGfxGetWindowWidth()) - AEGfxGetWindowWidth() / 2) - setscale;
	if (posx < (-AEGfxGetWindowWidth() / 2) + setscale) {
		posx = (-AEGfxGetWindowWidth() / 2) + setscale;
	}

	float posy = static_cast<float>((AERandFloat() * AEGfxGetWindowHeight()) - AEGfxGetWindowHeight() / 2) - setscale;
	if (posy < (-AEGfxGetWindowHeight() / 2) + setscale) {
		posy = (-AEGfxGetWindowHeight() / 2) + setscale;
	}

	enemy->pos.x = posx;
	enemy->pos.y = posy;
	enemy->scale = setscale;
	enemy->xp = 25;
	enemy->rotation = static_cast<float>(AERandFloat() * 360);
	enemy->alive = true;
	enemy->hp = 100;
	enemy->velocity.x = 1;
	enemy->velocity.y = 1;
}

void EnemyDeath(std::array <Enemies, MAX_ENEMIES> maxenemy, int size, f64 dt){
	for (int i{}; i < MAX_ENEMIES; i++) {
		if (maxenemy[i].hp <= 0) {
			
			while (maxenemy[i].scale>0) maxenemy[i].scale -= 50 * dt;
			maxenemy[i].scale < 0 ? 0 : maxenemy[i].scale;
			maxenemy[i].alive = false;
		}
	}
}

void LoadDebug3() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
	DrawEnemyMesh();
	for (int i = 0; i < MAX_ENEMIES; i++) {
		InitEnemies(&maxenemy[i]);
	}
	
	for (int i = 0; i < 20; i++) {
		SpawnEnemies(&maxenemy[i]);
	}

	applyPhysicsSeparation(dt, maxenemy, MAX_ENEMIES);

	// Update enemy positions and velocities
	updateEnemyPhysics(dt, maxenemy, MAX_ENEMIES);

}

void DrawDebug3() {

	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);

	dt = AEFrameRateControllerGetFrameTime();
	timer += dt;

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	if (timer >= 3) {
		float random = AERandFloat() * 10;
		if (random >= 4) {
			for (int i = 0; i < MAX_ENEMIES; i++) {

				if (!maxenemy[i].alive) {
					
					SpawnEnemies(&maxenemy[i]);
					//AEGfxSetCamPosition(maxenemy[i].pos_x, maxenemy[i].pos_y);
					break;
				}
			}
		}
		else if (random < 4) {

			for (int i = 0; i < MAX_ENEMIES; i++) {

				if (!maxenemy[i].alive) {
					
					SpawnBigEnemies(&maxenemy[i]);
					//AEGfxSetCamPosition(maxenemy[i].pos_x, maxenemy[i].pos_y);
					break;
				}
			}
		}

		timer = 0;
	}

	for (int i = 0; i < MAX_ENEMIES; i++) {

		if (maxenemy[i].alive) {

			if (maxenemy[i].scale <= 30) {
				AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1.0f);
			}
			else if (maxenemy[i].scale > 30) AEGfxSetColorToMultiply(0.2f, 0.2f, 0.8f, 1.0f);;

		
		AEMtx33 transformSquare, scaleSquare, translateSquare, rotateSquare;
		AEMtx33Identity(&transformSquare);



		AEMtx33Scale(&scaleSquare, maxenemy[i].scale, maxenemy[i].scale);

		AEMtx33RotDeg(&rotateSquare, maxenemy[i].rotation);

		AEMtx33Trans(&translateSquare, maxenemy[i].pos.x, maxenemy[i].pos.y);



		AEMtx33Concat(&transformSquare, &translateSquare, &rotateSquare);

		AEMtx33Concat(&transformSquare, &transformSquare, &scaleSquare);

		AEGfxSetTransform(transformSquare.m);
		AEGfxMeshDraw(MeshRect, AE_GFX_MDM_TRIANGLES);
		}

		//if () {

		//}
}

	

	/*timer2 += dt;
	
	if (timer2 >= 1) {
		int dmg = std::rand() % MAX_ENEMIES;
		if (maxenemy[dmg].alive) {
			maxenemy[dmg].hp = 0;
		}
		timer2 = 0;
	}*/
	//separateEnemies();
	applyPhysicsSeparation(dt, maxenemy, MAX_ENEMIES);

	// Update enemy positions and velocities
	updateEnemyPhysics(dt, maxenemy, MAX_ENEMIES);
	
	EnemyDeath(maxenemy, MAX_ENEMIES, dt);


}

void FreeDebug3() {
	// free font
	AEGfxDestroyFont(boldPixels);
	AEGfxMeshFree(MeshRect);
}