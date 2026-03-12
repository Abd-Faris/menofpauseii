#include "MasterHeader.h"

namespace World {
	//its currently set to twice our screen size
	int mapGrid[WORLD_ROWS][WORLD_COLS];
	static AEGfxVertexList* pWallMesh = nullptr;
	static AEGfxTexture* pWallTex = nullptr;
	static AEGfxTexture* pGroundTex = nullptr;

	void Load_World() {
		if (pWallTex != nullptr || pGroundTex != nullptr || pWallMesh != nullptr) return;
		pWallTex = AEGfxTextureLoad("./Assets/wall.png");
		pGroundTex = AEGfxTextureLoad("./Assets/sand.png");

		AEGfxMeshStart();
		AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
		AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
		pWallMesh = AEGfxMeshEnd();
	}

	void Init_World() {
		//loop through each row and column of the map grid
		for (int row = 0; row < WORLD_ROWS; row++) {
			for (int col = 0; col < WORLD_COLS; col++) {
				//check if the current cell is on the border
				//if yes, set it to 1, otherwise set it to 0
				if (row == 0 || row == WORLD_ROWS - 1 || col == 0 || col == WORLD_COLS - 1) {
					//border wall
					mapGrid[row][col] = 1;
				}
				else {
					//empty space
					mapGrid[row][col] = 0;
				}
			}
		}
	}


	bool isPointColliding(float worldX, float worldY) {
		//convert world coordinates to grid indices
		int col = static_cast<int>((worldX + World::HALF_WIDTH) / World::TILE_SIZE);
		int row = static_cast<int>((World::HALF_HEIGHT - worldY) / World::TILE_SIZE);

		//check if the indices are out of bounds, if yes, treat it as a collision
		if (col < 0 || col >= World::WORLD_COLS || row < 0 || row >= World::WORLD_ROWS) return true;
		return (World::mapGrid[row][col] == 1);
	}


	bool CheckCollision(float x, float y, float playerScale, float playerRotation) {
		//corners of the tank and barrel in local space 
		float h = (playerScale * 0.9f);
		AEVec2 tankPoints[4] = {
			{ h,  h }, { h, -h },
			{-h,  h }, {-h, -h }
		};

		float barrelLen = GameConfig::Tank::BARREL_LENGTH * (playerScale / GameConfig::Tank::SCALE);
		float halfBWidth = 11.0f;

		AEVec2 barrelPoints[4] = {
		{ -halfBWidth, barrelLen }, 
		{  halfBWidth, barrelLen }, 
		{ -halfBWidth, barrelLen * 0.5f }, 
		{  halfBWidth, barrelLen * 0.5f }
		};

		float cosA = cosf(playerRotation);
		float sinA = sinf(playerRotation);

		//rotate and translate each point to world space, then check for collision
		for (int i = 0; i < 4; i++) {
			float worldX = x + (tankPoints[i].x * cosA - tankPoints[i].y * sinA);
			float worldY = y + (tankPoints[i].x * sinA + tankPoints[i].y * cosA);

			if (isPointColliding(worldX, worldY)) return true;
		}
		//also check the barrel points for collision
		for (int i = 0; i < 4; i++) {
			float worldX = x + (barrelPoints[i].x * cosA - barrelPoints[i].y * sinA);
			float worldY = y + (barrelPoints[i].x * sinA + barrelPoints[i].y * cosA);

			if (isPointColliding(worldX, worldY)) return true;
		}

		return false;
	}


	void Draw_World(){

		//set up the graphics state for drawing the world
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxSetTransparency(1.0f);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

		//loop through each row and column of the map grid
		for (int row = 0; row < WORLD_ROWS; row++) {
			for (int col = 0; col < WORLD_COLS; col++) {
				if (mapGrid[row][col] == 1) {
					AEGfxTextureSet(pWallTex, 0, 0);
				}
				else { 
					AEGfxTextureSet(pGroundTex, 0, 0);
				}

				AEVec2 position{};
					//the position is calculated by taking the column and row indices, 
					//multiplying them by the tile size, and then adjusting for the center 
					//of the tile and the entire world dimension
					position.x = (col * TILE_SIZE) - HALF_WIDTH + (TILE_SIZE / 2.0f);
					position.y = HALF_HEIGHT - (row * TILE_SIZE) - (TILE_SIZE / 2.0f);

					AEMtx33 scale, trans;
					AEMtx33Scale(&scale, TILE_SIZE, TILE_SIZE);
					AEMtx33Trans(&trans, position.x, position.y);
					AEMtx33 finalTransform;
					AEMtx33Concat(&finalTransform, &trans, &scale);
					AEGfxSetTransform(finalTransform.m);
					AEGfxMeshDraw(pWallMesh, AE_GFX_MDM_TRIANGLES);
					
			
			}
		}
	}


	void Free_World() {
		if (pWallMesh != nullptr) {
			AEGfxMeshFree(pWallMesh);
			pWallMesh = nullptr;
		}
		if (pWallTex != nullptr) {
			AEGfxTextureUnload(pWallTex);
			pWallTex = nullptr;
		}
		if (pGroundTex != nullptr) {
			AEGfxTextureUnload(pGroundTex);
			pGroundTex = nullptr;
		}
	}
}