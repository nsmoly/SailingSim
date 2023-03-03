/* Copyright Steve Rabin, 2008. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2008"
 */


#include "DXUT.h"
#include "example.h"
#include "database.h"




//Add new states here
enum StateName {
	STATE_Initialize,	//Note: the first enum is the starting state
	STATE_RenameThisState1,
	STATE_RenameThisState2,
	STATE_RenameThisState3
};

//Add new substates here
enum SubstateName {
	SUBSTATE_RenameThisSubstate1,
	SUBSTATE_RenameThisSubstate2,
	SUBSTATE_RenameThisSubstate3
};

bool Example::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses go here

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			
	
	///////////////////////////////////////////////////////////////
	DeclareState( STATE_RenameThisState1 )

		OnEnter


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_RenameThisState2 )

		OnEnter


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_RenameThisState3 )

		OnEnter


EndStateMachine
}


