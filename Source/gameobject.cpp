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
#include "gameobject.h"
#include "msgroute.h"
#include "statemch.h"



GameObject::GameObject( objectID id, unsigned int type, char* name )
: m_markedForDeletion(false),
  m_stateMachineManager(0),
  m_health(0)
{
	m_id = id;
	m_type = type;
	
	if( strlen(name) < GAME_OBJECT_MAX_NAME_SIZE ) {
		strcpy( m_name, name );
	}
	else {
		strcpy( m_name, "invalid_name" );
		ASSERTMSG(0, "GameObject::GameObject - name is too long" );
	}
}

GameObject::~GameObject( void )
{
	if(m_stateMachineManager)
	{
		delete m_stateMachineManager;
	}
}

void GameObject::CreateStateMachineManager( void )
{
	m_stateMachineManager = new StateMachineManager( *this );
}

/*---------------------------------------------------------------------------*
  Name:         Update

  Description:  Calls the update function of the currect state machine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::Update( void )
{
	if(m_stateMachineManager)
	{
		m_stateMachineManager->Update();
	}
}


