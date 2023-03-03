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

#pragma warning(disable: 4995)

#include "statemch.h"
#include "gameobject.h"
#include "global.h"
#include "singleton.h"
#include <list>

#define REGISTER_MESSAGE_NAME(x) #x,
static const char* MessageNameText[] =
{
	#include "msgnames.h"
	"Invalid"
};
#undef REGISTER_MESSAGE_NAME


class LogEntry
{
public:
	
	LogEntry( void );
	~LogEntry( void ) {}

	objectID m_owner;
	char m_name[GAME_OBJECT_MAX_NAME_SIZE];
	bool m_handled;

	float m_timestamp;
	char m_statename[64];
	char m_substatename[64];
	char m_eventmsgname[64];

	//msg only info
	bool m_msg;
	objectID m_receiver;
	objectID m_sender;
	unsigned int m_data;


};

class DebugLog : public Singleton <DebugLog>
{
public:

	DebugLog( void ) {}
	~DebugLog( void );

	void LogStateMachineEvent( objectID id, char* name, MSG_Object * msg, const char* statename, const char* substatename, char* eventmsgname, bool handled ); 
	void LogStateMachineEvent( objectID id, char* name, MSG_Object * msg, const char* statename, const char* substatename, MSG_Name eventmsgname, bool handled ); 
	void LogStateMachineStateChange( objectID id, char* name, unsigned int state, int substate );

	const char * TranslateMsgNameToString( MSG_Name msgname )		{ return( MessageNameText[ msgname ] ); }

	void Dump( objectID id );

	void OutputDebugStringX( const wchar_t * string, ... ) { va_list args; va_start(args, string); wchar_t buf[2048]; vswprintf(buf, string, args); OutputDebugString(buf); }


private:

	typedef std::list<LogEntry*> LoggingContainer;

	LoggingContainer m_log;

	void PrintLogEntry( LogEntry& entry );

};
