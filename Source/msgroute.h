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

#include "msg.h"
#include "time.h"
#include "singleton.h"
#include <list>

//Forward declaration
enum StateMachineQueue;


typedef std::list<MSG_Object*> MessageContainer;

class MsgRoute : public Singleton <MsgRoute>
{
public:

	MsgRoute( void );
	~MsgRoute( void );

	void DeliverDelayedMessages( void );

	void SendMsg( float delay, MSG_Name name,
	              objectID receiver, objectID sender, 
	              Scope_Rule rule, unsigned int scope,
	              StateMachineQueue queue, MSG_Data& data, 
				  bool timer, bool cc );
	
	void SendMsgBroadcast( MSG_Object & msg, unsigned int type = 0 );

	//Delayed message load balancing
	inline void SetLoadBalancingConstraint(float maxTimePerFrameInSeconds)	{ m_loadBalancingTimeLimit = maxTimePerFrameInSeconds; }
	
	//Removing delayed messages
	void RemoveMsg( MSG_Name name, objectID receiver, objectID sender, bool timer );
	void PurgeScopedMsg( objectID receiver, StateMachineQueue queue );

	//For testing (unit tests)
	bool VerifyDelayedMessageOrder( void );

private:

	MessageContainer m_delayedMessages;
	float m_loadBalancingTimeLimit;

	void RouteMsg( MSG_Object & msg );	

};
