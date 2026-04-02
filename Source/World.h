// -----------------------------Gloomy's Revenge---------------------------- //
// File:	World.h
// Authors:	[Men of Pause II]
// Brief:	World dimensions, tile constants, and collision detection system.
// ------------------------------------------------------------------------- //

#include "MasterHeader.h"

namespace World {

	// =============================================================================
	// WORLD CONSTANTS
	// =============================================================================

	// initialise world dimensions and tile size
	const int WORLD_COLS = 58;
	const int WORLD_ROWS = 32;
	const float TILE_SIZE = 70.0f;

	// half dimensions for collision checks
	const float HALF_WIDTH = (WORLD_COLS * TILE_SIZE) / 2.0f;
	const float HALF_HEIGHT = (WORLD_ROWS * TILE_SIZE) / 2.0f;

	// =============================================================================
	// WORLD & COLLISION FUNCTIONS
	// =============================================================================

	// ~ Brief: Loads world-related assets, such as tile textures or meshes.
	void Load_World();

	// ~ Brief: Initializes the world state, grid layout, or object positions.
	void Init_World();

	// ~ Brief: Renders the world tiles and environment to the screen.
	void Draw_World();

	// ~ Brief: Frees world assets from memory to prevent leaks.
	void Free_World();

	// ~ Brief: Checks for a collision at a specific position considering scale and rotation.
	bool CheckCollision(float x, float y, float playerScale, float playerRotation);

	// ~ Brief: Simple bounding box check for the player's body against world objects.
	bool CheckBodyCollision(float x, float y, float playerScale);

	// ~ Brief: Checks if a specific point in world coordinates overlaps with a wall/tile.
	bool isPointColliding(float worldX, float worldY);

	// ~ Brief: Resolves collision by pushing an object out of a wall based on its radius.
	void PushOutOfWalls(float& x, float& y, float radius);
}