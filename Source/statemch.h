/* Copyright Steve Rabin, 2008. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2008"
 */

#pragma once
#pragma warning(disable: 4996)
#pragma warning(disable: 4995)

#include <vector>
#include "gameobject.h"
#include "msg.h"
#include "msgroute.h"
#include "debuglog.h"
#include "time.h"


#define MAX_STATE_NAME_SIZE (64)
#define ONE_FRAME (0.0001f)


#define DEBUG_STATE_MACHINE_MACROS		//Comment out to get the release macros (no string state/substate names and no debug logging info)
#ifdef DEBUG_STATE_MACHINE_MACROS
	#define BEGIN_STATE_MACHINE_ADDITIONAL_DEBUG_1				char eventbuffer[5];
	#define BEGIN_STATE_MACHINE_ADDITIONAL_DEBUG_2				const char statename[MAX_STATE_NAME_SIZE] = "STATE_Global"; const char substatename[MAX_STATE_NAME_SIZE] = "";
	#define END_STATE_MACHINE_ADDITIONAL_DEBUG_1				g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, itoa(event, eventbuffer, 10), false );
	#define DECLARE_STATE_ADDITIONAL_DEBUG_1					g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, itoa(event, eventbuffer, 10), false );
	#define DECLARE_STATE_ADDITIONAL_DEBUG_2(name)				int DUPLICATE_DeclareState_ ## name = 0;
	#define DECLARE_STATE_ADDITIONAL_DEBUG_3(name)				const char statename[MAX_STATE_NAME_SIZE] = #name; const char substatename[MAX_STATE_NAME_SIZE] = ""; int verifystatecontext = 0; if( EVENT_Enter == event ) { SetCurrentStateName( #name ); }
	#define DECLARE_SUBSTATE_ADDITIONAL_DEBUG_1(name)			const char statename[MAX_STATE_NAME_SIZE] = ""; const char substatename[MAX_STATE_NAME_SIZE] = #name; int verifysubstatecontext = 0; if( EVENT_Enter == event ) { SetCurrentSubstateName( #name ); } SubstateName verifysubstatename = name;
	#define ONMSG_ADDITIONAL_DEBUG_1(msgname)					VerifyMessageEnum( msgname ); g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname, true );
	#define ONEITHERMSG_ADDITIONAL_DEBUG_1(msgname1, msgname2)	VerifyMessageEnum( msgname1 ); VerifyMessageEnum( msgname2 ); if( msgname1 == msg->GetName() ) { g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname1, true ); } else { g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname2, true ); }
	#define ONBOTHMSG_ADDITIONAL_DEBUG_1(msgname1, msgname2)	if( msgname1 == msg->GetName() ) { g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname1, true ); } else { g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname2, true ); }
	#define ONANYMSG_ADDITIONAL_DEBUG_1							g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, msg->GetName(), true );
	#define ONANYUNHANDLEDMSGDEBUGBREAK_ADDITIONAL_DEBUG_1		return( true ); } } while( false ); do { if( EVENT_Message == event && msg ) { __debugbreak();
	#define ONCCMSG_ADDITIONAL_DEBUG_1(msgname)					g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname, true );
	#define ONTIMEINSTATE_ADDITIONAL_DEBUG_1					g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, "MSG_GENERIC_TIMER", true );
	#define ONEVENT_ADDITIONAL_DEBUG_1(a)						g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #a, true );
	#define ONNTHUPDATE_ADDITIONAL_DEBUG_1(n)					g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, "EVENT_Update", true ); COMPILE_TIME_ASSERT( n>0, argument_must_be_greater_than_zero );
	#define ONEVERYNTHUPDATE_ADDITIONAL_DEBUG_1(n)				g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, "EVENT_Update", true ); COMPILE_TIME_ASSERT( n>1, argument_must_be_greater_than_one );
	#define ONEVERYODDUPDATE_ADDITIONAL_DEBUG_1					g_debuglog.LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, "EVENT_Update", true );
	#define VERIFYSTATECONTEXT_ADDITIONAL_DEBUG_1				verifystatecontext;
	#define VERIFYSUBSTATECONTEXT_ADDITIONAL_DEBUG_1			verifysubstatecontext;
#else
	#define BEGIN_STATE_MACHINE_ADDITIONAL_DEBUG_1
	#define BEGIN_STATE_MACHINE_ADDITIONAL_DEBUG_2
	#define END_STATE_MACHINE_ADDITIONAL_DEBUG_1
	#define DECLARE_STATE_ADDITIONAL_DEBUG_1
	#define DECLARE_STATE_ADDITIONAL_DEBUG_2(name)
	#define DECLARE_STATE_ADDITIONAL_DEBUG_3(name)
	#define DECLARE_SUBSTATE_ADDITIONAL_DEBUG_1(name)
	#define ONMSG_ADDITIONAL_DEBUG_1(msgname)
	#define ONEITHERMSG_ADDITIONAL_DEBUG_1(msgname1, msgname2)
	#define ONBOTHMSG_ADDITIONAL_DEBUG_1(msgname1, msgname2)
	#define ONANYMSG_ADDITIONAL_DEBUG_1
	#define ONANYUNHANDLEDMSGDEBUGBREAK_ADDITIONAL_DEBUG_1
	#define ONCCMSG_ADDITIONAL_DEBUG_1(msgname)
	#define ONTIMEINSTATE_ADDITIONAL_DEBUG_1
	#define ONEVENT_ADDITIONAL_DEBUG_1(a)
	#define ONNTHUPDATE_ADDITIONAL_DEBUG_1(n)
	#define ONEVERYNTHUPDATE_ADDITIONAL_DEBUG_1(n)
	#define ONEVERYODDUPDATE_ADDITIONAL_DEBUG_1
	#define VERIFYSTATECONTEXT_ADDITIONAL_DEBUG_1
	#define VERIFYSUBSTATECONTEXT_ADDITIONAL_DEBUG_1
#endif


//State Machine Language Macros (put the keywords in the file USERTYPE.DAT in the same directory as MSDEV.EXE to get keyword highlighting)
#define BeginStateMachine						StateName laststatedeclared; BEGIN_STATE_MACHINE_ADDITIONAL_DEBUG_1 if( state < 0 ) { BEGIN_STATE_MACHINE_ADDITIONAL_DEBUG_2 if( EVENT_Message == event && msg && MSG_CHANGE_STATE_DELAYED == msg->GetName() ) { ChangeState( static_cast<unsigned int>( msg->GetIntData() ) ); return( true ); } if( EVENT_Message == event && msg && MSG_CHANGE_SUBSTATE_DELAYED == msg->GetName() ) { ChangeSubstate( static_cast<unsigned int>( msg->GetIntData() ) ); return( true ); } do { if(0) {
#define EndStateMachine							return( true ); } } while( false ); END_STATE_MACHINE_ADDITIONAL_DEBUG_1 return( false ); } ASSERTMSG( 0, "Invalid State" ); return( false );

#define DeclareState(name)						return( true ); } } while( false ); DECLARE_STATE_ADDITIONAL_DEBUG_1 return( false ); } laststatedeclared = name; DECLARE_STATE_ADDITIONAL_DEBUG_2( name ) if( name == state && substate < 0 ) { int statevariableindexinternal = 0; int substatevariableindexinternal = 0; DECLARE_STATE_ADDITIONAL_DEBUG_3( name ) do { if(0) { 
#define DeclareSubstate(name)					return( true ); } } while( false ); return( false ); } if( laststatedeclared == state && name == substate ) { int statevariableindexinternal = 0; int substatevariableindexinternal = 0; DECLARE_SUBSTATE_ADDITIONAL_DEBUG_1(name) do { if(0) { 

#define OnMsg(msgname)							return( true ); } } while( false ); do { if( EVENT_Message == event && msg && msgname == msg->GetName() ) { ONMSG_ADDITIONAL_DEBUG_1( msgname )
#define OnEitherMsg(msgname1, msgname2)			return( true ); } } while( false ); do { if( EVENT_Message == event && msg && (msgname1 == msg->GetName() || msgname2 == msg->GetName()) ) { ONEITHERMSG_ADDITIONAL_DEBUG_1( msgname1, msgname2 )
#define OnBothMsg(msgname1, msgname2)			return( true ); } } while( false ); int variableindexinternal__ ## msgname1 ## msgname2; StateVariableScope onbothmsgvariablescope__ ## msgname1 ## msgname2; if( substate < 0 ) { variableindexinternal__ ## msgname1 ## msgname2 = statevariableindexinternal++; onbothmsgvariablescope__ ## msgname1 ## msgname2 = STATE_VARIABLE_SCOPE; } else { variableindexinternal__ ## msgname1 ## msgname2 = substatevariableindexinternal++; onbothmsgvariablescope__ ## msgname1 ## msgname2 = SUBSTATE_VARIABLE_SCOPE; } StateVariableInt msgname1 ## msgname2( variableindexinternal__ ## msgname1 ## msgname2, this, onbothmsgvariablescope__ ## msgname1 ## msgname2, EVENT_Probe == event ); do { if( EVENT_Message == event && msg ) { if( msgname1 == msg->GetName() ) { msgname1 ## msgname2 |= 0x01; } if( msgname2 == msg->GetName() ) { msgname1 ## msgname2 |= 0x10; } if( msgname1 ## msgname2 != 0x11 ) { continue; } msgname1 ## msgname2 = 0; VerifyMessageEnum( msgname1 ); VerifyMessageEnum( msgname2 ); ONBOTHMSG_ADDITIONAL_DEBUG_1( msgname1, msgname2 )
#define OnAnyMsg								return( true ); } } while( false ); do { if( EVENT_Message == event && msg ) { ONANYMSG_ADDITIONAL_DEBUG_1
#define OnAnyUnhandledMsgDebugBreak				ONANYUNHANDLEDMSGDEBUGBREAK_ADDITIONAL_DEBUG_1
#define OnCCMsg(msgname)						return( true ); } } while( false ); do { if( EVENT_CCMessage == event && msg && msgname == msg->GetName() ) { ONCCMSG_ADDITIONAL_DEBUG_1( msgname )

#define ONTIME_INTERNAL_HELPER(f, s)			return( true ); } } while( false ); do { if( EVENT_Probe == event ) { f( s, MSG_GENERIC_TIMER, MSG_Data( __LINE__ ) ); continue; } if( EVENT_Message == event && msg && MSG_GENERIC_TIMER == msg->GetName() && msg->GetIntData() == __LINE__ ) { ONTIMEINSTATE_ADDITIONAL_DEBUG_1
#define OnTimeInSubstate(s)						ONTIME_INTERNAL_HELPER( SendMsgDelayedToSubstate, s )
#define OnTimeInState(s)						ONTIME_INTERNAL_HELPER( SendMsgDelayedToState, s )

#define ONPERIODIC_INTERNAL_HELPER(f, s)		return( true ); } } while( false ); do { if( EVENT_Probe == event ) { f( s, MSG_GENERIC_TIMER, MSG_Data( __LINE__ ) ); continue; } if( EVENT_Message == event && msg && MSG_GENERIC_TIMER == msg->GetName() && msg->GetIntData() == __LINE__ ) { f( s, MSG_GENERIC_TIMER, MSG_Data( __LINE__ ) ); ONTIMEINSTATE_ADDITIONAL_DEBUG_1
#define OnPeriodicTimeInSubstate(s)				ONPERIODIC_INTERNAL_HELPER( SendMsgDelayedToSubstate, s )
#define OnPeriodicTimeInState(s)				ONPERIODIC_INTERNAL_HELPER( SendMsgDelayedToState, s )

#define ONEVENT_INTERNAL_HELPER(a, f)			return( true ); } } while( false ); do { if( EVENT_Probe == event ) { f( state, substate ); continue; } if( a == event ) { ONEVENT_ADDITIONAL_DEBUG_1( a )
#define OnUpdate								ONEVENT_INTERNAL_HELPER( EVENT_Update, RegisterOnUpdate )
#define OnEnter									ONEVENT_INTERNAL_HELPER( EVENT_Enter, RegisterOnEnter )
#define OnExit									ONEVENT_INTERNAL_HELPER( EVENT_Exit, RegisterOnExit )

#define OnNthUpdate(n)							return( true ); } } while( false ); do { if( EVENT_Probe == event ) { RegisterOnUpdate( state, substate ); continue; } if( EVENT_Update == event && IsUpdateIteration( n ) ) { ONNTHUPDATE_ADDITIONAL_DEBUG_1( n )
#define OnFirstUpdate							OnNthUpdate( 1 )
#define OnSecondUpdate							OnNthUpdate( 2 )
#define OnThirdUpdate							OnNthUpdate( 3 )
#define OnFourthUpdate							OnNthUpdate( 4 )
#define OnFifthUpdate							OnNthUpdate( 5 )

#define OnEveryNthUpdate(n)						return( true ); } } while( false ); do { if( EVENT_Probe == event ) { RegisterOnUpdate( state, substate ); continue; } if( EVENT_Update == event && IsUpdateMultiple( n ) ) { ONEVERYNTHUPDATE_ADDITIONAL_DEBUG_1( n )
#define OnEveryOddUpdate						return( true ); } } while( false ); do { if( EVENT_Probe == event ) { RegisterOnUpdate( state, substate ); continue; } if( EVENT_Update == event && IsUpdateOdd() ) { ONEVERYODDUPDATE_ADDITIONAL_DEBUG_1
#define OnEveryEvenUpdate						OnEveryNthUpdate( 2 )

#define EscapeWithoutConsumingUpdate			ASSERTMSG( EVENT_Update == event, "EscapeWithoutConsumingUpdate - event is not an update"); continue;
#define EscapeWithoutConsumingMsg				ASSERTMSG( EVENT_Message == event, "EscapeWithoutConsumingMsg - event is not a message"); continue;

#define DECLARESTATEVAR_INTERNAL_HELPER(t,n)	return( true ); } } while( false ); VERIFYSTATECONTEXT_ADDITIONAL_DEBUG_1 t n( statevariableindexinternal++, this, STATE_VARIABLE_SCOPE, EVENT_Probe == event ); do { if(0) {
#define DeclareStateInt(name)					DECLARESTATEVAR_INTERNAL_HELPER( StateVariableInt, name )
#define DeclareStateFloat(name)					DECLARESTATEVAR_INTERNAL_HELPER( StateVariableFloat, name )
#define DeclareStateBool(name)					DECLARESTATEVAR_INTERNAL_HELPER( StateVariableBool, name )
#define DeclareStateObjectID(name)				DECLARESTATEVAR_INTERNAL_HELPER( StateVariableObjectID, name )
#define DeclareStatePointerVoid(name)			DECLARESTATEVAR_INTERNAL_HELPER( StateVariablePointerVoid, name )
#define DeclareStatePointerVector2(name)		DECLARESTATEVAR_INTERNAL_HELPER( StateVariablePointerVector2, name )
#define DeclareStatePointerVector3(name)		DECLARESTATEVAR_INTERNAL_HELPER( StateVariablePointerVector3, name )

#define DECLARESUBSTATEVAR_INTERNAL_HELPER(t,n)	return( true ); } } while( false ); VERIFYSUBSTATECONTEXT_ADDITIONAL_DEBUG_1 t n( substatevariableindexinternal++, this, SUBSTATE_VARIABLE_SCOPE, EVENT_Probe == event ); do { if(0) {
#define DeclareSubstateInt(name)				DECLARESUBSTATEVAR_INTERNAL_HELPER( StateVariableInt, name )
#define DeclareSubstateFloat(name)				DECLARESUBSTATEVAR_INTERNAL_HELPER( StateVariableFloat, name )
#define DeclareSubstateBool(name)				DECLARESUBSTATEVAR_INTERNAL_HELPER( StateVariableBool, name )
#define DeclareSubstateObjectID(name)			DECLARESUBSTATEVAR_INTERNAL_HELPER( StateVariableObjectID, name )
#define DeclareSubstatePointerVoid(name)		DECLARESUBSTATEVAR_INTERNAL_HELPER( StateVariablePointerVoid, name )
#define DeclareSubstatePointerVector2(name)		DECLARESUBSTATEVAR_INTERNAL_HELPER( StateVariablePointerVector2, name )
#define DeclareSubstatePointerVector3(name)		DECLARESUBSTATEVAR_INTERNAL_HELPER( StateVariablePointerVector3, name )


//Helpers to set breakpoints for particular game objects
#define WATCHPOINT_ID(id)					if( m_owner->GetID() == id ) { __debugbreak(); }
#define WATCHPOINT_NAME(name)				if( strcmp(name, m_owner->GetName() ) == 0 ) { __debugbreak(); }

enum State_Machine_Event {
	EVENT_INVALID,
	EVENT_Update,
	EVENT_Message,
	EVENT_CCMessage,
	EVENT_Enter,
	EVENT_Exit,
	EVENT_Probe
};

#define REGISTERED_EVENT_NULL					(0)
#define REGISTERED_EVENT_ENTER_SUBSTATE			(1<<1)
#define REGISTERED_EVENT_ENTER_STATE			(1<<2)
#define REGISTERED_EVENT_ENTER_STATEMACHINE		(1<<3)
#define REGISTERED_EVENT_EXIT_SUBSTATE			(1<<4)
#define REGISTERED_EVENT_EXIT_STATE				(1<<5)
#define REGISTERED_EVENT_EXIT_STATEMACHINE		(1<<6)
#define REGISTERED_EVENT_UPDATE_SUBSTATE		(1<<7)
#define REGISTERED_EVENT_UPDATE_STATE			(1<<8)
#define REGISTERED_EVENT_UPDATE_STATEMACHINE	(1<<9)
#define REGISTERED_EVENT_MESSAGE_SUBSTATE		(1<<10)
#define REGISTERED_EVENT_MESSAGE_STATE			(1<<11)
#define REGISTERED_EVENT_MESSAGE_STATEMACHINE	(1<<12)
#define REGISTERED_EVENT_SUBSTATE				(REGISTERED_EVENT_ENTER_SUBSTATE | REGISTERED_EVENT_EXIT_SUBSTATE | REGISTERED_EVENT_UPDATE_SUBSTATE | REGISTERED_EVENT_MESSAGE_SUBSTATE)
#define REGISTERED_EVENT_STATE					(REGISTERED_EVENT_ENTER_STATE | REGISTERED_EVENT_EXIT_STATE | REGISTERED_EVENT_UPDATE_STATE | REGISTERED_EVENT_MESSAGE_STATE)
#define REGISTERED_EVENT_STATEMACHINE			(REGISTERED_EVENT_ENTER_STATEMACHINE | REGISTERED_EVENT_EXIT_STATEMACHINE | REGISTERED_EVENT_UPDATE_STATEMACHINE | REGISTERED_EVENT_MESSAGE_STATEMACHINE)
#define REGISTERED_EVENT_UPDATE					(REGISTERED_EVENT_UPDATE_SUBSTATE | REGISTERED_EVENT_UPDATE_STATE | REGISTERED_EVENT_UPDATE_STATEMACHINE)

enum StateMachineQueue {
	STATE_MACHINE_QUEUE_0,
	STATE_MACHINE_QUEUE_1,
	STATE_MACHINE_QUEUE_2,
	STATE_MACHINE_QUEUE_3,
	STATE_MACHINE_NUM_QUEUES,
	STATE_MACHINE_QUEUE_ALL,
	STATE_MACHINE_QUEUE_NULL
};

enum StateMachineChange {
	NO_STATE_MACHINE_CHANGE,
	STATE_MACHINE_RESET,
	STATE_MACHINE_REPLACE,
	STATE_MACHINE_QUEUE,
	STATE_MACHINE_REQUEUE,
	STATE_MACHINE_PUSH,
	STATE_MACHINE_POP
};

enum StateVariableScope {
	STATE_VARIABLE_SCOPE,
	SUBSTATE_VARIABLE_SCOPE
};

union StateMachine_Data_Union
{
	int intValue;
	float floatValue;
	bool boolValue;
	objectID objectIDValue;
	void* pointerValue;
	D3DXVECTOR2* vector2Value;
	D3DXVECTOR3* vector3Value;
};

class StateMachinePersistentData
{
public:
	StateMachinePersistentData( void )				{ m_data.intValue = 0; }
	~StateMachinePersistentData( void )				{}

	inline void SetInt( int value )					{ m_data.intValue = value; }
	inline void SetFloat( float value )				{ m_data.floatValue = value; }
	inline void SetBool( bool value )				{ m_data.boolValue = value; }
	inline void SetObjectID( objectID value )		{ m_data.objectIDValue = value; }
	inline void SetPointer( void* value )			{ m_data.pointerValue = value; }
	inline void SetVector2( D3DXVECTOR2* value )	{ m_data.vector2Value = value; }
	inline void SetVector3( D3DXVECTOR3* value )	{ m_data.vector3Value = value; }

	inline int GetInt( void )						{ return m_data.intValue; }
	inline float GetFloat( void )					{ return m_data.floatValue; }
	inline bool GetBool( void )						{ return m_data.boolValue; }
	inline objectID GetObjectID( void )				{ return m_data.objectIDValue; }
	inline void* GetPointer( void )					{ return m_data.pointerValue; }
	inline D3DXVECTOR2* GetVector2( void )			{ return m_data.vector2Value; }
	inline D3DXVECTOR3* GetVector3( void )			{ return m_data.vector3Value; }

private:
	StateMachine_Data_Union m_data;
	
};


//Forward declarations
class StateMachineManager;


class StateMachine
{
public:

	StateMachine( GameObject & object );
	virtual ~StateMachine( void );

	void SetStateMachineQueue( StateMachineQueue queue )	{ ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachine::SetQueue - invalid queue" ); m_queue = queue; }

	//Should only be called by GameObject
	void Update( void );
	void Reset( void );

	//Only to be used by msgroute!
	void SetTimerExternal( float delay, MSG_Name name, Scope_Rule rule );

	//Access state and scope
	inline int GetState( void )							{ return( (int)m_currentState ); }
	inline int GetSubstate( void )						{ return( m_currentSubstate ); }
	inline unsigned int GetScopeState( void )			{ return( m_scopeState ); }
	inline unsigned int GetScopeSubstate( void )		{ return( m_scopeSubstate ); }
	
	//Main state machine code stored in here
	void Process( State_Machine_Event event, MSG_Object * msg );

	//Debug info
	inline char * GetCurrentStateNameString( void )		{ return( m_currentStateNameString ); }
	inline char * GetCurrentSubstateNameString( void )	{ return( m_currentSubstateNameString ); }

	//Used for state variables (internal only - don't call directly from state machine)
	void SetStateVariableInt( int value, int id, StateVariableScope scope );
	void SetStateVariableFloat( float value, int id, StateVariableScope scope );
	void SetStateVariableBool( bool value, int id, StateVariableScope scope );
	void SetStateVariableObjectID( objectID value, int id, StateVariableScope scope );
	void SetStateVariablePointer( void* value, int id, StateVariableScope scope );
	void SetStateVariableVector2( D3DXVECTOR2* value, int id, StateVariableScope scope );
	void SetStateVariableVector3( D3DXVECTOR3* value, int id, StateVariableScope scope );

	//Used for state variables (internal only - don't call directly from state machine)
	int GetStateVariableInt( int id, StateVariableScope scope );
	float GetStateVariableFloat( int id, StateVariableScope scope );
	bool GetStateVariableBool( int id, StateVariableScope scope );
	objectID GetStateVariableObjectID( int id, StateVariableScope scope );
	void* GetStateVariablePointer( int id, StateVariableScope scope );
	D3DXVECTOR2* GetStateVariableVector2( int id, StateVariableScope scope );
	D3DXVECTOR3* GetStateVariableVector3( int id, StateVariableScope scope );
	void DeclareVariable( int id, StateVariableScope scope );


protected:

	GameObject * m_owner;				//GameObject that owns this state machine
	StateMachineManager * m_mgr;		//StateMachineManager that owns this state machine
	StateMachineQueue m_queue;			//The queue this state machine is on

	/////////////////////////////////////
	//Send messages
	/////////////////////////////////////
	//Send message next frame to another agent (with optional data)
	void SendMsg( MSG_Name name, objectID receiver, MSG_Data& data = MSG_Data() );
	//Send message immediately to another agent (with optional data)
	void SendMsgNow( MSG_Name name, objectID receiver, MSG_Data& data = MSG_Data() );
	//Send message next frame to self only in the current state, but any substate (with optional data)
	void SendMsgToState( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message next frame to self only in the current substate (with optional data)
	void SendMsgToSubstate( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message next frame to self only in the current state machine, but any state or substate (with optional data)
	void SendMsgToStateMachine( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message immediately to self (with optional data)
	void SendMsgToStateMachineNow( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message next frame to the this queue of the agent (with optional data)
	void SendMsgToSingleQueue( MSG_Name name, StateMachineQueue queue, MSG_Data& data = MSG_Data() );
	//Send message immediately to this queue of the agent (with optional data)
	void SendMsgToSingleQueueNow( MSG_Name name, StateMachineQueue queue, MSG_Data& data = MSG_Data() );
	//Send message next frame to all queues of this agent (with optional data)
	void SendMsgToAllQueues( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message immediately to all queues of this agent (with optional data)
	void SendMsgToAllQueuesNow( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message next frame to all other queues of this agent (with optional data)
	void SendMsgToAllOtherQueues( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message immediately to all other queues of this agent (with optional data)
	void SendMsgToAllOtherQueuesNow( MSG_Name name, MSG_Data& data = MSG_Data() );

	/////////////////////////////////////
	//Send delayed messages
	/////////////////////////////////////
	//Send message delayed to another agent
	void SendMsgDelayed( float delay, MSG_Name name, objectID receiver, MSG_Data& data = MSG_Data() );
	//Send message delayed to self only in the current substate (with optional data)
	void SendMsgDelayedToSubstate( float delay, MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message delayed to self only in the current state, but any substate (with optional data)
	void SendMsgDelayedToState( float delay, MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message delayed to self only in the current state machine, but any state or substate (with optional data)
	void SendMsgDelayedToStateMachine( float delay, MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message delayed to this queue of the agent (with optional data)
	void SendMsgDelayedToSingleQueue( float delay, MSG_Name name, StateMachineQueue queue, MSG_Data& data = MSG_Data() );
	//Send message delayed to all queues of this agent (with optional data)
	void SendMsgDelayedToAllQueues( float delay, MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send message delayed to all other queues of this agent (with optional data)
	void SendMsgDelayedToAllOtherQueues( float delay, MSG_Name name, MSG_Data& data = MSG_Data() );

	/////////////////////////////////////
	//Broadcast messages
	/////////////////////////////////////
	//Send broadcast message immediately to every agent of a particular type (with optional data)
	//Note: Use OBJECT_Ignore_Type if type doesn't matter
	void SendMsgBroadcastNow( MSG_Name name, unsigned int type, MSG_Data& data = MSG_Data() );
	//Send broadcast message next frame to every agent on the list (with optional data)
	void SendMsgBroadcastToList( MSG_Name name, MSG_Data& data = MSG_Data() );
	//Send broadcast message immediately to every agent on the list (with optional data)
	void SendMsgBroadcastToListNow( MSG_Name name, MSG_Data& data = MSG_Data() );


	//Broadcast List
	void BroadcastClearList( void );
	void BroadcastAddToList( objectID id );

	//CCing other objects
	inline void SetCCReceiver( objectID id )			{ m_ccMessagesToGameObject = id; }
	inline void ClearCCReceiver( void )					{ m_ccMessagesToGameObject = 0; }
	inline objectID GetCCReceiver( void )				{ return( m_ccMessagesToGameObject ); }
	
	//Message timers - prefer over OnUpdate since it is load balanced (all delayed messages are load balanced)
	void SetTimerSubstate( float delay, MSG_Name name );		//Timer will be destroyed if current substate is exited
	void SetTimerState( float delay, MSG_Name name );			//Timer will be destroyed if current state is exited
	void SetTimerStateMachine( float delay, MSG_Name name );	//Timer will be destroyed if current state machine is exited
	void StopTimer( MSG_Name name );
	
	//Change State
	void PopState( void );
	void ChangeState( unsigned int newState );
	void ChangeStateDelayed( float delay, unsigned int newState );
	void ChangeSubstate( unsigned int newSubstate );
	void ChangeSubstateDelayed( float delay, unsigned int newSubstate );
	
	//Switch to another State Machine
	int GetNumStateMachinesInQueue( void );
	void ResetStateMachine( void );
	void ReplaceStateMachine( StateMachine & mch );
	void QueueStateMachine( StateMachine & mch );
	void RequeueStateMachine( void );
	void PushStateMachine( StateMachine & mch );
	void PopStateMachine( void );
	void DeleteStateMachineQueue( StateMachineQueue queue );

	//Random generation for times
	float RandDelay( float min, float max );

	//Destroy game object
	void MarkForDeletion( void )						{ m_owner->MarkForDeletion(); }

	//Helper functions
	inline float GetTimeInState( void )					{ return( g_time.GetCurTime() - m_timeOnEnterState ); }
	inline float GetTimeInSubstate( void )				{ return( g_time.GetCurTime() - m_timeOnEnterSubstate ); }
	inline bool IsChangeStateDelayedQueued( void )		{ return( m_delayedStateChangeQueued ); }
	inline bool IsChangeSubstateDelayedQueued( void )	{ return( m_delayedSubstateChangeQueued ); }
	inline bool IsUpdateIteration( int i )				{ ASSERTMSG( i > 0, "StateMachine::OnNthUpdate - Argument must be > 0."); return( i == m_updateIteration ); }
	inline bool IsUpdateMultiple( int i )				{ ASSERTMSG( i > 0, "StateMachine::OnEveryNthUpdate - Argument must be > 0."); return( 0 == m_updateIteration%i ); }
	inline bool IsUpdateOdd( void )						{ return( 0 == (m_updateIteration+1)%2 ); }

	//Register events
	inline void RegisterOnEnter( int state, int substate )		{ if( state >= 0 ) { if( substate < 0 ) { RegisterOnEnterState(); } else { RegisterOnEnterSubstate(); } } else { RegisterOnEnterStateMachine(); } }
	inline void RegisterOnEnterSubstate( void )					{ m_registeredEvents |= REGISTERED_EVENT_ENTER_SUBSTATE; }
	inline void RegisterOnEnterState( void )					{ m_registeredEvents |= REGISTERED_EVENT_ENTER_STATE; }
	inline void RegisterOnEnterStateMachine( void )				{ m_registeredEvents |= REGISTERED_EVENT_ENTER_STATEMACHINE; }
	inline void RegisterOnExit( int state, int substate )		{ if( state >= 0 ) { if( substate < 0 ) { RegisterOnExitState(); } else { RegisterOnExitSubstate(); } } else { RegisterOnExitStateMachine(); } }
	inline void RegisterOnExitSubstate( void )					{ m_registeredEvents |= REGISTERED_EVENT_EXIT_SUBSTATE; }
	inline void RegisterOnExitState( void )						{ m_registeredEvents |= REGISTERED_EVENT_EXIT_STATE; }
	inline void RegisterOnExitStateMachine( void )				{ m_registeredEvents |= REGISTERED_EVENT_EXIT_STATEMACHINE; }
	inline void RegisterOnUpdate( int state, int substate )		{ if( state >= 0 ) { if( substate < 0 ) { RegisterOnUpdateState(); } else { RegisterOnUpdateSubstate(); } } else { RegisterOnUpdateStateMachine(); } }
	inline void RegisterOnUpdateSubstate( void )				{ m_registeredEvents |= REGISTERED_EVENT_UPDATE_SUBSTATE; }
	inline void RegisterOnUpdateState( void )					{ m_registeredEvents |= REGISTERED_EVENT_UPDATE_STATE; }
	inline void RegisterOnUpdateStateMachine( void )			{ m_registeredEvents |= REGISTERED_EVENT_UPDATE_STATEMACHINE; }
	inline void RegisterOnMsg( int state, int substate )		{ if( state >= 0 ) { if( substate < 0 ) { RegisterOnMsgState(); } else { RegisterOnMsgSubstate(); } } else { RegisterOnMsgStateMachine(); } }
	inline void RegisterOnMsgSubstate( void )					{ m_registeredEvents |= REGISTERED_EVENT_MESSAGE_SUBSTATE; }
	inline void RegisterOnMsgState( void )						{ m_registeredEvents |= REGISTERED_EVENT_MESSAGE_STATE; }
	inline void RegisterOnMsgStateMachine( void )				{ m_registeredEvents |= REGISTERED_EVENT_MESSAGE_STATEMACHINE; }

	//Used to verify proper message enums
	inline void VerifyMessageEnum( MSG_Name name ) {}

	//Used for debug to capture current state/substate name string
	inline void SetCurrentStateName( char * state )				{ strcpy( m_currentStateNameString, state ); m_currentSubstateNameString[0] = 0; }
	inline void SetCurrentSubstateName( char * substate )		{ strcpy( m_currentSubstateNameString, substate ); }


private:

	typedef std::vector<objectID> BroadcastListContainer;	//Container to hold game objects to broadcast to
	typedef std::list<unsigned int> StateListContainer;		//Container to hold past states

	enum State_Change {							//Possible state change requests
		NO_STATE_CHANGE,						//No change pending
		STATE_CHANGE,							//State change pending
		STATE_POP								//State pop pending
	};

	unsigned int m_scopeState;					//The current scope of the state
	unsigned int m_scopeSubstate;				//The current scope of the substate
	unsigned int m_currentState;				//Current state
	unsigned int m_nextState;					//Next state to switch to
	int m_updateIteration;						//The update iteration within this substate
	int m_currentSubstate;						//Current substate (-1 indicates not valid)
	int m_nextSubstate;							//Next substate (-1 indicates not valid)
	bool m_stateChangeAllowed;					//If a state change is allowed
	bool m_delayedStateChangeQueued;			//If a delayed state change was queued
	bool m_delayedSubstateChangeQueued;			//If a delayed state change was queued
	State_Change m_stateChange;					//If a state change is pending
	float m_timeOnEnterState;					//Time since state was entered
	float m_timeOnEnterSubstate;				//Time since substate was entered
	unsigned int m_registeredEvents;			//Whether particular events are registered
	objectID m_ccMessagesToGameObject;			//A GameObject to CC messages to
	BroadcastListContainer m_broadcastList;		//List of GameObjects to broadcast to
	StateListContainer m_stack;					//Stack of past states (used for PopState)

	std::vector<StateMachinePersistentData*> m_stateVariables;		//Container of state variables
	std::vector<StateMachinePersistentData*> m_substateVariables;	//Container of substate variables

	//Debug info
	char m_currentStateNameString[MAX_STATE_NAME_SIZE];		//Current state name string
	char m_currentSubstateNameString[MAX_STATE_NAME_SIZE];	//Current substate name string

	void Initialize( void );
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate ) = 0;
	void PerformStateChanges( void );
	void SendCCMsg( MSG_Name name, objectID receiver, MSG_Data& data );
	void SendMsgDelayedToMeHelper( float delay, MSG_Name name, Scope_Rule scope, StateMachineQueue queue, MSG_Data& data, bool timer );

	//Used for state variables (internal only - don't call directly from state machine)
	void DeleteAllStateVariables( void );
	void DeleteAllSubstateVariables( void );

};


class StateMachineManager
{
public:
	StateMachineManager( GameObject & object );
	~StateMachineManager( void );

	void Update( void );
	void SendMsg( MSG_Object msg );
	void Process( State_Machine_Event event, MSG_Object * msg, StateMachineQueue queue );

	inline StateMachine* GetStateMachine( StateMachineQueue queue )	{ if( m_stateMachineList[queue].empty() ) { return( 0 ); } else { return( m_stateMachineList[queue].back() ); } }
	inline int GetNumStateMachinesInQueue( StateMachineQueue queue )	{ return( (int)m_stateMachineList[queue].size() ); }
	void RequestStateMachineChange( StateMachine * mch, StateMachineChange change, StateMachineQueue queue );
	void ResetStateMachine( StateMachineQueue queue );
	void ReplaceStateMachine( StateMachine & mch, StateMachineQueue queue );
	void QueueStateMachine( StateMachine & mch, StateMachineQueue queue );
	void RequeueStateMachine( StateMachineQueue queue );
	void PushStateMachine( StateMachine & mch, StateMachineQueue queue, bool initialize );
	void PopStateMachine( StateMachineQueue queue );
	void DeleteStateMachineQueue( StateMachineQueue queue );

private:

	GameObject * m_owner;													//GameObject that owns this state machine

	typedef std::list<StateMachine*> stateMachineListContainer;				//Queue of state machines. Top one is active.
	stateMachineListContainer m_stateMachineList[STATE_MACHINE_NUM_QUEUES];	//Array of state machine queues
	StateMachineChange m_stateMachineChange[STATE_MACHINE_NUM_QUEUES];		//Directions for any pending state machine changes
	StateMachine * m_newStateMachine[STATE_MACHINE_NUM_QUEUES];				//A state machine that will be added to the queue later
	void ProcessStateMachineChangeRequests( StateMachineQueue queue );

};


class StateVariableInt
{
public:
	StateVariableInt( int id, StateMachine* sm, StateVariableScope scope, bool init )	{ m_writeback = false; if( !init ) { m_id = id; m_stateMachine = sm; m_scope = scope; m_int = m_stateMachine->GetStateVariableInt( m_id, m_scope ); } else { sm->DeclareVariable( id, scope ); } }
	~StateVariableInt( void )															{ if( m_writeback ) { m_stateMachine->SetStateVariableInt(m_int, m_id, m_scope); } }
	
	inline operator int()			{ return m_int; }
	inline int operator= (int a)	{ m_writeback = true; return( m_int = a ); }
	inline int operator+ (int a)	{ return( m_int + a ); }
	inline int operator- (int a)	{ return( m_int - a ); }
	inline int operator* (int a)	{ return( m_int * a ); }
	inline int operator/ (int a)	{ return( m_int / a ); }
	inline int operator+= (int a)	{ m_writeback = true; return( m_int += a ); }
	inline int operator-= (int a)	{ m_writeback = true; return( m_int -= a ); }
	inline int operator*= (int a)	{ m_writeback = true; return( m_int *= a ); }
	inline int operator/= (int a)	{ m_writeback = true; return( m_int /= a ); }
	inline int operator++ (int)		{ m_writeback = true; return( m_int++ ); }
	inline int operator++ ()		{ m_writeback = true; return( ++m_int ); }
	inline int operator-- (int)		{ m_writeback = true; return( m_int-- ); }
	inline int operator-- ()		{ m_writeback = true; return( --m_int ); }
	inline bool operator< (int a)	{ return( m_int < a ); }
	inline bool operator<= (int a)	{ return( m_int <= a ); }
	inline bool operator> (int a)	{ return( m_int > a ); }
	inline bool operator>= (int a)	{ return( m_int >= a ); }
	inline bool operator!= (int a)	{ return( m_int != a ); }
	inline bool operator== (int a)	{ return( m_int == a ); }
	inline int operator<< (int a)	{ return( m_int << a ); }
	inline int operator>> (int a)	{ return( m_int >> a ); }
	inline int operator<<= (int a)	{ m_writeback = true; return( m_int <<= a ); }
	inline int operator>>= (int a)	{ m_writeback = true; return( m_int >>= a ); }
	inline int operator% (int a)	{ return( m_int % a ); }
	inline int operator| (int a)	{ return( m_int | a ); }
	inline int operator& (int a)	{ return( m_int & a ); }
	inline int operator^ (int a)	{ return( m_int ^ a ); }
	inline int operator%= (int a)	{ m_writeback = true; return( m_int %= a ); }
	inline int operator|= (int a)	{ m_writeback = true; return( m_int |= a ); }
	inline int operator&= (int a)	{ m_writeback = true; return( m_int &= a ); }
	inline int operator^= (int a)	{ m_writeback = true; return( m_int ^= a ); }
	inline int operator~ ()			{ return ~m_int; }

private:
	int m_int;
	int m_id;
	StateVariableScope m_scope;
	StateMachine* m_stateMachine;
	bool m_writeback;
};

class StateVariableFloat
{
public:
	StateVariableFloat( int id, StateMachine* sm, StateVariableScope scope, bool init )	{ m_writeback = false; if( !init ) { m_id = id; m_stateMachine = sm; m_scope = scope; m_float = m_stateMachine->GetStateVariableFloat( m_id, m_scope ); } else { sm->DeclareVariable( id, scope ); } }
	~StateVariableFloat( void )															{ if( m_writeback ) { m_stateMachine->SetStateVariableFloat(m_float, m_id, m_scope); } }

	inline operator float()				{ return m_float; }
	inline float operator= (float a)	{ m_writeback = true; return( m_float = a ); }
	inline float operator= (int a)		{ m_writeback = true; return( m_float = (float)a ); }
	inline float operator+ (float a)	{ return( m_float + a ); }
	inline float operator+ (int a)		{ return( m_float + (float)a ); }
	inline float operator- (float a)	{ return( m_float - a ); }
	inline float operator- (int a)		{ return( m_float - (float)a ); }
	inline float operator* (float a)	{ return( m_float * a ); }
	inline float operator* (int a)		{ return( m_float * (float)a ); }
	inline float operator/ (float a)	{ return( m_float / a ); }
	inline float operator/ (int a)		{ return( m_float / (float)a ); }
	inline float operator+= (float a)	{ m_writeback = true; return( m_float += a ); }
	inline float operator+= (int a)		{ m_writeback = true; return( m_float += (float)a ); }
	inline float operator-= (float a)	{ m_writeback = true; return( m_float -= a ); }
	inline float operator-= (int a)		{ m_writeback = true; return( m_float -= (float)a ); }
	inline float operator*= (float a)	{ m_writeback = true; return( m_float *= a ); }
	inline float operator*= (int a)		{ m_writeback = true; return( m_float *= a ); }
	inline float operator/= (float a)	{ m_writeback = true; return( m_float /= (float)a ); }
	inline float operator/= (int a)		{ m_writeback = true; return( m_float /= (float)a ); }
	inline bool operator< (float a)		{ return( m_float < a ); }
	inline bool operator< (int a)		{ return( m_float < (float)a ); }
	inline bool operator<= (float a)	{ return( m_float <= a ); }
	inline bool operator<= (int a)		{ return( m_float <= (float)a ); }
	inline bool operator> (float a)		{ return( m_float > a ); }
	inline bool operator> (int a)		{ return( m_float > (float)a ); }
	inline bool operator>= (float a)	{ return( m_float >= a ); }
	inline bool operator>= (int a)		{ return( m_float >= (float)a ); }
	inline bool operator!= (float a)	{ return( m_float != a ); }
	inline bool operator!= (int a)		{ return( m_float != (float)a ); }
	inline bool operator== (float a)	{ return( m_float == a ); }
	inline bool operator== (int a)		{ return( m_float == (float)a ); }

private:
	float m_float;
	int m_id;
	StateVariableScope m_scope;
	StateMachine* m_stateMachine;
	bool m_writeback;
};

class StateVariableBool
{
public:
	StateVariableBool( int id, StateMachine* sm, StateVariableScope scope, bool init )	{ m_writeback = false; if( !init ) { m_id = id; m_stateMachine = sm; m_scope = scope; m_bool = m_stateMachine->GetStateVariableBool( m_id, m_scope ); } else { sm->DeclareVariable( id, scope ); } }
	~StateVariableBool( void )															{ if( m_writeback ) { m_stateMachine->SetStateVariableBool(m_bool, m_id, m_scope); } }

	inline operator bool()			{ return m_bool; }
	inline bool operator= (bool a)	{ m_writeback = true; return( m_bool = a ); }
	inline bool operator!= (bool a)	{ return( m_bool != a ); }
	inline bool operator== (bool a)	{ return( m_bool == a ); }
	inline bool operator! ()		{ return !m_bool; }

private:
	bool m_bool;
	int m_id;
	StateVariableScope m_scope;
	StateMachine* m_stateMachine;
	bool m_writeback;
};

class StateVariableObjectID
{
public:
	StateVariableObjectID( int id, StateMachine* sm, StateVariableScope scope, bool init )	{ m_writeback = false; if( !init ) { m_id = id; m_stateMachine = sm; m_scope = scope; m_objectID = m_stateMachine->GetStateVariableObjectID( m_id, m_scope ); } else { sm->DeclareVariable( id, scope ); } }
	~StateVariableObjectID( void )															{ if( m_writeback ) { m_stateMachine->SetStateVariableObjectID(m_objectID, m_id, m_scope); } }

	inline operator objectID()				{ return m_objectID; }
	inline objectID operator= (objectID a)	{ m_writeback = true; return( m_objectID = a ); }
	inline bool operator!= (objectID a)		{ return( m_objectID != a ); }
	inline bool operator== (objectID a)		{ return( m_objectID == a ); }

private:
	objectID m_objectID;
	int m_id;
	StateVariableScope m_scope;
	StateMachine* m_stateMachine;
	bool m_writeback;
};

class StateVariablePointerVoid
{
public:
	StateVariablePointerVoid( int id, StateMachine* sm, StateVariableScope scope, bool init )	{ m_writeback = false; if( !init ) { m_id = id; m_stateMachine = sm; m_scope = scope; m_pointervoid = m_stateMachine->GetStateVariablePointer( m_id, m_scope ); } else { sm->DeclareVariable( id, scope ); } }
	~StateVariablePointerVoid( void )															{ if( m_writeback ) { m_stateMachine->SetStateVariablePointer(m_pointervoid, m_id, m_scope); } }
	
	inline operator void*()				{ return m_pointervoid; }
	inline void* operator= (void* a)	{ m_writeback = true; return( m_pointervoid = a ); }
	inline void* operator-> ()			{ return m_pointervoid; }

private:
	void* m_pointervoid;
	int m_id;
	StateVariableScope m_scope;
	StateMachine* m_stateMachine;
	bool m_writeback;
};

class StateVariablePointerVector2
{
public:
	StateVariablePointerVector2( int id, StateMachine* sm, StateVariableScope scope, bool init )	{ m_writeback = false; if( !init ) { m_id = id; m_stateMachine = sm; m_scope = scope; m_pointervector2 = m_stateMachine->GetStateVariableVector2( m_id, m_scope ); } else { sm->DeclareVariable( id, scope ); } }
	~StateVariablePointerVector2( void )															{ if( m_writeback ) { m_stateMachine->SetStateVariableVector2(m_pointervector2, m_id, m_scope); } }

	inline operator D3DXVECTOR2*()					{ return m_pointervector2; }
	inline D3DXVECTOR2* operator= (D3DXVECTOR2* a)	{ m_writeback = true; return( m_pointervector2 = a ); }
	inline D3DXVECTOR2* operator-> ()				{ return m_pointervector2; }

private:
	D3DXVECTOR2* m_pointervector2;
	int m_id;
	StateVariableScope m_scope;
	StateMachine* m_stateMachine;
	bool m_writeback;
};

class StateVariablePointerVector3
{
public:
	StateVariablePointerVector3( int id, StateMachine* sm, StateVariableScope scope, bool init )	{ m_writeback = false; if( !init ) { m_id = id; m_stateMachine = sm; m_scope = scope; m_pointervector3 = m_stateMachine->GetStateVariableVector3( m_id, m_scope ); } else { sm->DeclareVariable( id, scope ); } }
	~StateVariablePointerVector3( void )															{ if( m_writeback ) { m_stateMachine->SetStateVariableVector3(m_pointervector3, m_id, m_scope); } }

	inline operator D3DXVECTOR3*()					{ return m_pointervector3; }
	inline D3DXVECTOR3* operator= (D3DXVECTOR3* a)	{ m_writeback = true; return( m_pointervector3 = a ); }
	inline D3DXVECTOR3* operator-> ()				{ return m_pointervector3; }

private:
	D3DXVECTOR3* m_pointervector3;
	int m_id;
	StateVariableScope m_scope;
	StateMachine* m_stateMachine;
	bool m_writeback;
};
