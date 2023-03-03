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

#include "singleton.h"
#include "DXUT\DXUTmisc.h"


class Time : public Singleton <Time>
{
public:

	Time( void );
	~Time( void ) {}

	void MarkTimeThisTick( void );
	inline float GetElapsedTime( void )			{ return( m_timeLastTick ); }
	inline float GetCurTime( void )				{ return( m_currentTime ); }
	inline double GetAbsoluteTime( void )		{ return( m_timer.GetAbsoluteTime() ); }
	inline double GetHighestResolutionTime( void )	{ LARGE_INTEGER qwTime; QueryPerformanceCounter( &qwTime ); return((double)qwTime.QuadPart); }


private:

	unsigned int m_startTime;
	float m_currentTime;
	float m_timeLastTick;
	CDXUTTimer m_timer;

};
