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
#include "Game.h"
#include "Ship.h"
#include "Land.h"
#include "Ocean.h"
#include "Sailor.h"

#include <sstream>
using namespace std;

extern std::tr1::shared_ptr<FSMManager> g_fsmManager;
extern std::tr1::shared_ptr<Land> g_worldEnvironment;
extern std::tr1::shared_ptr<Ocean> g_ocean;
extern bool	g_isInDebugging;

extern IDirect3DVertexShader9* g_pVertexShader;
extern IDirect3DPixelShader9*  g_pPixelShader;
extern IDirect3DPixelShader9*  g_pPixelShaderTexture;

Ship::Ship(IDirect3DDevice9* pd3dDevice, bool playable) : 
	Node(pd3dDevice)
{
	m_playable = playable;
	m_sailorId = NOT_MACHINE;
	m_collisionStatus = Ship::NoCollision;
	m_collisionPlace = Ship::NoPlace;
	m_lastTime = 0;
	m_targetCourseAngleToXAxes = D3DX_PI/6.0f;
	
	m_pMesh = NULL;
	m_numberOfMaterials = 0;
	m_pMeshMaterials = NULL;
	m_pMeshTextures = NULL;

	// Rotation animation speeds (radians/second)
	m_sailRotationSpeed = D3DXToRadian(20.0f);
	m_rudderRotationSpeed = D3DXToRadian(30.0f);

	// Environment
	m_windSpeed = 10;	// meters/second
	m_windToXAxesAngle = Utilities::NormalizeAngleInRadians(-D3DX_PI/2.0f);

	// Math model is initialized when the ship's model is loaded
	m_shipLength = 0;
	m_shipWidth = 0;
	m_shipMass = 0;				
	m_shipMomentOfInertia = 0;	
	m_mainSailHalfSize = 0;
	m_mastToGCenterDistance = 0;
	m_rudderToGCenterDistance = 0; 
	m_hullLateralWaterResistance = 0;
	m_hullForwardWaterResistance = 0;
	m_sailsLiftEfficiencyConstant =  0;
	m_sailsPushEfficiencyConstant =  0;
	m_rudderEfficiencyConstant = 0;
	m_x = 0;
	m_z = 0;
	m_shipSpeed = 0;
	m_shipAngularSpeed = 0;
	m_shipToXAxesAngle = 0;
	m_sailsAngle = 0;
	m_rudderAngle = 0;

	// Sails state
	m_jibOrientation = 1;
	m_mainsailOrientation = 1;
	m_leftAllowedSailsAngle = -MAINSAIL_MAX_ANGLE;
	m_rightAllowedSailsAngle = MAINSAIL_MAX_ANGLE;
}

Ship::~Ship()
{
	Unload();

	for(size_t i=0; i<m_numberOfMaterials; i++)
	{
		SAFE_RELEASE(m_pMeshTextures[i]);
	}

	SAFE_DELETE_ARRAY(m_pMeshMaterials);
	SAFE_DELETE_ARRAY(m_pMeshTextures);
	SAFE_RELEASE(m_pMesh);	
}

HRESULT Ship::Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath)
{
	if(!szDataPath)
	{
		OutputDebugStringW(L"Invalid argument - path to level data file is not specified!");
		return E_INVALIDARG;
	}

	wstring xFilePath(szDataPath);
	xFilePath += L"\\sloop.x";

	if(FAILED(Utilities::LoadXModel(pd3dDevice, xFilePath.c_str(), &m_pMesh, &m_pMeshTextures, &m_numberOfMaterials)))
	{
		wchar_t message[128];
		swprintf_s(message, 128, L"Could not load ship mesh from X file %s\n", xFilePath.c_str());
		OutputDebugStringW(message);
		return E_FAIL;
	}

	// Compute model size for scale calculations
	D3DXVECTOR3 minCorner, maxCorner;
	Vertex3D* pVertices;
	m_pMesh->LockVertexBuffer(0, (void**)&pVertices);
	if( FAILED(D3DXComputeBoundingBox( (D3DXVECTOR3*)pVertices, m_pMesh->GetNumVertices(), m_pMesh->GetNumBytesPerVertex(), &minCorner, &maxCorner)) )
	{
		OutputDebugStringA("Cannot compute bounding box of the ship!\n");
		m_pMesh->UnlockVertexBuffer();
		return E_FAIL;
	}
	m_pMesh->UnlockVertexBuffer();

	// Initializes axes aligned bounding box and places its center at (0,0,0). Length is aligned with X axes. 
	// 0,1 points are left and right nose corners. 2,3 points are left and right aft corners
	float halfLength = m_shipLength/2.0f;
	float halfWidth = m_shipWidth/2.0f;
	m_boundingBox[0] = D3DXVECTOR3(maxCorner.x, 0, maxCorner.z); 
	m_boundingBox[1] = D3DXVECTOR3(maxCorner.x, 0, minCorner.z);
	m_boundingBox[2] = D3DXVECTOR3(minCorner.x, 0, maxCorner.z);
	m_boundingBox[3] = D3DXVECTOR3(minCorner.x, 0, minCorner.z);

	// Initialize model scale vector
	float modelLength = max(abs(maxCorner.x-minCorner.x), abs(maxCorner.z-minCorner.z));
	float modelWidth = min(abs(maxCorner.x-minCorner.x), abs(maxCorner.z-minCorner.z));
	float scale = (float)SHIP_LENGTH/modelLength;
	m_scaleVector = D3DXVECTOR3(scale, scale, scale);
	
	// Initialize ship's parameters
	m_shipLength = SHIP_LENGTH;
	m_shipWidth  = modelWidth*scale;
	m_shipMaxSpeed = 2.5f*sqrt(SHIP_LENGTH)*0.51444f;		// in m/s
	m_shipMass = 10000.0;				// for a 50 foot boat in kilograms, approximately
	m_shipMomentOfInertia = m_shipMass*((m_shipWidth*m_shipWidth)/4.0f + (m_shipLength*m_shipLength)/4.0f)/10.0f; // approximates ship's moment of inertia by half ellipsoid's moment of inertia
	m_rudderToGCenterDistance = (float)SHIP_LENGTH/2.0f;
	m_mastToGCenterDistance = (float)RUDDER_TO_MAST_DISTANCE_IN_MODEL*scale - m_rudderToGCenterDistance;
	m_mainSailHalfSize = (m_mastToGCenterDistance + m_rudderToGCenterDistance)/2.0f;

	// Assign "magic" constants to ship parameters - these are found experimentally in gaming
	m_sailsLiftEfficiencyConstant =  5000;	
	m_sailsPushEfficiencyConstant =  2000;	
	m_rudderEfficiencyConstant = 600;		
	m_hullLateralWaterResistance = 30500;	
	m_hullForwardWaterResistance = 3000;	

	// Initial ship position
	m_x = SHIP_LENGTH;
	m_z = SHIP_LENGTH;
	m_shipSpeed = 0;
	m_shipAngularSpeed = 0;
	m_shipToXAxesAngle = 0;
	m_sailsAngle = 0;
	m_rudderAngle = 0;
	m_jibOrientation = 1;
	m_mainsailOrientation = 1;

	return S_OK;
}

void Ship::Update(double fTime)
{
	float deltaTime = (float)(fTime - m_lastTime);
	m_lastTime = fTime;

	// Process controls
	bool teelerTouched = false;
	if(m_playable)
	{
		// Process player controls
		if(::IsControlKeyDown(SAILS_RIGHT))
		{
			m_sailsAngle += m_sailRotationSpeed * deltaTime;
		}
		
		if(::IsControlKeyDown(SAILS_LEFT))
		{
			m_sailsAngle -= m_sailRotationSpeed * deltaTime;
		}

		if(::IsControlKeyDown(TEELER_LEFT))
		{
			m_rudderAngle -= m_rudderRotationSpeed * deltaTime;
			teelerTouched = true;
		}

		if(::IsControlKeyDown(TEELER_RIGHT))
		{
			m_rudderAngle += m_rudderRotationSpeed * deltaTime;
			teelerTouched = true;
		}
	}
	else
	{
		// Process NPC control actions
		// Move the rudder to reach the set course
		float rudderCorrection = CalculateRudderCorrection(deltaTime);
		if(rudderCorrection!=0)
		{
			m_rudderAngle += rudderCorrection;
			teelerTouched = true;
		}

		// Trim sails for the current conditions course
		m_sailsAngle += CalculateOptimalSailTrimChange(deltaTime);
	}

	// Update sails orientation according to ships position, current trim and wind direction
	UpdateSailsOrientation(deltaTime);

	// Update rudder orientation
	UpdateRudderOrientation(deltaTime, teelerTouched);

	// Update the math model
	UpdateModel(deltaTime);

	// Call base class update to update children nodes
	Node::Update(fTime);

	// Update all matrices
	UpdateSailsTransforms();
	UpdateRudderTransform();
	UpdateFlagTransform();
	UpdateShipTransform();

	// Notify about events
	NotifySailor();
}

void Ship::UpdateModel(float deltaTime)
{
	//m_shipSpeed = 5.14f; // uncomment for testing, in m/s - moves the boat at constant speed on on angles

	float rudderForce = (-1.0f) * m_rudderEfficiencyConstant * m_shipSpeed * sin(m_rudderAngle); // multipled by -1 so force to the left is positive and to the right is negative, so angle to X axes changes correctly (increases or decreases)
	float angularAcceleration = (m_rudderToGCenterDistance*rudderForce - m_hullLateralWaterResistance*m_shipAngularSpeed) / m_shipMomentOfInertia; // a = torque / moment of inertia
	m_shipAngularSpeed += angularAcceleration * deltaTime;
	m_shipToXAxesAngle += m_shipAngularSpeed * deltaTime;
	m_shipToXAxesAngle = Utilities::NormalizeAngleInRadians(m_shipToXAxesAngle);

	float sailsLiftForce = 0;
	float windPushForce = 0;
	CalculateSailsForces(m_shipToXAxesAngle, m_sailsAngle, m_windSpeed, m_windToXAxesAngle, sailsLiftForce, windPushForce);
	float acceleration = (sailsLiftForce + windPushForce - m_hullForwardWaterResistance*m_shipSpeed) / m_shipMass;
	m_shipSpeed += acceleration * deltaTime;

	if(m_shipSpeed>m_shipMaxSpeed)
	{
		// Ship speed cannot exceed theoretical limit for its hull size (see comment at m_shipMaxSpeed declaration)
		m_shipSpeed = m_shipMaxSpeed;
	}

	// Calculate new coordinates
	if(m_collisionStatus==Ship::NoCollision)
	{
		m_x += m_shipSpeed * cos(m_shipToXAxesAngle) * deltaTime;
		m_z += m_shipSpeed * sin(m_shipToXAxesAngle) * deltaTime;
	}

    // Save these for testing
    m_sailsLiftForce = sailsLiftForce;
    m_sailsPushForce = windPushForce;
}

// Calculates sails lift and push forces depending on current sail trim, wind and ship position
void Ship::CalculateSailsForces(float shipToXAxesAngle, float sailsAngle, float windSpeed, float windToXAxesAngle, float& sailsLiftForce, float& windPushForce)
{
	float sailsWorldAngle = Utilities::NormalizeAngleInRadians(shipToXAxesAngle + D3DX_PI + sailsAngle);
	float windWorldAngle = Utilities::NormalizeAngleInRadians(windToXAxesAngle);
	float sailsToWindAngle = abs(sailsWorldAngle - windWorldAngle); 
	// TODO: need to come up with a better schema than angle difference and then "if", since on angles more than 90degrees it will not work (we need smallest angle between two for this)

	sailsLiftForce = 0;
	if (sailsToWindAngle>=NO_SAIL_ANGLE_TO_WIND && sailsToWindAngle<D3DX_PI/2.0f)
	{
		sailsLiftForce = windSpeed * m_sailsLiftEfficiencyConstant * cos(sailsToWindAngle);
	}
	
	windPushForce = windSpeed * m_sailsPushEfficiencyConstant * abs(sin(sailsToWindAngle)) * cos(windWorldAngle - shipToXAxesAngle);
}

// Updates max allowed sails angles and sails orientation
void Ship::UpdateSailsOrientation(float deltaTime)
{
	float windAngleRelativeToShip = Utilities::NormalizeAngleInRadians(m_windToXAxesAngle - m_shipToXAxesAngle) - D3DX_PI;

	D3DXVECTOR3 windDir(cos(m_windToXAxesAngle), 0, sin(m_windToXAxesAngle));
	D3DXVECTOR3 shipDir(cos(m_shipToXAxesAngle), 0, sin(m_shipToXAxesAngle));
	D3DXVECTOR3 right;
	D3DXVec3Cross(&right, &D3DXVECTOR3(0,1,0), &shipDir);
	float jibOrientation = D3DXVec3Dot(&windDir, &right)>=0 ? 1.0f : -1.0f;
	
	float mainsailAbsAngle = m_shipToXAxesAngle + D3DX_PI + m_sailsAngle;
	D3DXVECTOR3 boomDir(cos(mainsailAbsAngle), 0, sin(mainsailAbsAngle));
	D3DXVec3Cross(&right, &D3DXVECTOR3(0,1,0), &boomDir);
	float mainsailOrientation = D3DXVec3Dot(&windDir, &right)<=0 ? 1.0f : -1.0f;

	// Flip sail normals and scale coefficients so they render correctly when their orientation changes
	if(m_jibOrientation!=jibOrientation)
	{
		m_jibOrientation = jibOrientation;
		Utilities::FlipNormals(m_pMesh, Jib);
	}

	if(m_mainsailOrientation!=mainsailOrientation)
	{
		m_mainsailOrientation = mainsailOrientation;
		Utilities::FlipNormals(m_pMesh, Mainsail);
	}

	// Update valid sails angle range	
	if( -D3DX_PI-D3DX_PI/12.0f <= windAngleRelativeToShip && windAngleRelativeToShip <= -D3DX_PI+D3DX_PI/12.0f )
	{
		// If the wind blows straight from behind (+-15 degrees or in 30 degrees arc) then sails can move either way
		m_leftAllowedSailsAngle = -MAINSAIL_MAX_ANGLE;
		m_rightAllowedSailsAngle = MAINSAIL_MAX_ANGLE;
	}
	else if(jibOrientation == -1)
	{
		// Wind blows from the right
		m_leftAllowedSailsAngle = max(-MAINSAIL_MAX_ANGLE, windAngleRelativeToShip);
		m_rightAllowedSailsAngle = 0;
	}
	else
	{
		// Wind blows from the left
		m_leftAllowedSailsAngle = 0;
		m_rightAllowedSailsAngle = min(MAINSAIL_MAX_ANGLE, windAngleRelativeToShip);
	}

	// Adjust sail angle if it is out of a valid range
	if(m_sailsAngle > m_rightAllowedSailsAngle)
	{
		m_sailsAngle = m_rightAllowedSailsAngle;
	}

	if(m_sailsAngle < m_leftAllowedSailsAngle)
	{
		m_sailsAngle = m_leftAllowedSailsAngle;
	}
}

void Ship::UpdateRudderOrientation(float deltaTime, bool wasTeelerTouched)
{
	if(m_rudderAngle < -RUDDER_MAX_ANGLE)
	{
		m_rudderAngle = -RUDDER_MAX_ANGLE;
	}

	if(m_rudderAngle > RUDDER_MAX_ANGLE)
	{
		m_rudderAngle = RUDDER_MAX_ANGLE;
	}

	// Move the teeler back to neutral position if it was not touched
	if(!wasTeelerTouched && m_rudderAngle!=0)
	{
		float absoluteRudderAngle = abs(m_rudderAngle);
		float rotationStep = 2*m_rudderRotationSpeed*deltaTime;
		if( absoluteRudderAngle < rotationStep )
		{
			m_rudderAngle = 0;
		}
		else
		{
			if(m_rudderAngle>0)
			{
				m_rudderAngle -= rotationStep;
			}
			else
			{
				m_rudderAngle += rotationStep;
			}
		}
	}
}

// Calculates sails angle change that moves sails towards their best efficiency. deltaTime - current time step
float Ship::CalculateOptimalSailTrimChange(float deltaTime)
{
	float sailAngleChangeStep = m_sailRotationSpeed * deltaTime;

	// First component is sail trim angle, second component what step to use to get to that trim
	float possibleTrims[6][2] = 
	{ 
		{m_sailsAngle, 0}, 
		{m_sailsAngle + sailAngleChangeStep, sailAngleChangeStep}, 
		{m_sailsAngle - sailAngleChangeStep, -sailAngleChangeStep}, 
		{0, m_sailsAngle>0 ? -sailAngleChangeStep : sailAngleChangeStep },
		{m_rightAllowedSailsAngle, sailAngleChangeStep},
		{m_leftAllowedSailsAngle, -sailAngleChangeStep}
	};

	int maxForceTrim = -1;
	float maxSailsForce = -FLT_MAX;
	int minForceTrim = -1;
	float minSailsForce = FLT_MAX;
	for(int i=0; i<6; i++)
	{
		float sailsLiftForce = 0;
		float windPushForce = 0;
		CalculateSailsForces(m_shipToXAxesAngle, possibleTrims[i][0], m_windSpeed, m_windToXAxesAngle, sailsLiftForce, windPushForce);
		float sailsForce = sailsLiftForce + windPushForce;
		if(sailsForce>maxSailsForce)
		{
			maxSailsForce = sailsForce;
			maxForceTrim = i;
		}

		if(sailsForce < minSailsForce)
		{
			minSailsForce = sailsForce;
			minForceTrim = i;
		}
	}
	
	if(maxForceTrim == -1 && minSailsForce == -1)
	{
		OutputDebugStringA("Could not find optimal trim!/n");
		_ASSERT(false);
		return 0;
	}

	// Check for "in irons" condition and try backwinding from it if we are "in irons"
	if(IsInIrons())
	{
		OutputDebugStringA("In Irons! Trying to backwind\n");
		return possibleTrims[minForceTrim][1];
	}

	// Normal case
	return possibleTrims[maxForceTrim][1];
}

// Calculates how much we need to move the rudder to get to the target course
float Ship::CalculateRudderCorrection(float deltaTime)
{
	float rudderCorrection = 0;
	m_targetCourseAngleToXAxes = Utilities::NormalizeAngleInRadians(m_targetCourseAngleToXAxes);

	// Predict what happens if we don't apply any rudder force. Will the ship move to the correct position?
	float predictedAngularAcceleration = 0;
	float predictedShipAngularSpeed = m_shipAngularSpeed;
	float predictedShipToXAxesAngle = m_shipToXAxesAngle;
	while( predictedShipAngularSpeed>ANGULAR_SPEED_EPSILON || predictedShipAngularSpeed<-ANGULAR_SPEED_EPSILON )
	{
		predictedAngularAcceleration = (-m_hullLateralWaterResistance*predictedShipAngularSpeed) / m_shipMomentOfInertia; // a = drag force / moment of inertia
		predictedShipAngularSpeed += predictedAngularAcceleration * deltaTime;
		predictedShipToXAxesAngle += predictedShipAngularSpeed * deltaTime;
	}
	predictedShipToXAxesAngle = Utilities::NormalizeAngleInRadians(predictedShipToXAxesAngle);

	if( abs(m_targetCourseAngleToXAxes-predictedShipToXAxesAngle)<TARGET_COURSE_EPSILON)
	{
		return 0; // no correction is needed
	}

	float shipToXAxesAngleForCorrecting = predictedShipToXAxesAngle;

	// Correction is needed, calculate it
	float correction = Utilities::NormalizeAngleInRadians(m_targetCourseAngleToXAxes - shipToXAxesAngleForCorrecting);
	if(correction > D3DX_PI)
	{
		correction -= 2.0f*D3DX_PI;
	}

	float absoluteCorrection = abs(correction);
	if( absoluteCorrection > TARGET_COURSE_EPSILON ) // should not touch the teeler if we are almost on the target course to avoid overregulation
	{
		if( correction > 0 )
		{
			// Turn left to reach target course (rudder angle should move to negative side). 
			rudderCorrection = -m_rudderRotationSpeed * deltaTime;
		}
		else
		{
			// Turn right to reach target course (rudder angle should be positive)
			rudderCorrection = m_rudderRotationSpeed * deltaTime;
		}
	}

	return rudderCorrection;
}

void Ship::UpdateShipTransform()
{
	D3DXVECTOR3 wavePosition;
	g_ocean->ComputeWaveAt(m_x, m_z, m_lastTime, &wavePosition, &m_waveNormal, NULL);

	D3DXVECTOR3 newShipNormal = m_waveNormal + D3DXVECTOR3(0,1,0);

	// It turns out that our model is not aligned with X axes and instead aligned with Z and headed to -Z, so we need to turn it
	float drawAngle = m_shipToXAxesAngle+D3DX_PI/2.0f;
	D3DXVECTOR3 direction(cos(drawAngle), 0, sin(drawAngle));
	D3DXMATRIX shipTransformMatrix;
	Utilities::CreateWorldMatrix(&shipTransformMatrix, newShipNormal, direction, D3DXVECTOR3(m_x, wavePosition.y - 0.4f, m_z));
	
	D3DXMATRIX shipScalingMatrix;
	D3DXMatrixScaling(&shipScalingMatrix, m_scaleVector.x, m_scaleVector.y, m_scaleVector.z);

	m_shipMatrix = shipScalingMatrix * shipTransformMatrix;
}

void Ship::UpdateSailsTransforms()
{
	D3DXMATRIX jibReflection;
	D3DXMatrixScaling(&jibReflection, 1, 1, m_jibOrientation);
	m_jibMatrix = jibReflection;
	
	D3DXMATRIX mainsailReflection;
	D3DXMatrixScaling(&mainsailReflection, 1, 1, m_mainsailOrientation);
	
	D3DXMatrixRotationY(&m_boomMatrix, -m_sailsAngle);
	m_mainsailMatrix = mainsailReflection * m_boomMatrix;
}

void Ship::UpdateRudderTransform()
{
	D3DXMATRIX rudderMatrixRotAroundZero;
	D3DXMatrixRotationY(&rudderMatrixRotAroundZero, -m_rudderAngle);
    D3DXMATRIX rudderTranslationToZero;
    D3DXMatrixTranslation(&rudderTranslationToZero, RUDDER_TO_MAST_DISTANCE_IN_MODEL, 0, 0);
    D3DXMATRIX rudderTranslationFromZero;
    D3DXMatrixTranslation(&rudderTranslationFromZero, -RUDDER_TO_MAST_DISTANCE_IN_MODEL, 0, 0);
    m_rudderMatrix = rudderTranslationToZero * rudderMatrixRotAroundZero * rudderTranslationFromZero;
}

void Ship::UpdateFlagTransform()
{
	float randomAngle = (float(rand())/float(RAND_MAX))*(D3DX_PI/18.0f)-D3DX_PI/36.0f; // to model random flag movements

	D3DXMATRIX flagTranslationToZero;
    D3DXMatrixTranslation(&flagTranslationToZero, -FLAG_TO_MAST_DISTANCE_IN_MODEL, 0.0f, 0.0f);
	D3DXMATRIX flagMatrixRotAroundZero;
	D3DXMatrixRotationY(&flagMatrixRotAroundZero, m_windToXAxesAngle+m_shipToXAxesAngle+randomAngle);
	D3DXMATRIX flagTranslationFromZero;
    D3DXMatrixTranslation(&flagTranslationFromZero, FLAG_TO_MAST_DISTANCE_IN_MODEL, 0.0f, 0.0f);
	m_flagMatrix = flagTranslationToZero * flagMatrixRotAroundZero * flagTranslationFromZero;
}

void Ship::Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	D3DXMATRIX mainsailWorldMatrix = m_mainsailMatrix * m_shipMatrix;
	D3DXMATRIX jibWorldMatrix = m_jibMatrix * m_shipMatrix;
	D3DXMATRIX boomWorldMatrix = m_boomMatrix * m_shipMatrix;
	D3DXMATRIX rudderWorldMatrix = m_rudderMatrix * m_shipMatrix;
	D3DXMATRIX flagWorldMatrix = m_flagMatrix * m_shipMatrix;
    D3DXMATRIX* pWorld;
	
	for (size_t i = 0; i < m_numberOfMaterials; i++)
	{
		// Set the material and texture for this subset
		if(m_pMeshTextures[i]!=NULL)
		{
			pd3dDevice->SetTexture(0, m_pMeshTextures[i]);
		}
		else
		{
			// Our model is not very well defined, so in case when we don't have texture just use this one (white)
			pd3dDevice->SetTexture(0, m_pMeshTextures[Mainsail]);
		}

		if(i==Mainsail)
		{
            pWorld = &mainsailWorldMatrix;
		}
		else if(i==Jib)
		{
            pWorld = &jibWorldMatrix;
		}
		else if(i==Boom || i==Mainsheets)
		{
            pWorld = &boomWorldMatrix;
		}
		else if(i==Rudder)
		{
            pWorld = &rudderWorldMatrix;
		}
		else if(i==Flag)
		{
            pWorld = &flagWorldMatrix;
		}
		else
		{
            pWorld = &m_shipMatrix;
		}

        // Setup transformation matrices
	    Utilities::SetVertexShaderTransforms(pd3dDevice, *pWorld, viewProj);

		// Draw the mesh subset
		m_pMesh->DrawSubset(i);
	}

	Node::Render(pd3dDevice, viewProj);

	if(g_isInDebugging)
	{
		RenderDebugInformation(pd3dDevice, viewProj);
	}
}

void Ship::RenderDebugInformation(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	pd3dDevice->SetPixelShader(g_pPixelShader);

	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);
	Utilities::SetVertexShaderTransforms(pd3dDevice, world, viewProj);

	D3DXVECTOR3 position = GetPosition();

	if(!m_playable)
	{
		position.y = m_shipWidth;
		Utilities::DrawLineSegment(pd3dDevice, position, m_endOfLineOfSight, D3DCOLOR_ARGB(255, 255, 0, 0));
	}

	D3DXVECTOR3 shipBoundingBox[4];
	GetBoundingBox(shipBoundingBox);
	Utilities::DrawLineSegment(pd3dDevice, shipBoundingBox[0], shipBoundingBox[1], D3DCOLOR_ARGB(255, 255, 0, 0));
	Utilities::DrawLineSegment(pd3dDevice, shipBoundingBox[1], shipBoundingBox[3], D3DCOLOR_ARGB(255, 255, 0, 0));
	Utilities::DrawLineSegment(pd3dDevice, shipBoundingBox[0], shipBoundingBox[2], D3DCOLOR_ARGB(255, 255, 0, 0));
	Utilities::DrawLineSegment(pd3dDevice, shipBoundingBox[2], shipBoundingBox[3], D3DCOLOR_ARGB(255, 255, 0, 0));

	Utilities::DrawLineSegment(pd3dDevice, position, position + m_waveNormal*10, D3DCOLOR_ARGB(255, 255, 0, 0));

	pd3dDevice->SetPixelShader(g_pPixelShaderTexture);
}

void Ship::NotifySailor()
{
	if(!m_playable) // notify AI for NPCs only
	{
		if(m_collisionStatus!=Ship::NoCollision)
		{
			g_fsmManager->SendEvent(m_sailorId, Sailor::CollisionEvent);
		}
		else if(IsInIrons())
		{
			g_fsmManager->SendEvent(m_sailorId, Sailor::StuckAgainstWindEvent);
		}
		else
		{
			g_fsmManager->SendEvent(m_sailorId, Sailor::AllIsGoodEvent);
		}
	}
}

bool Ship::IsInIrons()
{
	float nosailDirectionToXAxesAngle = Utilities::NormalizeAngleInRadians(m_windToXAxesAngle + D3DX_PI);
	if( m_shipSpeed<LINEAR_SPEED_EPSILON && abs(nosailDirectionToXAxesAngle - m_shipToXAxesAngle)<NO_SAIL_ANGLE_TO_WIND )
	{
		return true;
	}

	return false;
}

/**
	Returns an array of 4 D3DXVECTOR3 vectors representing ship's bounding box aligned with ship's position and course.
	Currently, the bounding box is a 2D rectangle located on the sea level. 
	The center of the box is aligned with the center of the ship. Array elements 0,1 are left and right nose corners (oriented as ship's bow) 
	and elements 2,3 are left and right aft corners
	
	The caller must allocated array memory!
*/
void Ship::GetBoundingBox(D3DXVECTOR3* pBoundingBox)
{
	D3DXVec3TransformCoordArray(pBoundingBox, sizeof(D3DXVECTOR3), m_boundingBox, sizeof(D3DXVECTOR3), &m_shipMatrix, 4);
}

/**
	Checks if the point is visually visible from where the ship is located at this point.
	Returns: true if visible, false if not
			 endOfLineOfSight is equal to searched point if it is visible OR is equal to land point if land blocks line of sight
*/
bool Ship::IsPointVisible(const D3DXVECTOR3& point, D3DXVECTOR3& endOfLineOfSight) 
{
	D3DXVECTOR3 origin = D3DXVECTOR3( m_x, m_shipWidth, m_z );
	D3DXVECTOR3 direction = point - origin;
	float distance = D3DXVec3Length(&direction);
	float scaleStep = m_shipLength / distance; // smaller step makes it more precise and slower, larger step has more error, but it is faster
	for(float t = scaleStep; t < 1; t+=scaleStep)
	{
		D3DXVECTOR3 linePoint = origin + t * direction;
	
		if( g_worldEnvironment->IsLand(linePoint) )
		{
			endOfLineOfSight = linePoint;
			return false;
		}
	}

	endOfLineOfSight = point;
	return true;
}
