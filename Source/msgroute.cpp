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
#include "msgroute.h"
#include "statemch.h"
#include "database.h"



/*---------------------------------------------------------------------------*
  Name:         MsgRoute

  Description:  Constructor
 *---------------------------------------------------------------------------*/
MsgRoute::MsgRoute( void )
: m_loadBalancingTimeLimit(0.05f/60.0f) //5% of a 60Hz frame
{

}

/*---------------------------------------------------------------------------*
  Name:         ~MsgRoute

  Description:  Destructor
 *---------------------------------------------------------------------------*/
MsgRoute::~MsgRoute( void )
{
	for( MessageContainer::iterator i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
	{
		MSG_Object * msg = *i;
		delete( msg );
	}

	m_delayedMessages.clear();

}


/*---------------------------------------------------------------------------*
  Name:         SendMsg

  Description:  Sends a message through the message router. This function
                determines if the message should be delivered immediately
				or should be held until the delivery time.

  Arguments:    delay    : the number of seconds to delay the message
                name     : the message name
				receiver : the ID of the receiver
				sender   : the ID of the sender
				rule     : the scoping rule for the message
				scope    : the scope of the message (a state index)
				queue    : the queue to send the message to
				data     : a piece of data
				timer    : if this message is a timer (sent periodically)
				cc       : if this message is a CC (a copy)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::SendMsg( float delay, MSG_Name name,
                        objectID receiver, objectID sender,
                        Scope_Rule rule, unsigned int scope,
                        StateMachineQueue queue, MSG_Data& data, 
						bool timer, bool cc )
{

	if( delay <= 0.0f )
	{	//Deliver immediately
		MSG_Object msg( g_time.GetCurTime(), name, sender, receiver, rule, scope, queue, data, timer, cc );
		RouteMsg( msg );
	}
	else
	{	
		float deliveryTime = delay + g_time.GetCurTime();

		//Check for duplicates - time complexity O(n)
		bool set = false;
		float lastDeliveryTime = 0.0f;
		MessageContainer::iterator insertPosition = m_delayedMessages.end();
		MessageContainer::iterator i;
		for( i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
		{
			if( (*i)->IsDelivered() == false &&
				(*i)->GetName() == name &&
				(*i)->GetReceiver() == receiver &&
				(*i)->GetSender() == sender &&
				(*i)->GetScopeRule() == rule &&
				(*i)->GetScope() == scope &&
				(*i)->GetQueue() == queue &&
				(*i)->IsTimer() == timer &&
				(*i)->GetMsgData() == data )
			{	//Already in list - don't add
				ASSERTMSG(0, "MsgRoute::SendMsg - Message already in list. This assert is designed "
							 "to promote good coding practices. If you know what you're doing, you "
							 "can certainly remove this assert and have the engine silently ignore "
							 "redundant messages.");
				return;
			}

			//Sanity check that list is in order
			if( (*i)->GetDeliveryTime() < lastDeliveryTime )
			{
				ASSERTMSG( 0, "MsgRoute::SendMsg - Message list not in order" );
			}
			lastDeliveryTime = (*i)->GetDeliveryTime();

			if( !set && (*i)->GetDeliveryTime() > deliveryTime )
			{	//Record place to insert new delayed message (we need the entry one beyond)
				set = true;
				insertPosition = i;
			}
		}
		
		//Store in delivery list
		MSG_Object * msg = new MSG_Object( deliveryTime, name, sender, receiver, rule, scope, queue, data, timer, false );
		if( m_delayedMessages.empty() || deliveryTime <= m_delayedMessages.front()->GetDeliveryTime() )
		{	//Put at the front if the list is empty or the delivery time is sooner than the first entry
			m_delayedMessages.push_front( msg );
		}
		else
		{
			m_delayedMessages.insert( insertPosition, msg );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         VerifyDelayedMessageOrder

  Description:  Verifies that the delayed messages are being ordered properly.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
bool MsgRoute::VerifyDelayedMessageOrder( void )
{	//Test for order - time complexity O(n)
	float lastDeliveryTime = 0;

	MessageContainer::iterator i;
	for( i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
	{
		float time = (*i)->GetDeliveryTime();
		if( time < lastDeliveryTime )
		{
			ASSERTMSG( 0, "MsgRoute::VerifyDelayedMessageOrder - Message list not in order" );
			return false;
		}
		lastDeliveryTime = (*i)->GetDeliveryTime();
	}

	return true;
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgBroadcast

  Description:  Sends a message to every object of a certain type.

  Arguments:    msg    : the message to broadcast
                type   : the type of object (optional)

  Returns:      None.
 *---------------------------------------------------------------------------*/

void MsgRoute::SendMsgBroadcast( MSG_Object & msg, unsigned int type )
{
	dbCompositionList list;
	g_database.ComposeList( list, type );

	dbCompositionList::iterator i;
	for( i=list.begin(); i!=list.end(); ++i )
	{
		if( msg.GetSender() != (*i)->GetID() )
		{
			if((*i)->GetStateMachineManager())
			{
				(*i)->GetStateMachineManager()->SendMsg( msg );
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         DeliverDelayedMessages

  Description:  Sends delayed messages if the time is right.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::DeliverDelayedMessages( void )
{
	double timeStart = g_time.GetHighestResolutionTime();

	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		if( (*i)->GetDeliveryTime() <= g_time.GetCurTime() )
		{	//Deliver and delete msg
			MSG_Object * msg = *i;
			RouteMsg( *msg );
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{	//All messages beyond this one are not ready to fire, since the list is sorted
			return;
		}

		//Decide whether to stop sending for this frame
		double curTime = g_time.GetHighestResolutionTime();
		if( curTime - timeStart > m_loadBalancingTimeLimit )
		{
			return;
		}

	}
}

/*---------------------------------------------------------------------------*
  Name:         RouteMsg

  Description:  Routes the message to the receiver, only if the scoping rules
                allow it.

  Arguments:    msg : the message to route

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RouteMsg( MSG_Object & msg )
{
	GameObject * object = g_database.Find( msg.GetReceiver() );

	if( object != 0 && object->GetStateMachineManager() )
	{
		Scope_Rule rule = msg.GetScopeRule();
		if( rule == SCOPE_TO_STATE_MACHINE ||
			( rule == SCOPE_TO_SUBSTATE && msg.GetScope() == object->GetStateMachineManager()->GetStateMachine((StateMachineQueue)msg.GetQueue())->GetScopeSubstate() ) ||
			( rule == SCOPE_TO_STATE && msg.GetScope() == object->GetStateMachineManager()->GetStateMachine((StateMachineQueue)msg.GetQueue())->GetScopeState() ) )
		{	//Scope matches
			msg.SetDelivered( true );	//Important to set as delivered since timer messages 
										//will resend themselves immediately (and would get
										//thrown away if we didn't set this, since it would look
										//like a redundant message)
			
			if( msg.IsTimer() )
			{	//Timer message that occurs periodically
				float delay = msg.GetFloatData();	//Timer value stored in data field
				msg.SetIntData( 0 );				//Zero out data field
				//Queue up next periodic msg
				object->GetStateMachineManager()->GetStateMachine((StateMachineQueue)msg.GetQueue())->SetTimerExternal( delay, msg.GetName(), rule );
			}
			
			if( msg.IsCC() ) {
				object->GetStateMachineManager()->Process( EVENT_CCMessage, &msg, (StateMachineQueue)msg.GetQueue() );
			}
			else {
				object->GetStateMachineManager()->Process( EVENT_Message, &msg, (StateMachineQueue)msg.GetQueue() );
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         RemoveMsg

  Description:  Removes messages from the delayed message list that meet
                the criteria. This is useful to avoid duplicate delayed
				messages.

  Arguments:    name     : the name of the message
                receiver : the receiver ID of the message
				sender   : the sender ID of the message
				timer    : whether the message is a timer

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RemoveMsg( MSG_Name name, objectID receiver, objectID sender, bool timer )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		MSG_Object * msg = *i;
		if( msg->GetName() == name &&
			msg->GetReceiver() == receiver &&
			msg->GetSender() == sender &&
			msg->IsTimer() == timer &&
			!msg->IsDelivered() )
		{
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{
			++i;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         PurgeScopedMsg

  Description:  Removes messages from the delayed message list for a given
                receiver if the message is scoped to a particular state. This
				is useful if the receiver changes state machines, since the 
				messages are no longer valid.

  Arguments:    receiver : the receiver ID of the message
                queue    : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::PurgeScopedMsg( objectID receiver, StateMachineQueue queue )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		MSG_Object * msg = *i;
		if( msg->GetReceiver() == receiver &&
			msg->GetQueue() == queue &&
			msg->GetScopeRule() != SCOPE_TO_STATE_MACHINE &&
			!msg->IsDelivered() )
		{
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{
			++i;
		}
	}
}




