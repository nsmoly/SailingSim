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

#include "Node.h"
#include "WorldFile.h"

struct MAPVERTEX
{
    FLOAT x, y, z;
    DWORD color;
    FLOAT tu1, tv1;
    FLOAT tu2, tv2;
};

#define D3DFVF_MAPVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)

class MiniMap : public Node
{
public:
	MiniMap(IDirect3DDevice9* pd3dDevice, WorldFile& worldMap);
	virtual ~MiniMap();

	virtual void Update(double fTime);
	virtual void Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);

private:
	void PositionMap();
	void MarkShipsOnMap();

	float m_mapWidth;
	float m_mapHeight;

	LPDIRECT3DVERTEXBUFFER9 m_pVertices;
	IDirect3DTexture9* m_pMapTexture;
	IDirect3DTexture9* m_pMaskTexture;
	UINT m_textureWidth;
	UINT m_textureHeight;
	std::list<UINT> m_shipPositions;
	IDirect3DStateBlock9* m_pStateBlock;
};

