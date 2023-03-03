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

#include "global.h"


//Macro trick to make message names enums
//rom the file msgnames.h
#define REGISTER_MESSAGE_NAME(x) x,
typedef enum
{
	#include "msgnames.h"
	MSG_NUM
} MSG_Name;
#undef REGISTER_MESSAGE_NAME


//Delayed messages can be scoped with the following enum.
//
//Scoping explanation:
//A message scoped to a state or substate will not be delivered
//if the state or substate changes (only applies to delayed
//messages sent to yourself). If a delayed message scoped to the
//state is sent from state "A" and the state changes to "B" then 
//back to "A", the message will not be delivered since there was 
//a state change. By default, delayed messages are scoped to the 
//substate (which is equivalient to being scoped to the state if 
//there are no substates).
//
enum Scope_Rule { 
	SCOPE_TO_SUBSTATE,
	SCOPE_TO_STATE,
	SCOPE_TO_STATE_MACHINE
};

#define NEXT_FRAME 0.0001f


union MSG_Data_Union
{
	int intValue;
	float floatValue;
	bool boolValue;
	objectID objectIDValue;
	void* pointerValue;
	float x;
};

enum MSG_Data_Value
{
	MSG_DATA_INVALID,
	MSG_DATA_INT,
	MSG_DATA_FLOAT,
	MSG_DATA_BOOL,
	MSG_DATA_OBJECTID,
	MSG_DATA_POINTER,
	MSG_DATA_VECTOR2,
	MSG_DATA_VECTOR3
};
	
class MSG_Data
{
public:
	MSG_Data( void )							{ m_valueType = MSG_DATA_INVALID; }
	MSG_Data( int data )						{ m_data.intValue = data; m_valueType = MSG_DATA_INT; }
	MSG_Data( float data )						{ m_data.floatValue = data; m_valueType = MSG_DATA_FLOAT; }
	MSG_Data( bool data )						{ m_data.boolValue = data; m_valueType = MSG_DATA_BOOL; }
	MSG_Data( objectID data )					{ m_data.objectIDValue = data; m_valueType = MSG_DATA_OBJECTID; }
	MSG_Data( void* data )						{ m_data.pointerValue = data; m_valueType = MSG_DATA_POINTER; }
	MSG_Data( D3DXVECTOR2 data )				{ m_data.x = data.x; y = data.y; m_valueType = MSG_DATA_VECTOR2; }
	MSG_Data( D3DXVECTOR3 data )				{ m_data.x = data.x; y = data.y; z = data.z; m_valueType = MSG_DATA_VECTOR3; }

	~MSG_Data()	{}

	inline bool IsValid( void )					{ return( m_valueType != MSG_DATA_INVALID ); }
	inline bool IsInt( void )					{ return( m_valueType == MSG_DATA_INT ); }
	inline bool IsFloat( void )					{ return( m_valueType == MSG_DATA_FLOAT ); }
	inline bool IsBool( void )					{ return( m_valueType == MSG_DATA_BOOL ); }
	inline bool IsObjectID( void )				{ return( m_valueType == MSG_DATA_OBJECTID ); }
	inline bool IsPointer( void )				{ return( m_valueType == MSG_DATA_POINTER ); }
	inline bool IsVector2( void )				{ return( m_valueType == MSG_DATA_VECTOR2 ); }
	inline bool IsVector3( void )				{ return( m_valueType == MSG_DATA_VECTOR3 ); }

	inline MSG_Data_Value GetType( void )		{ return( m_valueType ); }

	inline void SetInt( int data )				{ m_data.intValue = data; m_valueType = MSG_DATA_INT; }
	inline int GetInt( void )					{ ASSERTMSG( m_valueType == MSG_DATA_INT, "Message data not of correct type" ); return( m_data.intValue ); }
	inline float GetFloat( void )				{ ASSERTMSG( m_valueType == MSG_DATA_FLOAT, "Message data not of correct type" ); return( m_data.floatValue ); }
	inline bool GetBool( void )					{ ASSERTMSG( m_valueType == MSG_DATA_BOOL, "Message data not of correct type" ); return( m_data.boolValue ); }
	inline objectID GetObjectID( void )			{ ASSERTMSG( m_valueType == MSG_DATA_OBJECTID, "Message data not of correct type" ); return( m_data.objectIDValue ); }
	inline void* GetPointer( void )				{ ASSERTMSG( m_valueType == MSG_DATA_POINTER, "Message data not of correct type" ); return( m_data.pointerValue ); }
	inline D3DXVECTOR2 GetVector2( void )		{ ASSERTMSG( m_valueType == MSG_DATA_VECTOR2, "Message data not of correct type" ); D3DXVECTOR2 v; v.x = m_data.x; v.y = y; return( v ); }
	inline D3DXVECTOR3 GetVector3( void )		{ ASSERTMSG( m_valueType == MSG_DATA_VECTOR3, "Message data not of correct type" ); D3DXVECTOR3 v; v.x = m_data.x; v.y = y; v.z = z; return( v ); }

	bool operator== (MSG_Data& a);
	//bool operator!= (MSG_Data& a)				{ return( !(this == a) ); }

private:
	MSG_Data_Value m_valueType;
	MSG_Data_Union m_data;
	float y, z;	//For 2D and 3D vectors
};


class MSG_Object
{
public:

	MSG_Object( void );
	MSG_Object( float deliveryTime, MSG_Name name, 
	            objectID sender, objectID receiver, 
	            Scope_Rule rule, unsigned int scope, 
				unsigned int queue, MSG_Data& data, 
				bool timer, bool cc );
	            
	~MSG_Object( void ) {}

	inline MSG_Name GetName( void )					{ return( m_name ); }
	inline void SetName( MSG_Name name )			{ m_name = name; }

	inline objectID GetSender( void )				{ return( m_sender ); }
	inline void SetSender( objectID sender )		{ m_sender = sender; }

	inline objectID GetReceiver( void )				{ return( m_receiver ); }
	inline void SetReceiver( objectID receiver )	{ m_receiver = receiver; }

	inline Scope_Rule GetScopeRule( void )			{ return( (Scope_Rule)m_scopeRule ); }
	inline void SetScopeRule( Scope_Rule rule )		{ m_scopeRule = rule; }

	inline unsigned int GetScope( void )			{ return( m_scope ); }
	inline void SetScope( unsigned int scope )		{ m_scope = scope; }

	inline unsigned int GetQueue( void )			{ return( m_queue ); }
	inline void SetQueue( unsigned int queue )		{ ASSERTMSG( queue < 8, "MSG_Object::SetQueue - queue out of bounds for 3 bit encoding. Change encoding if needed." ); m_queue = queue; }

	inline float GetDeliveryTime( void )			{ return( m_deliveryTime ); }
	inline void SetDeliveryTime( float time )		{ m_deliveryTime = time; }

	inline bool IsDelivered( void )					{ return( m_delivered ); }
	inline void SetDelivered( bool value )			{ m_delivered = value; }

	inline bool IsValidData( void )					{ return( m_data.IsValid() ); }
	inline bool IsIntData( void )					{ return( m_data.IsInt() ); }
	inline bool IsFloatData( void )					{ return( m_data.IsFloat() ); }
	inline bool IsBoolData( void )					{ return( m_data.IsBool() ); }
	inline bool IsObjectIDData( void )				{ return( m_data.IsObjectID() ); }
	inline bool IsPointerData( void )				{ return( m_data.IsPointer() ); }
	inline bool IsVector2Data( void )				{ return( m_data.IsVector2() ); }
	inline bool IsVector3Data( void )				{ return( m_data.IsVector3() ); }

	inline MSG_Data_Value GetDataType( void )		{ return( m_data.GetType() ); }

	inline void SetIntData( int data )				{ m_data.SetInt( data ); }
	inline int GetIntData( void )					{ return( m_data.GetInt() ); }
	inline float GetFloatData( void )				{ return( m_data.GetFloat() ); }
	inline bool GetBoolData( void )					{ return( m_data.GetBool() ); }
	inline objectID GetObjectIDData( void )			{ return( m_data.GetObjectID() ); }
	inline void* GetPointerData( void )				{ return( m_data.GetPointer() ); }
	inline D3DXVECTOR2 GetVector2Data( void )		{ return( m_data.GetVector2() ); }
	inline D3DXVECTOR3 GetVector3Data( void )		{ return( m_data.GetVector3() ); }

	inline MSG_Data& GetMsgData( void )				{ return( m_data ); }

	inline bool IsTimer( void )						{ return( m_timer ); }
	inline void SetTimer( bool value )				{ m_timer = value; }
	
	inline bool IsCC( void )						{ return( m_cc ); }
	inline void SetCC( bool value )					{ m_cc = value; }
	

private:

	MSG_Name m_name;				//Message name
	objectID m_sender;				//Object that sent the message
	objectID m_receiver;			//Object that will get the message
	MSG_Data m_data;				//Data that is passed with the message
	float m_deliveryTime;			//Time at which to send the message
	unsigned int m_scope;			//State or substate instance in which the receiver is allowed to get the message

	unsigned int m_queue: 3;		//Queue index to deliver message to (only valid when sender = receiver)
	unsigned int m_scopeRule: 2;	//Rule for how to interpret scope
	unsigned int m_delivered: 1;	//Whether the message has been delivered
	unsigned int m_timer: 1;		//Message is sent periodically
	unsigned int m_cc: 1;			//Message is a carbon copy that was received by someone else
};
