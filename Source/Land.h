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
#include "WorldFile.h"

#define LAND_NCELL_X	25
#define LAND_NCELL_Z	25

class Land : public Node
{
public:
	Land(IDirect3DDevice9* pd3dDevice);
	virtual ~Land();

	virtual HRESULT	Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath);
	
	virtual void Update(double fTime);
	virtual void Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);

	bool IsLand(const D3DXVECTOR3& point);
	bool IsOutsideBorders(const D3DXVECTOR3& point);
	WorldFile& GetLevelDescription();

private:
	HRESULT CreateLand(const WorldFile& levelData, Vertex3D** ppVertices, DWORD* pNumOfVertices, DWORD** ppMesh, DWORD* pNumOfTriangles);
    size_t CountTakenCells(const WorldFile& levelData, int cornerXIndex, int cornerZIndex, size_t sizeX, size_t sizeZ);

    double m_lastTime;
	WorldFile m_levelData;
	ID3DXMesh* m_pLandMesh;		// Land mesh
	LPDIRECT3DTEXTURE9 m_pLandTexture;

	Vertex3D* m_pLandVertices;
	DWORD m_numberOfLandVertices;
	DWORD* m_pLandFaces;
	DWORD m_numberOfLandFaces;
	
	// World dimensions
	size_t m_numOfCellsX;
	size_t m_numOfCellsZ;
	float m_cellSizeX;
	float m_cellSizeZ;
	float m_worldSizeX;
	float m_worldSizeZ;
};

