/* 
 * Sailboat simulation game.
 *
 * Copyright (c) Nikolai Smolyanskiy, 2009-2010. All rights reserved. 
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute.
 */

#include "DXUT.h"

#include "Utilities.h"
#include "Sailor.h"
using namespace std;

extern std::tr1::shared_ptr<Ship> g_playerShip;

Sailor::Sailor(std::tr1::shared_ptr<Ship>& ship, std::list<D3DXVECTOR3>& racePoints) : 
	m_lastCourseChangeTime(0),
	m_lastCourseChangeDueToCollisionTime(0),
	m_ship(ship),
	m_racePoints(racePoints)
{
	RegisterState(0, &Sailor::OnSailing);
    RegisterState(1, &Sailor::OnBackWinding);
    RegisterState(2, &Sailor::OnAvoidingCollision);
    RegisterState(3, &Sailor::OnCollision);
    RegisterState(4, &Sailor::OnMarkerReached);
    
    AddTransition(0, AllIsGoodEvent, 0);
    
    AddTransition(0, StuckAgainstWindEvent, 1);
    AddTransition(1, AllIsGoodEvent, 0);
    
    AddTransition(0, ObstacleDetectedEvent, 2);
    AddTransition(2, AllIsGoodEvent, 0);
    AddTransition(2, StuckAgainstWindEvent, 1);
    
    AddTransition(0, CollisionEvent, 3);
    AddTransition(3, AllIsGoodEvent, 0);
    AddTransition(3, StuckAgainstWindEvent, 1);
    
    AddTransition(0, MarkerReachedEvent, 4);
    AddTransition(4, AllIsGoodEvent, 0);
            
    SetCurrentState(0);

	m_nextRacePoint = m_racePoints.begin();
    m_prevRacePoint = m_racePoints.end();
}

Sailor::~Sailor()
{
}

void Sailor::OnSailing(double time, FSMEvent event)
{
    double timeSinceLastCourseChange = time - m_lastCourseChangeTime;

    D3DXVECTOR3 shipPosition = m_ship->GetPosition();
	D3DXVECTOR3 bearingToNextPoint = *m_nextRacePoint - shipPosition;
	float distanceToNextRacePoint = D3DXVec3Length(&bearingToNextPoint);
	if(distanceToNextRacePoint<=NPC_RACEPOINT_RADIUS)
	{
        m_prevRacePoint = m_nextRacePoint;
		++m_nextRacePoint;
		if(m_nextRacePoint==m_racePoints.end())
		{
			m_nextRacePoint = m_racePoints.begin();
		}
		DXUTOutputDebugStringA("Sailor %d, switched to moving to the next race point: (%f, %f)", GetId(), m_nextRacePoint->x, m_nextRacePoint->y);
	}

    // Check if we need to tack
    float windToXAxesAngle = m_ship->GetWindToXAxesAngle();
	float nosailDirectionToXAxesAngle = Utilities::NormalizeAngleInRadians(windToXAxesAngle + D3DX_PI);
    //float courseBetweenWayPoints = CalculateCourseBetweenPoints(*m_prevRacePoint, *m_nextRacePoint);
    float courseBetweenWayPoints = CalculateCourseBetweenPoints(shipPosition, *m_nextRacePoint);
	if(abs(nosailDirectionToXAxesAngle - courseBetweenWayPoints)<NO_SAIL_ANGLE_TO_WIND)
	{
        if(timeSinceLastCourseChange > NPC_RACE_TACK_COURSE_CHANGE_DELTA_TIME)
	    {    
            // Tack
            float targetCourseAngleToXAxes = CalculateNextTackCourse();
            m_ship->SetCourse(targetCourseAngleToXAxes);
            m_lastCourseChangeTime = time;
        }
    }
    else if(timeSinceLastCourseChange > NPC_RACE_COURSE_CHANGE_DELTA_TIME)
	{
        // Adjust course
        bool isTacking = false;
		float targetCourseAngleToXAxes = CalculateCourseToPoint(*m_nextRacePoint, isTacking);
        m_ship->SetCourse(targetCourseAngleToXAxes);
        m_lastCourseChangeTime = time;
	}
}

void Sailor::OnBackWinding(double time, FSMEvent event)
{
	float windToXAxesAngle = m_ship->GetWindToXAxesAngle();
	float nosailDirectionToXAxesAngle = Utilities::NormalizeAngleInRadians(windToXAxesAngle + D3DX_PI);

	float targetCourseAngleToXAxes = Utilities::NormalizeAngleInRadians(nosailDirectionToXAxesAngle + 2.0f*NO_SAIL_ANGLE_TO_WIND); // go away from the wind since we are stuck "in irons"
	m_ship->SetCourse(targetCourseAngleToXAxes);
	m_lastCourseChangeTime = time;
}

void Sailor::OnAvoidingCollision(double time, FSMEvent event)
{
}

void Sailor::OnCollision(double time, FSMEvent event)
{
	// Update target course for NPC in case of collisions
	if(time - m_lastCourseChangeDueToCollisionTime > NPC_RANDOM_COURSE_CHANGE_DELTA_TIME)
	{
		float shipCourse = m_ship->GetShipToXAxesAngle();
		float targetCourseAngleToXAxes = Utilities::NormalizeAngleInRadians(shipCourse + D3DX_PI);
		m_ship->SetCourse(targetCourseAngleToXAxes);
		m_lastCourseChangeDueToCollisionTime = time;
	}
}

void Sailor::OnMarkerReached(double time, FSMEvent event)
{
}

/*
// Sets the course to hit the player ship. This is the original "seek and find" game implementation
void Sailor::OnSailing(double time, FSMEvent event)
{
	D3DXVECTOR3 playerPosition = g_playerShip->GetPosition();
	playerPosition.y = m_ship->GetWidth(); // check a point above the sea level
	D3DXVECTOR3 endOfLineOfSight;
	if( m_ship->IsPointVisible(playerPosition, endOfLineOfSight) )
	{
		// Player is visible - set collision course if it is time (we do it from time to time to avoid too much movement
		if(time - m_lastCourseChangeTime > NPC_COLLISION_COURSE_CHANGE_DELTA_TIME)
		{
			m_lastCourseChangeTime = time;
			float targetCourseAngleToXAxes = CalculateCourseToPoint(playerPosition);
			m_ship->SetCourse(targetCourseAngleToXAxes);
		}
	}
	else
	{
		// Player is not visible so pick a random course if it is time
		if(time - m_lastCourseChangeTime > NPC_RANDOM_COURSE_CHANGE_DELTA_TIME)
		{
			m_lastCourseChangeTime = time;
			float targetCourseAngleToXAxes = PickRandomCourse();
			m_ship->SetCourse(targetCourseAngleToXAxes);

			DXUTOutputDebugStringA("New random target course: %f\n", D3DXToDegree(targetCourseAngleToXAxes));
		}
	}
}
*/

// Picks a course that sets moves this ship towards the player's ship
float Sailor::CalculateCourseToPoint(const D3DXVECTOR3& point, bool& isTacking)
{
	float windToXAxesAngle = m_ship->GetWindToXAxesAngle();
	float nosailDirectionToXAxesAngle = Utilities::NormalizeAngleInRadians(windToXAxesAngle + D3DX_PI);
    float course = CalculateCourseBetweenPoints(m_ship->GetPosition(), point);

	if(abs(nosailDirectionToXAxesAngle - course)<NO_SAIL_ANGLE_TO_WIND)
	{
		if(nosailDirectionToXAxesAngle>=course)
		{
			course = Utilities::NormalizeAngleInRadians(nosailDirectionToXAxesAngle - NO_SAIL_ANGLE_TO_WIND - DELTA_AWAY_FROM_WIND);
		}
		else
		{
			course = Utilities::NormalizeAngleInRadians(nosailDirectionToXAxesAngle + NO_SAIL_ANGLE_TO_WIND + DELTA_AWAY_FROM_WIND);
		}
        
        isTacking = true;
	}

	return course;
}

// Calculate next tack course. The next tack is the oposite of the current tack
float Sailor::CalculateNextTackCourse()
{
	float windToXAxesAngle = m_ship->GetWindToXAxesAngle();
    float upwindDirectionToXAxesAngle = Utilities::NormalizeAngleInRadians(windToXAxesAngle + D3DX_PI);
    float currentCourse = m_ship->GetShipToXAxesAngle();

   	D3DXVECTOR3 upwindDir(cos(upwindDirectionToXAxesAngle), 0, sin(upwindDirectionToXAxesAngle));
	D3DXVECTOR3 shipDir(cos(currentCourse), 0, sin(currentCourse));
    D3DXVECTOR3 temp;
	D3DXVec3Cross(&temp, &upwindDir, &shipDir);
	float tackCoeff = (temp.y >= 0) ? 1.0f : -1.0f; // 1 = tack right, -1 = tack left
	float course = Utilities::NormalizeAngleInRadians(upwindDirectionToXAxesAngle + tackCoeff*(NO_SAIL_ANGLE_TO_WIND + DELTA_AWAY_FROM_WIND));

    return course;
}

// Picks random course which can be sailed (excludes direct upwind +-30 degrees zone, since we cannot sail there)
float Sailor::PickRandomCourse()
{
	float windToXAxesAngle = m_ship->GetWindToXAxesAngle();
	float course = 0;
	float nosailDirectionToXAxesAngle = Utilities::NormalizeAngleInRadians(windToXAxesAngle + D3DX_PI);

	do
	{
		// Pick random courses until they are not in the "no sail sector"
		course = (float(rand()) / float(RAND_MAX))*2.0f*D3DX_PI;

	} while( abs(nosailDirectionToXAxesAngle - course)<NO_SAIL_ANGLE_TO_WIND );

	return course;
}

float Sailor::CalculateCourseBetweenPoints(const D3DXVECTOR3& startPoint, const D3DXVECTOR3& endPoint)
{
    D3DXVECTOR3 direction = endPoint - startPoint;
	D3DXVec3Normalize(&direction, &direction);
	D3DXVECTOR3 xUnitVector = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	float course = acosf(D3DXVec3Dot(&direction, &xUnitVector));
	if(direction.z<0)
	{
		course = 2.0f*D3DX_PI - course;
	}

    return course;
}