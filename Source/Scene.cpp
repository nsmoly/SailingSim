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
#include "DXUT\DXUTcamera.h"

#include <list>

#include "FSM.h"
#include "Scene.h"
#include "Land.h"
#include "Ocean.h"
#include "Ship.h"
#include "Creature.h"
#include "Sailor.h"

using namespace std;

extern std::tr1::shared_ptr<FSMManager> g_fsmManager;
extern std::tr1::shared_ptr<Land> g_worldEnvironment;
extern std::tr1::shared_ptr<Ocean> g_ocean;
extern std::tr1::shared_ptr<Ship> g_playerShip;
extern std::list<D3DXVECTOR3>	  g_racePoints;

extern IDirect3DVertexShader9* g_pVertexShader;
extern IDirect3DVertexShader9* g_pWaveVertexShader;
extern IDirect3DPixelShader9*  g_pPixelShaderTexture;
extern IDirect3DPixelShader9*  g_pPixelShader;
extern IDirect3DPixelShader9*  g_pWavePixelShader;

Scene::Scene(IDirect3DDevice9* pd3dDevice) : 
	Node(pd3dDevice), 
    m_lastTime(0)
{
}

Scene::~Scene()
{
}

HRESULT Scene::Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath)
{
	if(!szDataPath)
	{
		OutputDebugStringW(L"Invalid argument - path to level data is not specified!");
		return E_INVALIDARG;
	}

	// Create and Add children nodes - ocean, land, ships, etc.
	
	// Create land
	std::tr1::shared_ptr<Land> land(new Land(pd3dDevice));
	if( !land || FAILED(land->Load(pd3dDevice, szDataPath)) )
	{
		OutputDebugStringW(L"Failed creating or loading the land");
		return E_FAIL;
	}
	Node::AddChild(land);
	g_worldEnvironment = land;

	// Create player ship
	wstring playerShipModelPath(szDataPath);
	playerShipModelPath += L"\\ship";
	g_playerShip = std::tr1::shared_ptr<Ship>(new Ship(pd3dDevice, true));
	if( !g_playerShip || FAILED(g_playerShip->Load(pd3dDevice, playerShipModelPath.c_str())) )
	{
		OutputDebugStringW(L"Failed creating or loading the main character ship");
		return E_FAIL;
	}
	g_playerShip->SetPosition(D3DXVECTOR3(50, 0, 50));
	g_playerShip->SetCourse(0);
	Node::AddChild(g_playerShip);

	// Create NPCs
	wstring npcShipModelPath(szDataPath);
	npcShipModelPath += L"\\npcship";
	std::tr1::shared_ptr<Ship> npcShip(new Ship(pd3dDevice, false));
	if( !npcShip || FAILED(npcShip->Load(pd3dDevice, npcShipModelPath.c_str())) )
	{
		OutputDebugStringW(L"Failed creating or loading the NPC ship 1");
		return E_FAIL;
	}
	npcShip->SetPosition(D3DXVECTOR3(50, 0, 65));
	npcShip->SetCourse(0);
	Node::AddChild(npcShip);
	g_fsmManager->RegisterFSM(1, std::tr1::shared_ptr<IFSM>(new Sailor(npcShip, g_racePoints)));
	npcShip->SetSailorId(1);

	npcShip = std::tr1::shared_ptr<Ship>(new Ship(pd3dDevice, false));
	if( !npcShip || FAILED(npcShip->Load(pd3dDevice, npcShipModelPath.c_str())) )
	{
		OutputDebugStringW(L"Failed creating or loading the NPC ship 2");
		return E_FAIL;
	}
	npcShip->SetPosition(D3DXVECTOR3(50, 0, 35));
	npcShip->SetCourse(0);
	Node::AddChild(npcShip);
	g_fsmManager->RegisterFSM(2, std::tr1::shared_ptr<IFSM>(new Sailor(npcShip, g_racePoints)));
	npcShip->SetSailorId(2);

	// Create shark
	std::tr1::shared_ptr<Creature> creature(new Creature(pd3dDevice));
	if( !creature || FAILED(creature->Load(pd3dDevice, szDataPath)) )
	{
		OutputDebugStringW(L"Failed creating or loading the creature");
		return E_FAIL;
	}
	Node::AddChild(creature);

	// Create ocean
	std::tr1::shared_ptr<Ocean> ocean(new Ocean(pd3dDevice));
	if( !ocean || FAILED(ocean->Load(pd3dDevice, szDataPath)) )
	{
		OutputDebugStringW(L"Failed creating or loading the ocean");
		return E_FAIL;
	}
	Node::AddChild(ocean);
	g_ocean = ocean;

    return S_OK;
}

void Scene::Update(double fTime)
{
    m_lastTime = fTime;

	// Call base class since this node is a static landscape and does not change (yet)
	Node::Update(fTime);
}

void Scene::Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	// Call base class
	Node::Render(pd3dDevice, viewProj);
}
