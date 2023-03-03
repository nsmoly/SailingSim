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
#include "msg.h"

bool MSG_Data::operator== (MSG_Data& a)
{
	if( m_valueType == a.GetType() )
	{
		switch( m_valueType )
		{
			case MSG_DATA_INVALID:
				return( true );
			case MSG_DATA_INT:
				return( m_data.intValue == a.GetInt() );
			case MSG_DATA_FLOAT:
				return( m_data.floatValue == a.GetFloat() );
			case MSG_DATA_BOOL:
				return( m_data.boolValue == a.GetBool() );
			case MSG_DATA_OBJECTID:
				return( m_data.objectIDValue == a.GetObjectID() );
			case MSG_DATA_POINTER:
				return( m_data.pointerValue == a.GetPointer() );
			case MSG_DATA_VECTOR2:
				{
					D3DXVECTOR2 vec2 = a.GetVector2();
					return( m_data.x == vec2.x && y == vec2.y );
				}
			case MSG_DATA_VECTOR3:
				{
					D3DXVECTOR3 vec3 = a.GetVector3();
					return( m_data.x == vec3.x && y == vec3.y && z == vec3.z );
				}
			default:
				ASSERTMSG( 0, "Unknown type" );
				return false;
		}
	}
	else
	{
		return false;
	}
}

MSG_Object::MSG_Object( void )
: m_name( MSG_NULL ),
  m_sender( INVALID_OBJECT_ID ),
  m_receiver( INVALID_OBJECT_ID ),
  m_scopeRule( SCOPE_TO_STATE_MACHINE ),
  m_scope( 0 ),
  m_queue( 0 ),
  m_deliveryTime( 0.0f ),
  m_delivered( false ),
  m_timer( 0 ),
  m_cc( false )
{

}


MSG_Object::MSG_Object( float deliveryTime, MSG_Name name,
                        objectID sender, objectID receiver,
                        Scope_Rule rule, unsigned int scope,
                        unsigned int queue, MSG_Data& data, 
						bool timer, bool cc )
{
	SetDeliveryTime( deliveryTime );
	SetName( name );
	SetSender( sender );
	SetReceiver( receiver );
	SetScopeRule( rule );
	SetScope( scope );
	SetQueue( queue );
	SetDelivered( false );
	SetTimer( timer );
	SetCC( cc );
	m_data = data;
}

