#include "MasterHeader.h"

f64 dt{};
f64 timer{};
Enemies maxenemy[20]{};

f32 maxspeed = 50;
f32 friction = 0.92;
namespace {
	s8 boldPixels;
	enum {NUM_OF_TEXTS = 1};
	gfxtext texts[NUM_OF_TEXTS] = {
		{"Debug 3", 0, 0, 1, 255, 255, 255, 255}
	};
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
	enemy->pos.x = 0;//67676767; // spawn way outside player boundary
	enemy->pos.y = 0;//67676767; // spawn way outside player boundary
	enemy->scale = 0;
	enemy->xp = 0;
	enemy->rotation = 0;//static_cast<float>(AERandFloat() * 360);
	enemy->alive = false;
	enemy->hp = 0;
	enemy->velocity.x = 1;
	enemy->velocity.y = 1;
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
void applyPhysicsSeparation(float dt) {
	const float separationStrength = 500.0f;
	const float separationRange = 1.5f; // Multiplier for when to start separating

	for (size_t i = 0; i < 20; i++) {
		if (!maxenemy[i].alive) continue;

		AEVec2 separationForce = { 0,0 };

		for (size_t j = 0; j < 20; j++) {
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
void updateEnemyPhysics(float dt) {
	for (auto& enemy : maxenemy) {
		if (!enemy.alive) continue;

		// Apply friction/damping for natural deceleration

		AEVec2Scale(&enemy.velocity, &enemy.velocity, friction);

		// Clamp to max speed
		float speed = AEVec2Length(&enemy.velocity);
		if (speed > maxspeed) {
			AEVec2 normal;
			AEVec2Normalize(&normal, &enemy.velocity);
			AEVec2Scale(&enemy.velocity, &normal, maxspeed);
			
		}

		// Update position

		AEVec2 currentvelo;
		AEVec2Scale(&currentvelo, &enemy.velocity, dt);
		AEVec2Add(&enemy.pos, &enemy.pos, &currentvelo);
		

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
	enemy->pos.x = static_cast<float>((AERandFloat() * AEGfxGetWindowWidth()) - AEGfxGetWindowWidth()/2);
	enemy->pos.y = static_cast<float>((AERandFloat() * AEGfxGetWindowHeight()) - AEGfxGetWindowHeight() / 2);
	enemy->scale = 67;
	enemy->xp = 5;
	enemy->rotation = static_cast<float>(AERandFloat() * 360);
	enemy->alive = true;
	enemy->hp = 5;
}

void LoadDebug3() {
	// load font
	boldPixels = AEGfxCreateFont("Assets/BoldPixels.ttf", 72);
	DrawEnemyMesh();
	
	for (int i = 0; i < 20; i++) {
		InitEnemies(&maxenemy[i]);
	}

}

void DrawDebug3() {

	// gray bg
	AEGfxSetBackgroundColor(0.82f, 0.82f, 0.82f);
	for (int i{ 0 }; i < NUM_OF_TEXTS; ++i) {
		Graphics::printText(texts[i], boldPixels);
	}

	dt = AEFrameRateControllerGetFrameTime();
	timer += AEFrameRateControllerGetFrameTime();

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);

	AEGfxSetColorToMultiply(0.8f, 0.2f, 0.2f, 1.0f);


	if (timer >= 1) {
		if (AERandFloat() * 10 >= 2) {
			for (int i = 0; i < 20; i++) {

				if (!maxenemy[i].alive) {
					SpawnEnemies(&maxenemy[i]);
					//AEGfxSetCamPosition(maxenemy[i].pos_x, maxenemy[i].pos_y);
					break;
				}
			}
		}

		timer = 0;
	}
	for (int i = 0; i < 20; i++) {

		if (maxenemy[i].alive) {
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

	//separateEnemies();
	applyPhysicsSeparation(dt);

	// Update enemy positions and velocities
	updateEnemyPhysics(dt);
	
}

void FreeDebug3() {
	// free font
	AEGfxDestroyFont(boldPixels);
	AEGfxMeshFree(MeshRect);
}