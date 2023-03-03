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

#include <assert.h>
#include "DXUT\DXUT.h"

#define ASSERTMSG(eval, message) assert(eval && message)
#define COMPILE_TIME_ASSERT(expression, message) { typedef int ASSERT__##message[1][(expression)]; }


#define g_time Time::GetSingleton()
#define g_database Database::GetSingleton()
#define g_msgroute MsgRoute::GetSingleton()
#define g_debuglog DebugLog::GetSingleton()
#define g_debugdrawing DebugDrawing::GetSingleton()


#define INVALID_OBJECT_ID 0
#define SYSTEM_OBJECT_ID 1


typedef unsigned int objectID;
