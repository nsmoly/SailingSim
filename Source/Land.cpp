
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

#include "Scene.h"
#include "Land.h"
#include "Ship.h"

using namespace std;

extern std::tr1::shared_ptr<Ship> g_playerShip;
extern CFirstPersonCamera g_Camera;

extern IDirect3DVertexShader9* g_pVertexShader;
extern IDirect3DPixelShader9*  g_pPixelShaderTexture;

Land::Land(IDirect3DDevice9* pd3dDevice) : 
	Node(pd3dDevice), 
    m_lastTime(0),
	m_pLandMesh(0),
	m_pLandTexture(0),
	m_pLandVertices(0),
	m_pLandFaces(0),
	m_numOfCellsX(0),
	m_numOfCellsZ(0),
	m_cellSizeX(0),
	m_cellSizeZ(0),
	m_worldSizeX(0),
	m_worldSizeZ(0)
{
}

Land::~Land()
{
	SAFE_DELETE_ARRAY(m_pLandVertices);
	SAFE_DELETE_ARRAY(m_pLandFaces);

	SAFE_RELEASE(m_pLandMesh);
	SAFE_RELEASE(m_pLandTexture);
}

HRESULT Land::Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath)
{
	if(!szDataPath)
	{
		OutputDebugStringW(L"Invalid argument - path to data directory is not specified!");
		return E_INVALIDARG;
	}

	// Load textures
	wstring landTextureFilePath(szDataPath);
	landTextureFilePath += L"\\World\\land.jpg";
	SAFE_RELEASE(m_pLandTexture);
	if( FAILED(D3DXCreateTextureFromFile(pd3dDevice, landTextureFilePath.c_str(), &m_pLandTexture)) )
	{
		OutputDebugStringW(L"Failed to load land texture");
		return E_FAIL;
	}

	// Load level data file
	wstring levelFilePath(szDataPath);
	levelFilePath += L"\\World\\level.grd";
	if( !m_levelData.Load(levelFilePath.c_str()) )
	{
		wchar_t message[128];
		swprintf_s(message, 128, L"Could not load level data from the level file %s\n", levelFilePath);
		OutputDebugString(message);
		return E_FAIL;
	}

	// Calculate & remember world dimensions
	m_worldSizeX = WORLD_SIZE_X;
	m_worldSizeZ = WORLD_SIZE_Z;
	m_numOfCellsX = LAND_NCELL_X;
	m_numOfCellsZ = LAND_NCELL_Z;
	m_cellSizeX = m_worldSizeX / m_numOfCellsX;
	m_cellSizeZ = m_worldSizeZ / m_numOfCellsZ;

	Vertex3D* pVertices;
	DWORD numberOfVertices;
	DWORD* pMeshIndexes;
	DWORD numberOfTriangles;

	// Create land mesh
	if( FAILED(CreateLand(m_levelData, &pVertices, &numberOfVertices, &pMeshIndexes, &numberOfTriangles)) )
	{
		OutputDebugStringW(L"Failed to create data arrays for the land mesh");
		return E_FAIL;
	}

	SAFE_RELEASE(m_pLandMesh);
	if( FAILED(Utilities::CreateMesh(pd3dDevice, pVertices, numberOfVertices, pMeshIndexes, numberOfTriangles, &m_pLandMesh)) )
	{
		OutputDebugStringW(L"Failed to create D3DX land mesh");
		return E_FAIL;
	}

	// Save land arrays for later use in collision detection
	SAFE_DELETE_ARRAY(m_pLandVertices);
	SAFE_DELETE_ARRAY(m_pLandFaces);
	m_pLandVertices = pVertices;
	m_numberOfLandVertices = numberOfVertices;
	m_pLandFaces = pMeshIndexes;
	m_numberOfLandFaces = numberOfTriangles;

    return S_OK;
}

HRESULT Land::CreateLand(const WorldFile& levelData, Vertex3D** ppVertices, DWORD* pNumOfVertices, DWORD** ppMesh, DWORD* pNumOfTriangles)
{
	// Common color properties for all vertices
	D3DXCOLOR diffuseColor(0.5f, 0.5f, 0.5f, 1.0f);
	D3DXCOLOR specularColor(0.5f, 0.5f, 0.5f, 1.0f);

	*ppVertices = NULL;
	*pNumOfVertices = 0;
	*ppMesh = NULL;
	*pNumOfTriangles = 0;

	// Allocate vertex array
	*pNumOfVertices = m_numOfCellsX*m_numOfCellsZ*4;
	*ppVertices = new Vertex3D[*pNumOfVertices];
	memset(*ppVertices, 0, (*pNumOfVertices)*sizeof(Vertex3D));

  	// Allocate index array
	*pNumOfTriangles = m_numOfCellsX*m_numOfCellsZ*2;
	*ppMesh = new DWORD[(*pNumOfTriangles)*3];
	memset(*ppMesh, 0, (*pNumOfTriangles)*sizeof(DWORD)*3);

	// Go over all mesh nodes and create corresponding vertices and faces with correct parameters
	size_t vertexIndex = 0;
    size_t indexIndex = 0;
	float cellXCoord = 0; 
	float cellZCoord = 0;
	float maxLandHeight = m_cellSizeX;

	D3DXVECTOR3 vert1;
	D3DXVECTOR3 vert2;
	D3DXVECTOR3 vert3;
	D3DXVECTOR3 normal;

	D3DXVECTOR3* pNormals = new D3DXVECTOR3[(m_numOfCellsZ+1)*(m_numOfCellsX+1)];
	memset(pNormals, 0, sizeof(D3DXVECTOR3)*(m_numOfCellsZ+1)*(m_numOfCellsX+1));
	size_t normStride = m_numOfCellsZ+1;

	for(size_t zIndex = 0; zIndex<m_numOfCellsZ; zIndex++)
	{
		for(size_t xIndex = 0; xIndex<m_numOfCellsX; xIndex++)
		{
            bool onTheWorldBorder = false;
            if(xIndex==0 || xIndex==m_numOfCellsX-1 || zIndex==0 || zIndex==m_numOfCellsZ-1)
            {
                onTheWorldBorder = true;
            }

            // Create 4 vertices for all 4 corners of the current cell, calculate on which level vertices should be: higher ground, sea level, deep sea
            // Vertices are added in the following order: topleft, topright, bottomleft, bottomright
            // Texture coordinates are assigned as (0,0), (1,0), (0,1), (1,1) to these corners
            for(int k=0; k<=1; k++)
            {
                for(int l=0; l<=1; l++)
                {
			        size_t takenCellCount = CountTakenCells(levelData, xIndex+k-1, zIndex+l-1, 2, 2);
                    float landHeight = 0; // sea level by default

			        if(takenCellCount == 0)
			        {
				        // No occupied cells in the neighbourhood of this vertice - this is the bottom of the sea
                        landHeight = -maxLandHeight;
			        }
			        else if(takenCellCount>2)
			        {
				        // More then 2 occupied cells in the neighbourhood of this vertice - this is the higher island ground
                        landHeight = maxLandHeight;
			        }
			        else
			        {
				        // Everything else is the beach - sea level
                        landHeight = 0;
			        }

                    (*ppVertices)[vertexIndex++] = Vertex3D(
						D3DXVECTOR3(cellXCoord + m_cellSizeX*(float)k, landHeight, cellZCoord + m_cellSizeZ*(float)l),
						D3DXVECTOR2((float)k, (float)l),
						D3DXVECTOR3(0,0,0),
						diffuseColor,
						specularColor
					);
                }
            }

            // Create 2 faces corresponding to created vertices. Faces are created by connecting vertices in CCW fashion
            (*ppMesh)[indexIndex++] = vertexIndex-4; // nearleft vertice (if looking from x=0 and z=0)
            (*ppMesh)[indexIndex++] = vertexIndex-2; // nearright
            (*ppMesh)[indexIndex++] = vertexIndex-3; // farleft
            (*ppMesh)[indexIndex++] = vertexIndex-3; // farleft
            (*ppMesh)[indexIndex++] = vertexIndex-2; // nearright
            (*ppMesh)[indexIndex++] = vertexIndex-1; // farright
            			
			// Accumulate normals for upper left triangle
			vert1 = (*ppVertices)[vertexIndex-4].position;
			vert2 = (*ppVertices)[vertexIndex-2].position;
			vert3 = (*ppVertices)[vertexIndex-3].position;

			D3DXVec3Cross(&normal, &(vert2-vert1), &(vert1-vert3));
			pNormals[zIndex*normStride + xIndex] += normal;

			D3DXVec3Cross(&normal, &(vert3-vert2), &(vert2-vert1));
			pNormals[(zIndex+1)*normStride + xIndex] += normal;

			D3DXVec3Cross(&normal, &(vert1-vert3), &(vert3-vert2));
			pNormals[zIndex*normStride + xIndex+1] += normal;

			// Accumulate normals for lower right triangle
			vert1 = (*ppVertices)[vertexIndex-3].position;
			vert2 = (*ppVertices)[vertexIndex-2].position;
			vert3 = (*ppVertices)[vertexIndex-1].position;

			D3DXVec3Cross(&normal, &(vert2-vert1), &(vert1-vert3));
			pNormals[zIndex*normStride + xIndex+1] += normal;

			D3DXVec3Cross(&normal, &(vert3-vert2), &(vert2-vert1));
			pNormals[(zIndex+1)*normStride + xIndex] += normal;

			D3DXVec3Cross(&normal, &(vert1-vert3), &(vert3-vert2));
			pNormals[(zIndex+1)*normStride + (xIndex+1)] += normal;

			cellXCoord += m_cellSizeX;
		}

		cellXCoord = 0;
		cellZCoord += m_cellSizeZ;
	}

	// Assign correct normals to all vertices
	for(size_t i=0; i<(*pNumOfVertices); i++)
	{
		size_t xIndex = size_t((*ppVertices)[i].position.x/m_cellSizeX);
		size_t zIndex = size_t((*ppVertices)[i].position.z/m_cellSizeZ);

		(*ppVertices)[i].normal = pNormals[zIndex*normStride+xIndex];
	}

	SAFE_DELETE_ARRAY(pNormals);

	return S_OK;
}

/**
    Counts how many cells in the 2D level map are taken in the rectangle of size sizeX by sizeY with the top-left corner at 
    (cornerXIndex, cornerYIndex). If any cells in this rectangle are out of the level map (not in its borders) then they are not considered 
*/
size_t Land::CountTakenCells(const WorldFile& levelData, int cornerXIndex, int cornerZIndex, size_t sizeX, size_t sizeZ)
{
	size_t takenCellCount = 0;

    for(int zIndex = cornerZIndex; zIndex < cornerZIndex + (int)sizeZ; zIndex++)
	{
		for(int xIndex = cornerXIndex; xIndex < cornerXIndex + (int)sizeX; xIndex++)
		{
			if(xIndex>=0 && xIndex<levelData.GetWidth() && zIndex>=0 && zIndex<levelData.GetHeight() && levelData(xIndex, zIndex) == WorldFile::OCCUPIED_CELL)
			{
				takenCellCount++;
			}
		}
	}

    return takenCellCount;
}

void Land::Update(double fTime)
{
    m_lastTime = fTime;

	// Call base class since this node is a static landscape and does not change (yet)
	Node::Update(fTime);
}

void Land::Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	// Turn off culling
	DWORD cullState;
	pd3dDevice->GetRenderState( D3DRS_CULLMODE, &cullState );
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Setup transformation matrices
    D3DXMATRIX world;
    D3DXMatrixIdentity(&world);
	Utilities::SetVertexShaderTransforms(pd3dDevice, world, viewProj);

	// Render the land
	pd3dDevice->SetTexture(0, m_pLandTexture);
	m_pLandMesh->DrawSubset(0);

	// Restore render states
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, cullState );

	// Call base class
	Node::Render(pd3dDevice, viewProj);
}

bool Land::IsOutsideBorders(const D3DXVECTOR3& point)
{
	if(point.x<0 || point.x>m_worldSizeX)
	{
		return true;
	}

	if(point.z<0 || point.z>m_worldSizeZ)
	{
		return true;
	}

	return false;
}

bool Land::IsLand(const D3DXVECTOR3& point)
{
	int cellXIndex = int(point.x/m_cellSizeX);
	int cellZIndex = int(point.z/m_cellSizeZ);

	if( m_levelData(cellXIndex, cellZIndex) == WorldFile::OCCUPIED_CELL )
	{
		// Land for sure
		return true;
	}

	// Check if there are neighbor cells that are land
	int numberOfTakenCells = CountTakenCells(m_levelData, cellXIndex-1, cellZIndex-1, 3, 3);
	if(numberOfTakenCells==0) 
	{
		// Open sea - bail.
		return false;
	}

	// Near the land - check if this point is "under" the corresponding land polygon
	size_t quadVertexStartIdex = (cellZIndex*m_numOfCellsX + cellXIndex)*4;
	Vertex3D* pVertex = m_pLandVertices + quadVertexStartIdex;

	// Flatten faces and tested point on y=0 plane to find which face it is over
	D3DXVECTOR3 ptNearLeft( pVertex[0].position.x, 0, pVertex[0].position.z);
	D3DXVECTOR3 ptFarLeft(  pVertex[1].position.x, 0, pVertex[1].position.z);
	D3DXVECTOR3 ptNearRight(pVertex[2].position.x, 0, pVertex[2].position.z);
	D3DXVECTOR3 ptFarRight( pVertex[3].position.x, 0, pVertex[3].position.z);
	D3DXVECTOR3 ptProj(point.x, 0, point.z);

	D3DXVECTOR3 crossProduct;
	D3DXVec3Cross(&crossProduct, &(ptFarLeft-ptNearRight), &(ptProj-ptNearRight));
	float distance = 0;
	
	// Unflatten
	ptNearLeft.y = pVertex[0].position.y;
	ptFarLeft.y = pVertex[1].position.y;
	ptNearRight.y = pVertex[2].position.y;
	ptFarRight.y = pVertex[3].position.y;

	if(crossProduct.y >= 0)
	{
		// Point is projected on to the far-right triangle (if looking from the (x=0, z=0) origin)
		distance = Utilities::ComputePointToPlaneDistance(&point, &ptNearRight, &ptFarRight, &ptFarLeft);
	}
	else
	{
		// Point is projected on to the near-left triangle (if looking from the (x=0, z=0) origin)
		distance = Utilities::ComputePointToPlaneDistance(&point, &ptFarLeft, &ptNearLeft, &ptNearRight);
	}

	if(distance<0)
	{
		// Point is below tested face, so this point is land
		return true;
	}
	else
	{
		// Point is above tested face, so this point is not land
		return false;
	}
}

WorldFile& Land::GetLevelDescription()
{
	return m_levelData;
}

