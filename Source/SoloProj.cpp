// ---------------------------------------------------------------------------
// includes

#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h" //literally the game engine lmao

//Enum and Global Variables
enum { SPEED = 1000 ,
	   NUM_OF_OBJECTS = 6
	 };
float percent{ 100.f };

// Object Struct
typedef struct Object {
	// Children
	f32 xscale{ 0 }, yscale{ 0 };
	f32 xposition{ 0 }, yposition{ 0 };
	// rgba defaults to black
	f32 r = 0.f, g = 0.f, b = 0.f, a = 1.f;
	// Functions
	void moveUp(f32 dt) { yposition += SPEED * dt; }
	void moveDown(f32 dt) { yposition -= SPEED * dt; }
	void moveLeft(f32 dt) { xposition -= SPEED * dt; }
	void moveRight(f32 dt) { xposition += SPEED * dt; }
} Object;

Object* initialiseObjects(Object *object) {
	//blue player
	Object& player{ object[2] };
	player.xscale = player.yscale = 60.f;
	player.xposition = 0.f;
	player.yposition = -50.f;
	player.b = 1.f;

	//areas of effects
	//red damage
	Object& damage{ object[1] };
	damage.xscale = damage.yscale = 400.f;
	damage.xposition = 400.f;
	damage.yposition = -50.f;
	damage.r = 1.f;
	//green heal
	Object& health{ object[0] };
	health.xscale = health.yscale = 400.f;
	health.xposition = -400.f;
	health.yposition = -50.f;
	health.g = 1.f;

	//healthbar
	//health bg
	Object& healthbg{ object[3] };
	healthbg.xscale = 1300.f;
	healthbg.yscale = 50.f;
	healthbg.xposition = -650.f;
	healthbg.yposition = 320.f;
	healthbg.r = 0.5f;
	//health bar
	Object& healthbar{ object[4] };
	healthbar.xscale = percent * 1300.f;
	healthbar.yscale = 50.f;
	healthbar.xposition = -650.f;
	healthbar.yposition = 320.f;
	healthbar.r = 1.f;
	//health bar rectangle
	Object& healthrect{ object[5] };
	healthrect.xscale = 1300.f / 19.f;
	healthrect.yscale = 50.f;
	healthrect.xposition = -650.f;
	healthrect.yposition = 220.f;
	healthrect.r = 1.f;

	return object;
}

void drawObjects(Object *object, int size, AEGfxVertexList* pMesh) {

	for (int i = 0; i < size; ++i) {
		Object& obj{ object[i] };
		// Scale matrix
		AEMtx33 scale = { 0 };
		AEMtx33Scale(&scale, obj.xscale, obj.yscale);

		// Translation Matrix
		AEMtx33 translate{ 0 };
		AEMtx33Trans(&translate, obj.xposition, obj.yposition);

		// Final Transformation Matrix
		AEMtx33 transform = { 0 };
		AEMtx33Concat(&transform, &translate, &scale);
		
		// apply transformation to pMesh
		AEGfxSetTransform(transform.m);

		// to print each area colour
		AEGfxSetColorToMultiply(obj.r, obj.g, obj.b, obj.a);
		AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
	}
}

void drawRect(Object rect, int number, AEGfxVertexList* rectMesh) {
	// Scale matrix
	AEMtx33 scale = { 0 };
	AEMtx33Scale(&scale, rect.xscale, rect.yscale);

	// Translation Matrix
	AEMtx33 translate{ 0 };
	AEMtx33Trans(&translate, (rect.xposition + ((1300.f / 19.f) * 2 * number)), rect.yposition);

	// Final Transformation Matrix
	AEMtx33 transform = { 0 };
	AEMtx33Concat(&transform, &translate, &scale);

	// apply transformation to pMesh
	AEGfxSetTransform(transform.m);

	// to print each area colour
	AEGfxSetColorToMultiply(rect.r, rect.g, rect.b, rect.a);
	AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
}

// new collision, currently using
int circlesIntersect(Object &player, Object &aoe) {
	// Determine squared distance between center of 2 circles
	f32 xdistance = player.xposition - aoe.xposition;
	xdistance *= xdistance;
	f32 ydistance = player.yposition - aoe.yposition;
	ydistance *= ydistance;
	f32 xydistance = xdistance + ydistance;

	// Determine sum of both circle radius squared
	f32 sumOfRadius = (player.xscale / 2.f) + (aoe.xscale / 2.f);
	sumOfRadius *= sumOfRadius;

	// If xydistance < sumOfRadius, circles intersect
	return xydistance < sumOfRadius ? 1 : 0;
}

// old collision, unused
int squaresIntersect(Object &player, Object &aoe) {
	
	// Determine limits of player
	f32 pTop{ player.yposition + (player.yscale / 2.f) };
	f32 pBottom{ player.yposition - (player.yscale / 2.f) };
	f32 pRight{ player.xposition + (player.xscale / 2.f) };
	f32 pLeft{ player.xposition - (player.xscale / 2.f) };
	
	// Determine limits of aoe
	f32 top{ aoe.yposition + (aoe.yscale / 2.f) };
	f32 bottom{ aoe.yposition - (aoe.yscale / 2.f) };
	f32 right{ aoe.xposition + (aoe.xscale / 2.f) };
	f32 left{ aoe.xposition - (aoe.xscale / 2.f) };
	if (pLeft < right && pRight > left && pTop > bottom && pBottom < top) return 1;
	return 0;
}

// ---------------------------------------------------------------------------
// main

int APIENTRY wWinMain2(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// set gamestate to "Running"
	int gGameRunning = 1;

	// Initialization of your own variables go here

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);

	// Changing the window title
	AESysSetWindowTitle("UI Demo!");

	// reset the system modules
	AESysReset();

	printf("Hello World\n");

	//inits mesh ptr
	AEGfxVertexList *pMesh = 0, *rectMesh = 0;

	//this is the part where i make a circle mesh and definitely 
	//dont crash out :)
	
	// how many straight lines does it take to make a circular object?
	// find out after the break!!! stay tuned1!!
	constexpr int edges{ 40 };
	AEGfxMeshStart(); // let the pain begin
	
	// for loop to create meshes at diff rotations to form a circle
	for (int i{ 0 }; i < edges; ++i) {
		float angle1 = (i / float(edges)) * TWO_PI;
		float angle2 = ((i + 1) / float(edges)) * TWO_PI;

		// define triangle mesh with centre pts (0,0)
		AEGfxTriAdd(
			0.f, 0.f, 0xFFFFFFFF, 0.5f, 0.5f, // Centre point
			AECos(angle1) * 0.5f, AESin(angle1) * 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
			AECos(angle2) * 0.5f, AESin(angle2) * 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	}
	// assign mesh to pMesh and marvel at a round object
	// such teknologi much wow
	// we truly live in a society
	pMesh = AEGfxMeshEnd();

	// quadrilateral mesh, left align
	AEGfxMeshStart();
	AEGfxTriAdd(
		0.f, 0.f, 0xFFFFFFFF, 0.0f, 1.0f,
		1.f, 0.f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.f, 1.f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(
		1.f, 0.f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.f, 1.f, 0xFFFFFFFF, 0.0f, 0.0f,
		1.f, 1.f, 0xFFFFFFFF, 1.0f, 0.0f);

	rectMesh = AEGfxMeshEnd();

	// create array of objects to print
	Object object[NUM_OF_OBJECTS] = { 0 };
	initialiseObjects(object);
	// aliases for easy referencing
	Object& player{ object[2] };
	Object& damage{ object[1] };
	Object& health{ object[0] };
	Object& healthbar{ object[4] };

	

	// Game Update Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();
		// get delta time
		f32 deltaTime = f32(AEFrameRateControllerGetFrameTime());

		// Basic way to trigger exiting the application
		// when ESCAPE is hit or when the window is closed
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
		{ gGameRunning = 0; }

		// Your own update logic goes here
		// movement logic here
		if (AEInputCheckCurr(AEVK_W)) player.moveUp(deltaTime);
		if (AEInputCheckCurr(AEVK_A)) player.moveLeft(deltaTime);
		if (AEInputCheckCurr(AEVK_S)) player.moveDown(deltaTime);
		if (AEInputCheckCurr(AEVK_D)) player.moveRight(deltaTime);
		// collision detection
		if (percent > 0.f && circlesIntersect(player, damage)) percent -= .25f;
		if (percent < 100.f && circlesIntersect(player, health)) percent += .25f;
		// clamps healthbar should it exceed limits
		if (percent < 0) percent = 0;
		if (percent > 100) percent = 100;
		// apply healthbar change in size
		healthbar.xscale = (percent/100) * 1300.f;

		// Your own rendering logic goes here
		// gray bg
		AEGfxSetBackgroundColor(211.f / 255.f, 211.f / 255.f, 211.f / 255.f);
		// object drawing settings
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
		AEGfxSetTransparency(1.f);

		// Object Drawing
		drawObjects(object, 3, pMesh); // player and aoe
		drawObjects((object + 3), 2, rectMesh); // healthbar + bg

		// Healthbar rectangles
		// if percent > 0, draw rectangles. else, dont draw
		if (percent > 0){
			Object& rectangle{ object[5] };
			switch (static_cast<int>(percent / 10 + EPSILON)) {
			case 10:
			case 9: drawRect(rectangle, 9, rectMesh);
			case 8: drawRect(rectangle, 8, rectMesh);
			case 7: drawRect(rectangle, 7, rectMesh);
			case 6: drawRect(rectangle, 6, rectMesh);
			case 5: drawRect(rectangle, 5, rectMesh);
			case 4: drawRect(rectangle, 4, rectMesh);
			case 3: drawRect(rectangle, 3, rectMesh);
			case 2: drawRect(rectangle, 2, rectMesh);
			case 1: drawRect(rectangle, 1, rectMesh);
			case 0: drawRect(rectangle, 0, rectMesh);
			default: break;
			}
		}

		// Informing the system about the loop's end
		AESysFrameEnd();

	}


	// free the system
	AESysExit();
	return 0;
}