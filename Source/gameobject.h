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

#include <list>
#include "global.h"



//Add new object types here (bitfield mask - objects can be combinations of types)
#define OBJECT_Ignore_Type  (0)
#define OBJECT_Gameflow     (1<<1)
#define OBJECT_Character    (1<<2)
#define OBJECT_NPC          (1<<3)
#define OBJECT_Player       (1<<4)
#define OBJECT_Enemy        (1<<5)
#define OBJECT_Weapon       (1<<6)
#define OBJECT_Item         (1<<7)
#define OBJECT_Projectile   (1<<8)

#define GAME_OBJECT_MAX_NAME_SIZE 64


//Forward declarations
class StateMachineManager;
class MSG_Object;



class GameObject
{
public:

	GameObject( objectID id, unsigned int type, char* name );
	~GameObject( void );

	inline objectID GetID( void )					{ return( m_id ); }
	inline unsigned int GetType( void )				{ return( m_type ); }
	inline char* GetName( void )					{ return( m_name ); }
	
	void Update( void );

	//State machine related
	void CreateStateMachineManager( void );
	inline StateMachineManager* GetStateMachineManager( void )	{ ASSERTMSG(m_stateMachineManager, "GameObject::GetStateMachineManager - m_stateMachineManager not set"); return( m_stateMachineManager ); }

	//Scheduled deletion
	inline void MarkForDeletion( void )				{ m_markedForDeletion = true; }
	inline bool IsMarkedForDeletion( void )			{ return( m_markedForDeletion ); }

	int GetHealth(void)								{ return( m_health ); }
	void SetHealth(int health)						{ m_health = health; }

private:

	objectID m_id;									//Unique id of object (safer than a pointer).
	unsigned int m_type;							//Type of object (can be combination).
	bool m_markedForDeletion;						//Flag to delete this object (when it is safe to do so).
	char m_name[GAME_OBJECT_MAX_NAME_SIZE];			//String name of object.

	StateMachineManager* m_stateMachineManager;
	
	
	int m_health;


};
