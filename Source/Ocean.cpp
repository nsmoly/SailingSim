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
#include "Ocean.h"
#include "Utilities.h"

using namespace std;

extern IDirect3DVertexShader9* g_pVertexShader;
extern IDirect3DVertexShader9* g_pWaveVertexShader;
extern IDirect3DPixelShader9*  g_pWavePixelShader;
extern IDirect3DPixelShader9*  g_pPixelShaderTexture;

Ocean::Ocean(IDirect3DDevice9* pd3dDevice) : 
	Node(pd3dDevice), 
    m_lastTime(0),
	m_nextWaveUpdateTime(10),
	m_nextWaveToUpdate(1),
	m_pSeaMesh(0),
	m_pSeaTexture(0),
    m_pSeaNormalMap(0),
	m_maxWaveCoordVariation(MAX_WAVEDIR_VARIATION)
{
}

Ocean::~Ocean()
{
	SAFE_RELEASE(m_pSeaMesh);
	SAFE_RELEASE(m_pSeaTexture);
    SAFE_RELEASE(m_pSeaNormalMap);
}

HRESULT Ocean::Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath)
{
	if(!szDataPath)
	{
		OutputDebugStringW(L"Invalid argument - path to data directory is not specified!");
		return E_INVALIDARG;
	}

	// Load textures
	wstring seaTextureFilePath(szDataPath);
	//seaTextureFilePath += L"\\World\\seaRough.png";
	seaTextureFilePath += L"\\World\\sea.jpg";
	SAFE_RELEASE(m_pSeaTexture);	
	if( FAILED(D3DXCreateTextureFromFile(pd3dDevice, seaTextureFilePath.c_str(), &m_pSeaTexture)) )
	{
		OutputDebugStringW(L"Failed to load sea texture");
		return E_FAIL;
	}

   	wstring seaNormalMapFilePath(szDataPath);
	//seaNormalMapFilePath += L"\\World\\seaRoughNormalMap.png";
	seaNormalMapFilePath += L"\\World\\seaNormalMap.jpg";
	SAFE_RELEASE(m_pSeaNormalMap);	
	if( FAILED(D3DXCreateTextureFromFile(pd3dDevice, seaNormalMapFilePath.c_str(), &m_pSeaNormalMap)) )
	{
		OutputDebugStringW(L"Failed to load sea normal map");
		return E_FAIL;
	}

	// Calculate & remember world dimensions
	m_worldSizeX = WORLD_SIZE_X;
	m_worldSizeZ = WORLD_SIZE_Z;
	m_cellSizeX = SEA_CELL_STEPX;
	m_cellSizeZ = SEA_CELL_STEPZ;

	Vertex3D* pVertices;
	DWORD numberOfVertices;
	DWORD* pMeshIndexes;
	DWORD numberOfTriangles;

	// Create sea mesh
	if( FAILED(CreateSea(&pVertices, &numberOfVertices, &pMeshIndexes, &numberOfTriangles)) )
	{
		OutputDebugStringW(L"Failed to create data arrays for the sea mesh");
		return E_FAIL;
	}

	SAFE_RELEASE(m_pSeaMesh);
	if( FAILED(Utilities::CreateMesh(pd3dDevice, pVertices, numberOfVertices, pMeshIndexes, numberOfTriangles, &m_pSeaMesh)) )
	{
		OutputDebugStringW(L"Failed to create D3DX sea mesh");
		return E_FAIL;
	}

	SAFE_DELETE_ARRAY(pVertices);
	SAFE_DELETE_ARRAY(pMeshIndexes);

	// Initialize sea wave pack based on 1 main wave [0] index
	memset(m_waves, 0, sizeof(m_waves));
	m_maxWaveCoordVariation = MAX_WAVEDIR_VARIATION;
	m_waves[0] = Wave(D3DXVECTOR3(0,0,1), 0.5f /* amplitude */, 3.0f /* speed */, 2.0f*D3DX_PI/28.f /* frequency */, 0.9f /* steepness */);
	//m_waves[0] = Wave(D3DXVECTOR3(0,0,1), 0.2f, 1.5f, 2.0f*D3DX_PI/20.f, 0.9f);
	//m_waves[0] = Wave(D3DXVECTOR3(0,0,1), 1.0f, 1.5f, 2.0f*D3DX_PI/20.f, 0.9f);
	m_waves[1] = GenerateRandomWave(m_waves[0]);
	m_waves[2] = GenerateRandomWave(m_waves[0]);
    m_waves[3] = GenerateRandomWave(m_waves[0]);

    return S_OK;
}

HRESULT Ocean::CreateSea(
	Vertex3D** ppVertices, 
	DWORD* pNumOfVertices, 
	DWORD** ppMesh, 
	DWORD* pNumOfTriangles
)
{
	// Common color properties for all vertices
	D3DXCOLOR diffuseColor(0.5f, 0.5f, 0.5f, 0.5f);
	D3DXCOLOR specularColor(0.5f, 0.5f, 0.5f, 0.5f);

	*ppVertices = NULL;
	*pNumOfVertices = 0;
	*ppMesh = NULL;
	*pNumOfTriangles = 0;

	// Allocate vertex array
	*pNumOfVertices = SEA_NCELL_X*SEA_NCELL_Z*4;
	*ppVertices = new Vertex3D[*pNumOfVertices];
	memset(*ppVertices, 0, (*pNumOfVertices)*sizeof(Vertex3D));

  	// Allocate index array
	*pNumOfTriangles = SEA_NCELL_X*SEA_NCELL_Z*2;
	*ppMesh = new DWORD[(*pNumOfTriangles)*3];
	memset(*ppMesh, 0, (*pNumOfTriangles)*sizeof(DWORD)*3);

	// Go over all mesh nodes and create corresponding vertices with correct parameters
	size_t vertexIndex = 0;
    size_t indexIndex = 0;
	float cellXCoord = 0;
	float cellZCoord = 0;
	float cellSizeX = SEA_CELL_STEPX;
	float cellSizeZ = SEA_CELL_STEPZ;
    float texU = 0.0f;
	float texV = 0.0f;
	float texUStep = 1.0f / SEA_TEX_NCELL_X;
	float texVStep = 1.0f / SEA_TEX_NCELL_Z;

	for(size_t zIndex = 0; zIndex < SEA_NCELL_Z; zIndex++)
	{
		float nextCellZ = cellZCoord + cellSizeZ;
		if(zIndex==SEA_NCELL_Z-1)
		{
			nextCellZ = SEA_NCELL_Z*SEA_CELL_STEPZ;
		}

		for(size_t xIndex = 0; xIndex < SEA_NCELL_X; xIndex++)
		{
            texU = texUStep*(xIndex%SEA_TEX_NCELL_X);
            texV = 1.0f - texVStep*(zIndex%SEA_TEX_NCELL_Z);

			float nextCellX = cellXCoord + cellSizeX;
			if(xIndex==SEA_NCELL_X-1)
			{
				nextCellX = SEA_NCELL_X*SEA_CELL_STEPX;
			}

            // Create 4 vertices for all 4 cell corners
			(*ppVertices)[vertexIndex++] = Vertex3D(D3DXVECTOR3(float(cellXCoord), 0, float(cellZCoord)), D3DXVECTOR2(texU, texV), 
									D3DXVECTOR3(0,1,0), diffuseColor, specularColor);

            (*ppVertices)[vertexIndex++] = Vertex3D(D3DXVECTOR3(float(nextCellX), 0, float(cellZCoord)), D3DXVECTOR2(texU+texUStep, texV), 
									D3DXVECTOR3(0,1,0), diffuseColor, specularColor);

            (*ppVertices)[vertexIndex++] = Vertex3D(D3DXVECTOR3(float(cellXCoord), 0, float(nextCellZ)), D3DXVECTOR2(texU, texV-texVStep),
									D3DXVECTOR3(0,1,0), diffuseColor, specularColor);

            (*ppVertices)[vertexIndex++] = Vertex3D(D3DXVECTOR3(float(nextCellX), 0, float(nextCellZ)), D3DXVECTOR2(texU+texUStep, texV-texVStep),
									D3DXVECTOR3(0,1,0), diffuseColor, specularColor);

            // Create 2 faces corresponding to created vertices. Faces are created by connecting vertices in CCW fashion
            (*ppMesh)[indexIndex++] = vertexIndex-4; // topleft vertice
            (*ppMesh)[indexIndex++] = vertexIndex-2; // bottomleft
            (*ppMesh)[indexIndex++] = vertexIndex-3; // topright
            (*ppMesh)[indexIndex++] = vertexIndex-3; // topright
            (*ppMesh)[indexIndex++] = vertexIndex-2; // bottomleft
            (*ppMesh)[indexIndex++] = vertexIndex-1; // bottomright

			cellXCoord += cellSizeX;
			texU += texUStep;
			if(texU == 1.0f)
			{
				texU = 0.0f;
			}
		}

		cellXCoord = 0;
		cellZCoord += cellSizeZ;
		texV += texVStep;
		if(texV == 1.0f)
		{
			texV = 0.0f;
		}
	}

	return S_OK;
}

void Ocean::Update(double fTime)
{
    m_lastTime = fTime;

	// Call base class since this node is a static landscape and does not change (yet)
	Node::Update(fTime);
}

void Ocean::Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	const float maxVisibleDistanceSq = (WORLD_SIZE_X*0.5f)*(WORLD_SIZE_X*0.5f);

	// Turn off culling
	DWORD cullState;
	pd3dDevice->GetRenderState( D3DRS_CULLMODE, &cullState );
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Setup shader and its parameters
    pd3dDevice->SetVertexShader(g_pWaveVertexShader);
	pd3dDevice->SetPixelShader(g_pWavePixelShader);

    Utilities::SetVertexShaderWaveConstants(pd3dDevice, m_waves, float(m_lastTime), maxVisibleDistanceSq);

    D3DXVECTOR4 psConst(float(m_lastTime), maxVisibleDistanceSq, 0, 0);	// create helper function for these
	pd3dDevice->SetPixelShaderConstantF(3, (float const*)&psConst, 1);

    // Setup transformation matrices
    D3DXMATRIX world;
    D3DXMatrixIdentity(&world);
	Utilities::SetVertexShaderTransforms(pd3dDevice, world, viewProj);

	// Set texture and normal map
	pd3dDevice->SetTexture(0, m_pSeaTexture);
    pd3dDevice->SetTexture(1, m_pSeaNormalMap);

	// Render
    //m_pSeaMesh->DrawSubset(0);

    // Tile sea grid over the world -- this does not stitch well yet. Need to come up with how to avoid floating point errors when stitching moving surfaces
	int xOffset = 0;
	int zOffset = 0;
	for(size_t zInd=0; zInd<(size_t)WORLD_SIZE_Z/(SEA_NCELL_Z*SEA_CELL_STEPZ); zInd++)
	{
		for(size_t xInd=0; xInd<(size_t)WORLD_SIZE_X/(SEA_NCELL_X*SEA_CELL_STEPX); xInd++)
		{
			D3DXMatrixTranslation(&world, (float)xOffset, 0, (float)zOffset);
			Utilities::SetVertexShaderTransforms(pd3dDevice, world, viewProj);
			m_pSeaMesh->DrawSubset(0);
			
			xOffset += SEA_NCELL_X*SEA_CELL_STEPX;
		}
		
		xOffset = 0;
		zOffset += SEA_NCELL_Z*SEA_CELL_STEPZ;
	}

    pd3dDevice->SetVertexShader(g_pVertexShader);
	pd3dDevice->SetPixelShader(g_pPixelShaderTexture);

	// Restore render states
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, cullState );
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	// Call base class
	Node::Render(pd3dDevice, viewProj);
}

Wave Ocean::GenerateRandomWave(const Wave& refWave)
{
	float refWaveLength = 2.0f*D3DX_PI / refWave.frequency;
	float refHalfWaveLength = refWaveLength / 2.0f;
	float refDoubleWaveLength = refHalfWaveLength*2.0f;

	float randCoordVariation = (Utilities::RandF()*2.0f-1.0f) * m_maxWaveCoordVariation;
	Wave wave;
	wave.direction.x = refWave.direction.x + randCoordVariation;
	wave.direction.y = refWave.direction.y;
	wave.direction.z = refWave.direction.z + randCoordVariation;
	
	float waveLength = Utilities::RandF()*(refDoubleWaveLength-refHalfWaveLength) + refHalfWaveLength;
	wave.speed = sqrt(9.8f*2.0f*D3DX_PI/waveLength); // 9.8 = g here. This approximates deep water wave speed based on wavelength
	wave.frequency = 2.0f*D3DX_PI/waveLength;
	wave.amplitude = refWave.amplitude/refWaveLength * waveLength;
	wave.steepness = Utilities::RandF();
	
	return wave;
}

/*
	Computes wave sum values at a given point and time. Computations are based on Gerstner formula (see GPU Gems #1)
*/
bool Ocean::ComputeWaveAt(float x, float z, double time, D3DXVECTOR3* pPosition, D3DXVECTOR3* pNormal, D3DXVECTOR3* pTangent)
{
	// Use only main wave for now to compute less
	D3DXVECTOR3 position(0,0,0);
	D3DXVECTOR3 normal(0,0,0);
	D3DXVECTOR3 tangent(0,0,0);
	
	// Accumulate contributions to delta change from different waves
	for(size_t i=0; i<SIZEOF_ARRAY(m_waves); i++)
	{
		float feta = m_waves[i].speed*m_waves[i].frequency;
		float WA = m_waves[i].frequency*m_waves[i].amplitude;
		float K = m_waves[i].steepness*m_waves[i].amplitude;

		float S = m_waves[i].frequency*D3DXVec3Dot(&(m_waves[i].direction), &D3DXVECTOR3(x,0,z)) + feta*(float)time;
		float cosS = cos(S);
		float sinS = sin(S);
		
		position.x += K*m_waves[i].direction.x*cosS;
		position.z += K*m_waves[i].direction.z*cosS;
		position.y += m_waves[i].amplitude*sinS;

		normal.x += m_waves[i].direction.x*WA*cosS;
		normal.y += m_waves[i].steepness*WA*sinS;
		normal.z += m_waves[i].direction.z*WA*cosS;

		tangent.x += m_waves[i].steepness*m_waves[i].direction.x*m_waves[i].direction.z*WA*sinS;
		tangent.y += m_waves[i].direction.z*WA*cosS;
		tangent.z += m_waves[i].steepness*m_waves[i].direction.z*m_waves[i].direction.z*WA*sinS;
	}

	if(pPosition)
	{
		position.x = x + position.x;
		// position.y = 0 + position.y; - y is 0 at the begining, so it changes around 0
		position.z = z + position.z;
		*pPosition = position;
	}

	if(pNormal)
	{
		normal.x = -normal.x;
		normal.y = 1-normal.y;
		normal.z = -normal.z;
		D3DXVec3Normalize(pNormal, &normal);
	}

	if(pTangent)
	{
		tangent.x = -tangent.x;
		// tangent.y = tangent.y; already accumulated and correct
		tangent.z = 1-tangent.z;
		D3DXVec3Normalize(pTangent, &tangent);
	}

	return true;
}
