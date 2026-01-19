/*****************************************************************************/
/*!
\file	Input.cpp
\author Men of Pause II
\brief	This file defines the beginning of the program and frame based loops
*/
/*****************************************************************************/

// csd1130_gsm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "GameStateManager.h"
#include "GameStateList.h"

//Implement the pseudo-code of A1_P1 here
//-----------------------------------------------------------//
// This function defines the beginning of the program and
// implements frame loops in the program.
//-----------------------------------------------------------//
int main()
{
    //Systems initialize
    //System_Initialize();
    //GSM initialize
    GSM_Initialize(GS_LEVEL1);
    while (current != GS_QUIT)
    {
        if (current != GS_RESTART) {
            GSM_Update();
            fpLoad();
        }
        else { // if current = GS_QUIT
            // set all to prev game state
            next = previous;
            current = previous;
        }
        // init game state
        fpInitialize();
        // the game loop
        while(next == current){
            //Input_Handle();
            fpUpdate();
            fpDraw();
        }

        // more steps
        fpFree();

        // unloads level
        if (next != GS_RESTART){
            fpUnload();
        }
        // shifts game states over by one
        previous = current;
        current = next;
        //break;//this break line is temporary here - MUST BE REMOVED
    }

    //Systems exit (terminate)
    //System_Exit();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
