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

#define SEA_NCELL_X		20
#define SEA_NCELL_Z		20
#define SEA_CELL_STEPX	2		// cell size in meters
#define SEA_CELL_STEPZ	2		// cell size in meters

#define SEA_TEX_NCELL_X	10
#define SEA_TEX_NCELL_Z	10

#define MAX_WAVEDIR_VARIATION	0.4f

class Ocean : public Node
{
public:
	Ocean(IDirect3DDevice9* pd3dDevice);
	virtual ~Ocean();

	virtual HRESULT	Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath);
	
	virtual void Update(double fTime);
	virtual void Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);

	virtual bool ComputeWaveAt(float x, float z, double time, D3DXVECTOR3* pPosition, D3DXVECTOR3* pNormal, D3DXVECTOR3* pTangent);

private:
	HRESULT CreateSea(Vertex3D** ppVertices, DWORD* pNumOfVertices, DWORD** ppMesh, DWORD* pNumOfTriangles);
	Wave GenerateRandomWave(const Wave& refWave);

    double m_lastTime;
	double m_nextWaveUpdateTime;
	size_t m_nextWaveToUpdate;

	ID3DXMesh* m_pSeaMesh;
	LPDIRECT3DTEXTURE9 m_pSeaTexture;
    LPDIRECT3DTEXTURE9 m_pSeaNormalMap;

	Wave m_waves[4];
	float m_maxWaveCoordVariation;

	// World dimensions
	float m_cellSizeX;
	float m_cellSizeZ;
	float m_worldSizeX;
	float m_worldSizeZ;
};

