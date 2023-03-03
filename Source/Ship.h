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
#include <vector>
#include "Node.h"

#define SHIP_LENGTH								15.0f				// ship length in meters
#define RUDDER_TO_MAST_DISTANCE_IN_MODEL		228.0f				// distance between rudder center and mast's center (model's origin) in the ship model in model's coordinates
#define FLAG_TO_MAST_DISTANCE_IN_MODEL			1.268f				// distance between flag location and mast's center (model's origin) in the ship model in model's coordinates
#define MAINSAIL_MAX_ANGLE						D3DX_PI / 3.0f		// 60 degrees
#define RUDDER_MAX_ANGLE						D3DX_PI / 4.0f		// 45 degrees
#define NO_SAIL_ANGLE_TO_WIND					D3DX_PI/6.0f		// 30 degrees "no sail" zone between ship course and wind
#define TARGET_COURSE_EPSILON					D3DX_PI / 36.0f		// 5 degrees
#define SAILFORCE_EPSILON						0.1f				// in newtons
#define LINEAR_SPEED_EPSILON					0.1f				// in m/s
#define ANGULAR_SPEED_EPSILON					0.001f				// in radians/s

/**
	Class that represents a ship in this game
*/
class Ship : public Node
{
public:
	enum CollisionStatus
	{
		NoCollision = 0,
		WorldBorderCollision,
		LandCollision,
		ShipCollision
	};

	enum CollisionPlace
	{
		NoPlace = 0,
		ForwardLeftSide,
		ForwardRightSide,
		BackLeftSide,
		BackRightSide
	};

	Ship(IDirect3DDevice9* pd3dDevice, bool playable = false);
	virtual	~Ship();

	virtual HRESULT Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath);

	// Update traversal for physics, controls, etc.
	virtual void Update(double fTime);

	// Render traversal for drawing objects
	virtual void Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);

	bool IsPlayable() { return m_playable; }
	void SetSailorId(unsigned short id) { m_sailorId = id; }

	// Positioning functions
	D3DXVECTOR3 GetPosition() { return D3DXVECTOR3( m_x, 0, m_z ); }
	void SetPosition(D3DXVECTOR3 newPosition) { m_x = newPosition.x; m_z = newPosition.z; }
	float GetSpeed() { return m_shipSpeed; }
	void SetSpeed(float newSpeed) { m_shipSpeed = newSpeed; }
	float GetShipToXAxesAngle() { return m_shipToXAxesAngle; }
	void SetShipToXAxesAngle(float newShipToXAxesAngle) { m_shipToXAxesAngle = newShipToXAxesAngle; }
	float GetWindToXAxesAngle() { return m_windToXAxesAngle; }

	// Physical status functions
	float GetLength() { return m_shipLength; }
	float GetWidth() { return m_shipWidth; }
	void GetBoundingBox(D3DXVECTOR3* pBoundingBox);
    float GetSailsLiftForce() { return m_sailsLiftForce; }
    float GetSailsPushForce() { return m_sailsPushForce; }

	// Control functions
	void SetCourse(float courseToXAxesAngle) { m_targetCourseAngleToXAxes = courseToXAxesAngle; }
	
	// Status functions
	void SetCollisionData(CollisionStatus status, CollisionPlace place) { m_collisionStatus = status; m_collisionPlace = place; }
	void GetCollisionData(CollisionStatus& status, CollisionPlace& place) { status = m_collisionStatus; place = m_collisionPlace; }
	bool IsPointVisible(const D3DXVECTOR3& point, D3DXVECTOR3& endOfLineOfSight);
	bool IsInIrons();

private:
	void UpdateModel(float deltaTime);
	void CalculateSailsForces(float shipToXAxesAngle, float sailsAngle, float windSpeed, float windToXAxesAngle, float& sailsLiftForce, float& windPushForce);
	float CalculateOptimalSailTrimChange(float deltaTime);
	float CalculateRudderCorrection(float deltaTime);
	void UpdateSailsOrientation(float deltaTime);
	void UpdateRudderOrientation(float deltaTime, bool wasTeelerTouched);
	void UpdateShipTransform();
	void UpdateSailsTransforms();
	void UpdateRudderTransform();
	void UpdateFlagTransform();
	void RenderDebugInformation(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);
	void NotifySailor();

	// Mesh subsets in the model are:
	enum ShipSubsets
	{
		Rudder = 3,
		Jib = 5,
		Mainsail = 7,
		Boom = 9,
		Mainsheets = 10,
		Forestay = 12,
		Flag = 20
	};

	// Time in seconds from the start of the simulation
	double m_lastTime;

	// NPC parameters
	bool m_playable;	// true if this ship object is playable by a used and false if this is NPC
	unsigned short m_sailorId;	// ID of a state machine that controls this ship if it is NPC
	float m_targetCourseAngleToXAxes; // used for setting a course for the autopilot

	// Meshes and textures
	LPD3DXMESH m_pMesh;
	D3DMATERIAL9* m_pMeshMaterials;
	LPDIRECT3DTEXTURE9* m_pMeshTextures;
	DWORD m_numberOfMaterials;

	// Model parts matrices
	D3DXVECTOR3 m_scaleVector;
	D3DXMATRIX m_shipMatrix;
	D3DXMATRIX m_mainsailMatrix;
	D3DXMATRIX m_jibMatrix;
	D3DXMATRIX m_boomMatrix;
	D3DXMATRIX m_rudderMatrix;
	D3DXMATRIX m_flagMatrix;

	// Math model
	float m_x;				// ship's position in meters
	float m_z;
	float m_sailsAngle;		// angle between ship's direction (if we look at the stern) and sails (0 - sails are all in, positive angle - sails to the right, negative - to the left) in radians
	float m_rudderAngle;	// angle between ship's direction (if we look at the stern) and its rudder (0 - rudder are straight, positive angle is rudder to the right, negative - to the left) in radians
	float m_shipSpeed;		// ship's linear speed in m/s
	float m_shipAngularSpeed;	// ship's angular speed radians/s
	float m_shipToXAxesAngle;	// angle between X axes and ship's direction in radians (0 angle - ship is going in parallel to X axes from 0 to infinity)
	float m_windSpeed;			// in m/s, constant for now
	float m_windToXAxesAngle;	// angle between X axes and wind's direction in radians (0 angle - wind is blowing in parallel to X axes from 0 to infinity)

	// Ship's parameters
	float m_shipMaxSpeed;			// ship's hull max theoretical speed given its length, in m/s. Calculated as 2.5*sqrt(shipHullLengthInMeters) (in knots) and then converted from knots to m/s by multiplying by 0.51444
	float m_shipLength;				// ship's length in meters
	float m_shipWidth;				// ship's width in meters
	float m_sailRotationSpeed;		// angular speed with wich the mainsail can be moved left or right (radians/sec)
	float m_rudderRotationSpeed;	// angular speed with wich the rudder can be moved left or right (radians/sec)
	float m_mastToGCenterDistance;	// distance between ship's center of gravity and mast in meters
	float m_rudderToGCenterDistance;	// distance between ship's center of gravity and rudder in meters
	float m_mainSailHalfSize;			// half length of the main sail in meters (or half boom's length)
	float m_shipMass;					// ship's mass in kilograms
	float m_shipMomentOfInertia;		// ship's moment of inertia (angular mass) in kg*m^2

	// Magic constants describing ship's hull and sails
	float m_sailsLiftEfficiencyConstant;
	float m_sailsPushEfficiencyConstant;
	float m_rudderEfficiencyConstant;
	float m_hullForwardWaterResistance;
	float m_hullLateralWaterResistance;

	// Sails current motion parameters
	float m_jibOrientation;		// if 1 then wind blows from the left, if -1 then wind blows from the right
	float m_mainsailOrientation;	// mainsail orientation which may be different from jib - 1 if it is to the right and -1 if it is to the left
	float m_leftAllowedSailsAngle;	// max angle at which sails can be moved to the left (negative or 0) if looking from the aft to the bow
	float m_rightAllowedSailsAngle;	// max angle at which sails can be moved to the right (positive or 0) if looking from the aft to the bow
    
    // For testing
    float m_sailsLiftForce;
    float m_sailsPushForce;

	// Collision detection and line of sight related stuff
	D3DXVECTOR3 m_boundingBox[4];
	CollisionStatus m_collisionStatus;
	CollisionPlace m_collisionPlace;
	D3DXVECTOR3 m_endOfLineOfSight;
	D3DXVECTOR3 m_waveNormal;
};
