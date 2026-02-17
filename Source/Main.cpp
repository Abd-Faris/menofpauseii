/*****************************************************************************/
/*!
\file	Main.cpp
\author Men of Pause II
\brief	This file defines the beginning of the program and frame based loops
*/
/*****************************************************************************/

// csd1130_gsm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "MasterHeader.h"

//-----------------------------------------------------------//
// This function defines the beginning of the program and
// implements frame loops in the program.
//-----------------------------------------------------------//
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    // System initialize
    Initialise_System(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    // GSM initialize
    GSM_Initialize(GS_MAIN_MENU);

    // Game Loop
    while (GS_current != GS_QUIT)
    {
        if (GS_current != GS_RESTART) {
            GSM_Update();
            if(GS_Functions.Load) GS_Functions.Load();
        }
        else { // if current = GS_RESTART
            // set all to prev game state
            GS_next = GS_previous;
            GS_current = GS_previous;
        }
        // init game state
        if(GS_Functions.Initialize) GS_Functions.Initialize();

        // the game loop
        while(GS_next == GS_current){
            AESysFrameStart(); // start of game frame
            
            // Update & Draw Functions
            if(GS_Functions.Update) GS_Functions.Update();
            if(GS_Functions.Draw) GS_Functions.Draw();

            // for access to debug screens
            Debug_States();
            AESysFrameEnd(); // end of game frame
        }

        // more steps
        if(GS_Functions.Free) GS_Functions.Free();

        // unloads level
        if (GS_next != GS_RESTART){
            if(GS_Functions.Unload) GS_Functions.Unload();
        }
        // shifts game states over by one
        GS_previous = GS_current;
        GS_current = GS_next;
    }

    //Systems exit (terminate)
    //System_Exit();
    return 0;
}
