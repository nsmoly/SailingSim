/* 
 * Sailboat simulation game.
 *
 * Copyright (c) Nikolai Smolyanskiy, 2009-2010. All rights reserved. 
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute.
 */

#pragma once

#include <list>

#include "FSM.h"
#include "Ship.h"

#define NPC_RANDOM_COURSE_CHANGE_DELTA_TIME		15.0f				// in seconds
#define NPC_COLLISION_COURSE_CHANGE_DELTA_TIME	2.0f				// in seconds
#define NPC_RACE_COURSE_CHANGE_DELTA_TIME		2.0f				// in seconds
#define NPC_RACE_TACK_COURSE_CHANGE_DELTA_TIME	20.0f				// in seconds
#define NPC_RACEPOINT_RADIUS					20.0f				// in meters
#define DELTA_AWAY_FROM_WIND                    D3DX_PI/180.0f*3.0f // keep 3 degrees away from the "in irons" edge

class Sailor: public FSM<Sailor>
{
public: 

    enum SailorFSMEvents
    {
        AllIsGoodEvent = 0,
        StuckAgainstWindEvent,
        CollisionEvent,
        ObstacleDetectedEvent,
        MarkerReachedEvent
    };

	Sailor(std::tr1::shared_ptr<Ship>& ship, std::list<D3DXVECTOR3>& racePoints);
    virtual ~Sailor();

	void OnSailing(double time, FSMEvent event);
    void OnBackWinding(double time, FSMEvent event);
    void OnAvoidingCollision(double time, FSMEvent event);
    void OnCollision(double time, FSMEvent event);
    void OnMarkerReached(double time, FSMEvent event);

private:
	float PickRandomCourse();
    float CalculateCourseToPoint(const D3DXVECTOR3& point, bool& isTacking);
    float CalculateCourseBetweenPoints(const D3DXVECTOR3& startPoint, const D3DXVECTOR3& endPoint);
    float CalculateNextTackCourse();

	std::tr1::shared_ptr<Ship> m_ship;
	double m_lastCourseChangeTime;	  // last time in seconds from the start of the simulation the course was changed - used for random course plotting in NPC case
	double m_lastCourseChangeDueToCollisionTime;
	std::list<D3DXVECTOR3> m_racePoints;
	std::list<D3DXVECTOR3>::iterator m_nextRacePoint;
    std::list<D3DXVECTOR3>::iterator m_prevRacePoint;
};
