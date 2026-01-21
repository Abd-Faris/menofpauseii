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
    while (current != GS_QUIT)
    {
        if (current != GS_RESTART) {
            GSM_Update();
            if(fpLoad) fpLoad();
        }
        else { // if current = GS_RESTART
            // set all to prev game state
            next = previous;
            current = previous;
        }
        // init game state
        if(fpInitialize) fpInitialize();

        // the game loop
        while(next == current){
            AESysFrameStart(); // start of game frame
            //input_handler();
            if(fpUpdate) fpUpdate();
            if(fpDraw) fpDraw();
            Debug_States(); // detect game state change
            AESysFrameEnd(); // end of game frame
        }

        // more steps
        if(fpFree) fpFree();

        // unloads level
        if (next != GS_RESTART){
            if(fpUnload) fpUnload();
        }
        // shifts game states over by one
        previous = current;
        current = next;
    }

    //Systems exit (terminate)
    //System_Exit();
    return 0;
}
