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
#include "unittest6.h"



//Add new states here
enum StateName {
	STATE_Initialize,	//Note: the first enum is the starting state
	STATE_Chain1,
	STATE_Chain2,
	STATE_Chain3,
	STATE_Chain4,
	STATE_Chain5,
	STATE_Chain6,
	STATE_Chain7,
	STATE_Chain8,
	STATE_Chain9,
	STATE_Chain10,
	STATE_Success,
	STATE_Broken
};

//Add new substates here
enum SubstateName {
	SUBSTATE_Inside1,	//Note: none of these substates will be active until explicitly transitioned to
	SUBSTATE_Inside2,
	SUBSTATE_Inside3
};

//unittest6 covers:
//OnEitherMsg, OnBothMsg
//OnNthUpdate
//OnFirstUpdate - OnFifthUpdate
//OnEveryNthUpdate, OnEveryOddUpdate, OnEveryEvenUpdate



bool UnitTest6::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			ChangeState( STATE_Chain1 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain1 )

		DeclareStateInt( count )

		OnEnter
			//Test that count is initialized to zero by default
			if( count != 0 ) {
				ChangeState( STATE_Broken );
			}

		OnFirstUpdate
			SendMsgToState( MSG_UnitTestMessage2 );
			SendMsgDelayedToState( 0.1f, MSG_UnitTestMessage3 );
			ChangeStateDelayed( 1.0f, STATE_Broken );

		OnMsg( MSG_UnitTestMessage3 )
			if( count != 1 ) {
				ChangeState( STATE_Broken );
			}
			EscapeWithoutConsumingMsg

		OnBothMsg( MSG_UnitTestMessage3, MSG_UnitTestMessage3 )
			count++;
			EscapeWithoutConsumingMsg

		OnBothMsg( MSG_UnitTestMessage2, MSG_UnitTestMessage3 )
			if( count != 2 ) {
				ChangeState( STATE_Broken );
			}
			ChangeState( STATE_Chain2 );	//Received both messages

		OnBothMsg( MSG_UnitTestMessage3, MSG_UnitTestMessage2 )
			ChangeState( STATE_Broken );	//Received both messages

		OnMsg( MSG_UnitTestMessage2 )
			count++;


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain2 )

		DeclareStateInt( count )

		OnFirstUpdate
			SendMsgToState( MSG_UnitTestMessage4 );
			SendMsgDelayedToState( 0.1f, MSG_UnitTestMessage5 );
			ChangeStateDelayed( 1.0f, STATE_Broken );

		OnEitherMsg( MSG_UnitTestMessage4, MSG_UnitTestMessage5 )
			count++;
			if( count == 2 ) {
				ChangeState( STATE_Chain3 );	//Received both messages
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain3 )

		DeclareStateInt( count )
		DeclareStateInt( noncount )

		OnNthUpdate(1)
			count++;
		OnNthUpdate(3)
			count++;
		OnNthUpdate(7)
			count++;
		OnNthUpdate(11)
			count++;
		OnNthUpdate(13)
			if( count == 4 && noncount == 8 ) {
				ChangeState( STATE_Chain4 );
			}
			else {
				ChangeState( STATE_Broken );
			}
		OnUpdate
			noncount++;


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain4 )

		DeclareStateInt( count )
		DeclareStateInt( noncount )

		OnFirstUpdate
			count++;
		OnSecondUpdate
			noncount++;
		OnThirdUpdate
			count++;
		OnFourthUpdate
			count++;
		OnFifthUpdate
			if( count == 3 && noncount == 1 ) {
				ChangeState( STATE_Chain5 );
			}
			else {
				ChangeState( STATE_Broken );
			}
		OnUpdate
			count++;
			noncount++;


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain5 )

		DeclareStateInt( count )

		OnEveryNthUpdate(3)
			count++;

		OnNthUpdate(31)
			if( count == 10 ) {
				ChangeState( STATE_Chain6 );
			}
			else {
				ChangeState( STATE_Broken );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain6 )
		
		DeclareStateBool( evenhit )
		DeclareStateBool( oddhit )
		DeclareStateInt( count )

		OnNthUpdate(3)
			if( evenhit && oddhit ) {
				ChangeState( STATE_Chain7 );
			}
			else {
				ChangeState( STATE_Broken );
			}

		OnEveryOddUpdate
			oddhit = true;
			if( evenhit ) {
				ChangeState( STATE_Broken );
			}

		OnEveryEvenUpdate
			evenhit = true;
			if( !oddhit ) {
				ChangeState( STATE_Broken );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain7 )

		DeclareStateBool( odd )

		OnNthUpdate(10)
			if( odd ) {
				ChangeState( STATE_Chain8 );
			}
			else {
				ChangeState( STATE_Broken );
			}
			EscapeWithoutConsumingUpdate

		OnEveryOddUpdate
			odd = !odd;

		OnEveryEvenUpdate
			odd = !odd;


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain8 )

		DeclareStateInt( count )

		OnEnter
			SendMsgDelayedToState( 0.1f, MSG_UnitTestMessage );
			SendMsgDelayedToState( 0.2f, MSG_UnitTestMessage2 );
			SendMsgDelayedToState( 0.3f, MSG_UnitTestMessage3 );
			SendMsgDelayedToState( 1.0f, MSG_UnitTestBroken );

		OnMsg( MSG_UnitTestBroken )
			ChangeState( STATE_Broken );

		OnAnyMsg
			count++;
			if( count == 3 ) {
				ChangeState( STATE_Chain9 );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain9 )

		DeclareStateInt( count )

		OnEnter
			ChangeStateDelayed( 1.5f, STATE_Broken );

		OnTimeInState( 0.1f )
			count++;

		OnTimeInState( 0.2f )
			if( count == 1 ) {
				ChangeState( STATE_Chain10 );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain10 )

		DeclareStateInt( count )

		OnEnter
			ChangeStateDelayed( 2.0f, STATE_Broken );

		OnPeriodicTimeInState( 0.15f )
			count++;

		OnPeriodicTimeInState( 0.1f )
			count++;
			if( count == 6 ) {
				ChangeState( STATE_Success );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Success )

		OnEnter
			OutputDebugString( L"UnitTest6 Success\n" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Broken )

		OnEnter
			OutputDebugString( L"UnitTest6 Broken\n" );


EndStateMachine
}