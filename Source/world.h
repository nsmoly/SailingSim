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

class Time;
class Database;
class MsgRoute;
class DebugLog;


#include <vector>

class World
{
public:
	World();
	~World();

	void InitializeSingletons( void );
	void Initialize( IDirect3DDevice9* pd3dDevice );
	void Update();

protected:

	bool m_initialized;
	Time* m_time;
	Database* m_database;
	MsgRoute* m_msgroute;
	DebugLog* m_debuglog;

};

