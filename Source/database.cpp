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
#include "database.h"
#include "gameobject.h"
#include "statemch.h"


Database::Database( void )
: m_nextFreeID( SYSTEM_OBJECT_ID + 1 )
{

}

Database::~Database( void )
{
	dbContainer::iterator i = m_database.begin();
	while( i != m_database.end() )
	{	//Destroy object
		delete( *i );
		i = m_database.erase( i );
	}
}

/*---------------------------------------------------------------------------*
  Name:         Update

  Description:  Calls the update function for all objects within the database.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Database::Update( void )
{
	for( dbContainer::iterator i = m_database.begin(); i != m_database.end(); ++i )
	{
		(*i)->Update();
	}

	g_msgroute.DeliverDelayedMessages();

	//Destroy objects that have requested it
	dbContainer::iterator i = m_database.begin();
	while( i != m_database.end() )
	{
		if( (*i)->IsMarkedForDeletion() )
		{	//Destroy object
			delete( *i );
			i = m_database.erase( i );
		}
		else
		{
			++i;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgFromSystem

  Description:  Send a message from the system to a game object. For example,
                a keystroke or event can be sent in the form of a message.

  Arguments:    id : the ID of the object
                name : the name of the message
				data : data to be delivered with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Database::SendMsgFromSystem( objectID id, MSG_Name name, MSG_Data& data )
{
	GameObject* object = Find( id );

	if( object )
	{
		MSG_Data data;
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, id, SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(object->GetStateMachineManager())
		{
			object->GetStateMachineManager()->SendMsg( msg );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgFromSystem

  Description:  Send a message from the system to a game object. For example,
                a keystroke or event can be sent in the form of a message.

  Arguments:    object : pointer to the game object
                name : the name of the message
				data : data to be delivered with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Database::SendMsgFromSystem( GameObject* object, MSG_Name name, MSG_Data& data )
{
	if( object )
	{
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, object->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(object->GetStateMachineManager())
		{
			object->GetStateMachineManager()->SendMsg( msg );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgFromSystem

  Description:  Send a message from the system to all game objects. For example,
                a keystroke or event can be sent in the form of a message.

  Arguments:    name : the name of the message
				data : data to be delivered with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Database::SendMsgFromSystem( MSG_Name name, MSG_Data& data )
{
	for( dbContainer::iterator i=m_database.begin(); i!=m_database.end(); ++i )
	{
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, (*i)->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if((*i)->GetStateMachineManager())
		{
			(*i)->GetStateMachineManager()->SendMsg( msg );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         Store

  Description:  Stores an object within the database.

  Arguments:    object : the game object

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Database::Store( GameObject & object )
{
	if( Find( object.GetID() ) == 0 ) {
		m_database.push_back( &object );
	}
	else {
		ASSERTMSG( 0, "Database::Store - Object ID already represented in database." );
	}
}

/*---------------------------------------------------------------------------*
  Name:         Remove

  Description:  Removes an object from the database.

  Arguments:    id : the ID of the object

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Database::Remove( objectID id )
{
	for( dbContainer::iterator i=m_database.begin(); i!=m_database.end(); ++i )
	{
		if( (*i)->GetID() == id ) {
			m_database.erase(i);	
			return;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         Find

  Description:  Find an object given its id.

  Arguments:    id : the ID of the object

  Returns:      An object pointer. If object is not found, returns 0.
 *---------------------------------------------------------------------------*/
GameObject* Database::Find( objectID id )
{
	for( dbContainer::iterator i=m_database.begin(); i!=m_database.end(); ++i )
	{
		if( (*i)->GetID() == id ) {
			return( *i );
		}
	}

	return( 0 );

}

/*---------------------------------------------------------------------------*
  Name:         FindByName

  Description:  Find an object given its id.

  Arguments:    id : the ID of the object

  Returns:      An object pointer. If object is not found, returns 0.
 *---------------------------------------------------------------------------*/
GameObject* Database::FindByName( char* name )
{
	for( dbContainer::iterator i=m_database.begin(); i!=m_database.end(); ++i )
	{
		if( strcmp( (*i)->GetName(), name ) == 0 ) {
			return( *i );
		}
	}

	return( 0 );

}

/*---------------------------------------------------------------------------*
  Name:         GetIDByName

  Description:  Get an object's id given its name.

  Arguments:    name : the name of the object

  Returns:      An ID. If object is not found, returns INVALID_OBJECT_ID.
 *---------------------------------------------------------------------------*/
objectID Database::GetIDByName( char* name )
{
	for( dbContainer::iterator i=m_database.begin(); i!=m_database.end(); ++i )
	{
		if( strcmp( (*i)->GetName(), name ) == 0 ) {
			return( (*i)->GetID() );
		}
	}

	return( INVALID_OBJECT_ID );
}

/*---------------------------------------------------------------------------*
  Name:         GetNewObjectID

  Description:  Get a fresh object ID.

  Arguments:    None.

  Returns:      An id.
 *---------------------------------------------------------------------------*/
objectID Database::GetNewObjectID( void )
{
	return( m_nextFreeID++ );

}

/*---------------------------------------------------------------------------*
  Name:         ComposeList

  Description:  Compose a list of objects given certain type.

  Arguments:    list   : the list to fill with the result of the operation
                type   : the type of object to add to the list (optional)

  Returns:      None. (The result is stored in the list argument.)
 *---------------------------------------------------------------------------*/
void Database::ComposeList( dbCompositionList & list, unsigned int type )
{
	//Find all objects of "type"
	for( dbContainer::iterator i=m_database.begin(); i!=m_database.end(); ++i )
	{
		if( type == OBJECT_Ignore_Type || (*i)->GetType() & type )
		{	//Type matches
			list.push_back(*i);
		}
	}
}

