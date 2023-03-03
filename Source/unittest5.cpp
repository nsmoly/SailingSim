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
#include "unittest5.h"


//Not in unit tests yet:
//OnEitherMsg
//OnNthUpdate
//OnFirstUpdate - OnFifthUpdate
//OnEveryNthUpdate, OnEveryOddUpdate, OnEveryEvenUpdate
//SendMsgDelayedToSingleQueue
//SendMsgDelayedToAllQueues
//SendMsgDelayedToAllOtherQueues
//DeleteStateMachineQueue
//IsChangeStateDelayedQueued
//IsChangeSubstateDelayedQueued
//msg data objectID, pointer, vector2, vector3


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

//unittest5 covers:
//DeclareStateInt, DeclareStateFloat, DeclareStateBool, DeclareStateObjectID
//DeclareSubstateInt, DeclareSubstateFloat, DeclareSubstateBool, DeclareSubstateObjectID

bool UnitTest5::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			ChangeState( STATE_Chain1 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain1 )

		DeclareStateInt( testVariableXXXXX )
		DeclareStateInt( testVariable )
		DeclareStateInt( testVariable2 )

		OnEnter
			testVariable = 5;

		OnFirstUpdate
			int v = 0;
			int v2 = 0;
			v = testVariable;		//Test casting and persistence
			if( v != 5 ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable = 10;	//Test =
			if( v != 10 ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable + 1;	//Test +
			if( v != 11 ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable - 1;	//Test -
			if( v != 9 ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable * 10;	//Test *
			if( v != 100 ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable / 10;	//Test /
			if( v != 1 ) { ChangeState( STATE_Broken ); return true; }
			testVariable += 10;		//Test +=
			v = testVariable;
			if( v != 20 ) { ChangeState( STATE_Broken ); return true; }
			testVariable -= 10;		//Test -=
			v = testVariable;
			if( v != 10 ) { ChangeState( STATE_Broken ); return true; }
			testVariable *= 10;		//Test *=
			v = testVariable;
			if( v != 100 ) { ChangeState( STATE_Broken ); return true; }
			testVariable /= 10;		//Test /=
			v = testVariable;
			if( v != 10 ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable++;		//Test post increment ++
			v2 = testVariable;
			if( v != 10  || v2 != 11 ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable--;		//Test post decrement --
			v2 = testVariable;
			if( v != 11  || v2 != 10 ) { ChangeState( STATE_Broken ); return true; }
			v = ++testVariable;		//Test pre increment ++
			v2 = testVariable;
			if( v != 11  || v2 != 11 ) { ChangeState( STATE_Broken ); return true; }
			v = --testVariable;		//Test pre decrement --
			v2 = testVariable;
			if( v != 10  || v2 != 10 )			{ ChangeState( STATE_Broken ); return true; }
			if( testVariable < 10 )				{ ChangeState( STATE_Broken ); return true; }	//Test <
			if( !(testVariable < 11) )			{ ChangeState( STATE_Broken ); return true; }	//Test <
			if( testVariable > 10 )				{ ChangeState( STATE_Broken ); return true; }	//Test >
			if( !(testVariable > 9) )			{ ChangeState( STATE_Broken ); return true; }	//Test >
			if( !(testVariable <= 10) )			{ ChangeState( STATE_Broken ); return true; }	//Test <=
			if( !(testVariable >= 10) )			{ ChangeState( STATE_Broken ); return true; }	//Test >=
			if( testVariable != 10 )			{ ChangeState( STATE_Broken ); return true; }	//Test !=
			if( !(testVariable == 10) )			{ ChangeState( STATE_Broken ); return true; }	//Test ==
			if( (testVariable << 1) != 20 )		{ ChangeState( STATE_Broken ); return true; }	//Test <<
			if( (testVariable >> 1) != 5 )		{ ChangeState( STATE_Broken ); return true; }	//Test >>
			testVariable <<= 1;
			if( testVariable != 20 )			{ ChangeState( STATE_Broken ); return true; }	//Test <<=
			testVariable >>= 1;
			if( testVariable != 10 )			{ ChangeState( STATE_Broken ); return true; }	//Test >>=
			if( (testVariable % 3) != 1 ) 		{ ChangeState( STATE_Broken ); return true; }	//Test %
			testVariable %= 3;
			if( testVariable != 1 ) 			{ ChangeState( STATE_Broken ); return true; }	//Test %=
			testVariable = 0x00000001;
			testVariable2 = 0x00000011;
			if( (testVariable & testVariable2) != 0x00000001 )	{ ChangeState( STATE_Broken ); return true; }	//Test &
			if( (testVariable | testVariable2) != 0x00000011 )	{ ChangeState( STATE_Broken ); return true; }	//Test |
			if( (testVariable ^ testVariable2) != 0x00000010 )	{ ChangeState( STATE_Broken ); return true; }	//Test ^
			testVariable &= testVariable2;
			if( testVariable != 0x00000001 )	{ ChangeState( STATE_Broken ); return true; }	//Test &=
			testVariable = 0x00000001;
			testVariable |= testVariable2;
			if( testVariable != 0x00000011 )	{ ChangeState( STATE_Broken ); return true; }	//Test |=
			testVariable = 0x00000001;
			testVariable ^= testVariable2;
			if( testVariable != 0x00000010 )	{ ChangeState( STATE_Broken ); return true; }	//Test ^=
			testVariable = 0x00000001;
			if( (~testVariable != 0xFFFFFFFE) )	{ ChangeState( STATE_Broken ); return true; }	//Test ~
			ChangeState( STATE_Chain2 );



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain2 )

		DeclareStateFloat( testVariable )

		OnEnter
			testVariable = 5.0f;

		OnFirstUpdate
			float v = 0;
			float v2 = 0;
			v = testVariable;			//Test casting and persistence
			if( v != 5.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable = 10;		//Test =
			if( v != 10.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable = 10.0f;	//Test =
			if( v != 10.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable + 1;		//Test +
			if( v != 11.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable + 1.0f;	//Test +
			if( v != 11.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable - 1;		//Test -
			if( v != 9.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable - 1.0f;	//Test -
			if( v != 9.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable * 10;		//Test *
			if( v != 100.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable * 10.0f;	//Test *
			if( v != 100.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable / 10;		//Test /
			if( v != 1.0f ) { ChangeState( STATE_Broken ); return true; }
			v = testVariable / 10.0f;	//Test /
			if( v != 1.0f ) { ChangeState( STATE_Broken ); return true; }
			testVariable += 10;			//Test +=
			testVariable += 10.0f;		//Test +=
			v = testVariable;
			if( v != 30.0f ) { ChangeState( STATE_Broken ); return true; }
			testVariable -= 10;			//Test -=
			testVariable -= 10.0f;		//Test -=
			v = testVariable;
			if( v != 10.0f ) { ChangeState( STATE_Broken ); return true; }
			testVariable *= 10;			//Test *=
			testVariable *= 10.0f;		//Test *=
			v = testVariable;
			if( v != 1000.0f ) { ChangeState( STATE_Broken ); return true; }
			testVariable /= 10;			//Test /=
			testVariable /= 10.0f;		//Test /=
			v = testVariable;
			if( v != 10.0f ) { ChangeState( STATE_Broken ); return true; }
			testVariable = 10.0f;
			if( testVariable < 10 )			{ ChangeState( STATE_Broken ); return true; }	//Test <
			if( testVariable < 10.0f )		{ ChangeState( STATE_Broken ); return true; }	//Test <
			if( !(testVariable < 11) )		{ ChangeState( STATE_Broken ); return true; }	//Test <
			if( !(testVariable < 11.0f) )	{ ChangeState( STATE_Broken ); return true; }	//Test <
			if( testVariable > 10 )			{ ChangeState( STATE_Broken ); return true; }	//Test >
			if( testVariable > 10.0f )		{ ChangeState( STATE_Broken ); return true; }	//Test >
			if( !(testVariable > 9) )		{ ChangeState( STATE_Broken ); return true; }	//Test >
			if( !(testVariable > 9.0f) )	{ ChangeState( STATE_Broken ); return true; }	//Test >
			if( !(testVariable <= 10) )		{ ChangeState( STATE_Broken ); return true; }	//Test <=
			if( !(testVariable <= 10.0f) )	{ ChangeState( STATE_Broken ); return true; }	//Test <=
			if( !(testVariable >= 10) )		{ ChangeState( STATE_Broken ); return true; }	//Test >=
			if( !(testVariable >= 10.0f) )	{ ChangeState( STATE_Broken ); return true; }	//Test >=
			if( testVariable != 10 )		{ ChangeState( STATE_Broken ); return true; }	//Test !=
			if( testVariable != 10.0f )		{ ChangeState( STATE_Broken ); return true; }	//Test !=
			if( !(testVariable == 10) )		{ ChangeState( STATE_Broken ); return true; }	//Test ==
			if( !(testVariable == 10.0f) )	{ ChangeState( STATE_Broken ); return true; }	//Test ==
			ChangeState( STATE_Chain3 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain3 )

		DeclareStateBool( testVariable )

		OnEnter
			testVariable = false;

		OnFirstUpdate
			if( testVariable )			{ ChangeState( STATE_Broken ); return true; }	//Test cast
			bool v = testVariable;														//Test =
			if( v )						{ ChangeState( STATE_Broken ); return true; }
			if( testVariable == true )	{ ChangeState( STATE_Broken ); return true; }	//Test ==
			testVariable = true;														//Test !
			if( !testVariable )			{ ChangeState( STATE_Broken ); return true; }
			if( testVariable != true )	{ ChangeState( STATE_Broken ); return true; }	//Test !=
			ChangeState( STATE_Chain4 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain4 )

		DeclareStateObjectID( testVariable )

		OnEnter
			testVariable = m_owner->GetID();

		OnFirstUpdate
			if( testVariable != m_owner->GetID() )		{ ChangeState( STATE_Broken ); return true; }	//Test !=
			ChangeState( STATE_Chain5 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain5 )

		DeclareStatePointerVoid( testVariable )

		OnEnter
			testVariable = (void*)new D3DXVECTOR3(5,5,5);

		OnFirstUpdate
			if( ((D3DXVECTOR3*)(void*)testVariable)->y != 5 )		{ ChangeState( STATE_Broken ); return true; }	//Test !=
			ChangeState( STATE_Chain6 );

		OnExit
			delete testVariable;	//Must delete on exit


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain6 )

		DeclareStatePointerVector2( testVariableVec2 )
		DeclareStatePointerVector3( testVariableVec3 )

		OnEnter
			D3DXVECTOR2* v = new D3DXVECTOR2(0,0);
			testVariableVec2 = v;																	//Test =
			if( testVariableVec2 != v )				{ ChangeState( STATE_Broken ); return true; }	//Test !=
			if( !(testVariableVec2 == v) )			{ ChangeState( STATE_Broken ); return true; }	//Test ==
			testVariableVec2->x = 5.0f;
			testVariableVec3 = new D3DXVECTOR3(10,10,10);											//Test =

		OnFirstUpdate
			float check = testVariableVec2->x;
			if( testVariableVec2->x != 5.0f )		{ ChangeState( STATE_Broken ); return true; }	//Test ->
			if( testVariableVec3->z != 10.0f )		{ ChangeState( STATE_Broken ); return true; }	//Test ->
			ChangeState( STATE_Chain7 );

		OnExit
			delete testVariableVec2;	//Must delete on exit
			delete testVariableVec3;	//Must delete on exit	


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain7 )

		DeclareStateInt( testVariableState )

		OnEnter
			testVariableState = 5;
			ChangeSubstate( SUBSTATE_Inside1 );

		OnUpdate
			if( testVariableState != 5 )				{ ChangeState( STATE_Broken ); return true; }	//Test substate int
			ChangeState( STATE_Chain8 );

		DeclareSubstate( SUBSTATE_Inside1 )

			DeclareSubstateInt( testVariableInt )
			DeclareSubstateBool( testVariableBool )
			DeclareSubstateFloat( testVariableFloat )
			DeclareSubstateObjectID( testVariableObjectID )

			OnEnter
				testVariableInt = 10;
				testVariableBool = true;
				testVariableFloat = 1.0f;
				testVariableObjectID = m_owner->GetID();

			OnFirstUpdate
				if( testVariableInt != 10 )						{ ChangeState( STATE_Broken ); return true; }	//Test substate int
				if( testVariableBool != true )					{ ChangeState( STATE_Broken ); return true; }	//Test substate bool
				if( testVariableFloat != 1.0f )					{ ChangeState( STATE_Broken ); return true; }	//Test substate float
				if( testVariableObjectID != m_owner->GetID() )	{ ChangeState( STATE_Broken ); return true; }	//Test substate objectID
				ChangeSubstate( SUBSTATE_Inside2 );

		DeclareSubstate( SUBSTATE_Inside2 )

			DeclareSubstateInt( testVariableInt )
			DeclareSubstateInt( testVariableBool )
			DeclareSubstateInt( testVariableFloat )
			DeclareSubstateInt( testVariableObjectID )
			
			OnEnter
				testVariableInt = 20;

			OnFirstUpdate
				if( testVariableInt != 20 )				{ ChangeState( STATE_Broken ); return true; }	//Test substate int


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain8 )

		//Mix it up and try and make it break
		DeclareStateBool( testVariableBool1 )
		DeclareStateFloat( testVariableFloat1 )
		DeclareStateInt( testVariableInt1 )
		DeclareStateObjectID( testVariableObjectID1 )

		OnEnter
			testVariableBool1 = true;
			testVariableFloat1 = 1.0f;
			testVariableInt1 = 10;
			testVariableObjectID1 = m_owner->GetID();
			ChangeSubstate( SUBSTATE_Inside1 );

		OnFirstUpdate
			if( testVariableBool1 != true )					{ ChangeState( STATE_Broken ); return true; }
			if( testVariableFloat1 != 1.0f )				{ ChangeState( STATE_Broken ); return true; }
			if( testVariableInt1 != 10 )					{ ChangeState( STATE_Broken ); return true; }
			if( testVariableObjectID1 != m_owner->GetID() )	{ ChangeState( STATE_Broken ); return true; }
			testVariableBool1 = false;
			testVariableFloat1 = -1.0f;
			testVariableInt1 = -10;
			testVariableObjectID1 = (objectID)0;

		OnSecondUpdate
			if( testVariableBool1 != false )				{ ChangeState( STATE_Broken ); return true; }
			if( testVariableFloat1 != -1.0f )				{ ChangeState( STATE_Broken ); return true; }
			if( testVariableInt1 != -10 )					{ ChangeState( STATE_Broken ); return true; }
			if( testVariableObjectID1 != (objectID)0 )		{ ChangeState( STATE_Broken ); return true; }
			
		DeclareSubstate( SUBSTATE_Inside1 )

			OnEnter

			DeclareSubstateInt( testVariableInt )
			DeclareSubstateBool( testVariableBool )

			OnThirdUpdate
				testVariableInt = 10;

			DeclareSubstateFloat( testVariableFloat )
			DeclareSubstateObjectID( testVariableObjectID )

			OnFourthUpdate
				testVariableInt = 10;
				testVariableBool = true;
				testVariableFloat = 1.0f;
				testVariableObjectID = m_owner->GetID();

			OnFifthUpdate
				if( testVariableInt != 10 )						{ ChangeState( STATE_Broken ); return true; }	//Test substate int
				if( testVariableBool != true )					{ ChangeState( STATE_Broken ); return true; }	//Test substate bool
				if( testVariableFloat != 1.0f )					{ ChangeState( STATE_Broken ); return true; }	//Test substate float
				if( testVariableObjectID != m_owner->GetID() )	{ ChangeState( STATE_Broken ); return true; }	//Test substate objectID
				ChangeState( STATE_Success );



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Success )

		DeclareStateInt( count )

		OnEnter
			count = 0;
			OutputDebugString( L"UnitTest5 Success\n" );

		OnBothMsg( MSG_UnitTestMessage2, MSG_UnitTestMessage3 )
			count++;

		OnFirstUpdate
			SendMsgDelayedToState( 0.1f, MSG_UnitTestMessage2 );

		OnSecondUpdate
			SendMsgDelayedToState( 0.1f, MSG_UnitTestMessage3 );

		DeclareSubstate( SUBSTATE_Inside1 )

			OnBothMsg( MSG_UnitTestMessage2, MSG_UnitTestMessage3 )
				//count++;

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Broken )

		OnEnter
			OutputDebugString( L"UnitTest5 Broken\n" );


EndStateMachine
}