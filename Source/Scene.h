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

#include "Node.h"
#include "Utilities.h"

#define WORLD_SIZE_X	1000.0f	// world size in meters
#define WORLD_SIZE_Z	1000.0f	// world size in meters

class Scene : public Node
{
public:
	Scene(IDirect3DDevice9* pd3dDevice);
	virtual ~Scene();

	virtual HRESULT	Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath);
	
	virtual void Update(double fTime);
	virtual void Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);

private:
    double m_lastTime;
};

