#include "MasterHeader.h"

namespace World {
	//its currently set to twice our screen size
	int mapGrid[WORLD_ROWS][WORLD_COLS];
	static AEGfxVertexList* pWallMesh = nullptr;
	static AEGfxTexture* pWallTex = nullptr;

	void Load_World() {
		if (pWallTex != nullptr || pWallMesh != nullptr) return;
		pWallTex = AEGfxTextureLoad("./Assets/wall.png");

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

	bool CheckCollision(float x, float y) {
		//
		float tank_radius = GameConfig::Tank::SCALE / 2.0f;
		//check the four corners of the tank's bounding box for collision with walls or world borders
		
		float pointsX[] = { x + tank_radius, x - tank_radius, x,x };
		float pointsY[] = { y, y, y + tank_radius, y - tank_radius };

		//loop through each corner point and check for collision
		for (int i = 0; i < 4; i++) {
			int col = static_cast<int>((pointsX[i] + HALF_WIDTH) / TILE_SIZE);
			int row = static_cast<int>((HALF_HEIGHT - pointsY[i]) / TILE_SIZE);

			//check if the point is outside the world boundaries or if it collides with a wall tile
			if (row < 0 || row >= WORLD_ROWS || col < 0 || col >= WORLD_COLS) {
				return true;
			}
			if (mapGrid[row][col] == 1) {
				return true;
			}
		}
		//if no collide
		return false;
	}

	void Draw_World(){
		if (pWallMesh == nullptr || pWallTex == nullptr) {
			return;
		}

		//set up the graphics state for drawing the world
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEGfxTextureSet(pWallTex, 0.0f, 0.0f);
		AEGfxSetTransparency(1.0f);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

		//loop through each row and column of the map grid
		for (int row = 0; row < WORLD_ROWS; row++) {
			for (int col = 0; col < WORLD_COLS; col++) {
				if (mapGrid[row][col] == 1) {
					AEVec2 position;
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
	}
}