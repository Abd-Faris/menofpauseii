#include "MasterHeader.h"

namespace World {
	//initialise world dimensions and tile size
	const int WORLD_COLS = 54;
	const int WORLD_ROWS = 30;
	const float TILE_SIZE = 60.0f;

	//half dimensions for collision checks
	const float HALF_WIDTH = (WORLD_COLS * TILE_SIZE) / 2.0f;
	const float HALF_HEIGHT = (WORLD_ROWS * TILE_SIZE) / 2.0f;

	//functions
	void Load_World();
	void Init_World();
	void Draw_World();
	void Free_World();
	bool CheckCollision(float x, float y);
}
