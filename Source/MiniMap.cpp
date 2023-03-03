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

#include "Scene.h"
#include "MiniMap.h"
#include "Ship.h"

DWORD LAND_COLOR = 0xff00CD00;
DWORD SEA_COLOR = 0x0a36648B;
DWORD PLAYER_COLOR = 0xffCD661D;
DWORD NPC_COLOR = 0xffff0000;

extern std::tr1::shared_ptr<Ship> g_playerShip;
extern std::tr1::shared_ptr<Node> g_scene;

MiniMap::MiniMap(IDirect3DDevice9* pd3dDevice, WorldFile& worldMap) : 
	m_mapWidth(100.0f),
	m_mapHeight(100.0f),
	Node(pd3dDevice), 
	m_pVertices(NULL),
	m_pMapTexture(NULL),
	m_pMaskTexture(NULL),
	m_pStateBlock(NULL)
{
	// Create and fill the map texture
	m_textureWidth = worldMap.GetWidth();
	m_textureHeight = worldMap.GetHeight();
	HRESULT hr = pd3dDevice->CreateTexture( m_textureWidth, m_textureHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pMapTexture, NULL);
	_ASSERT(SUCCEEDED(hr));
	D3DLOCKED_RECT lockedRect;
	hr = m_pMapTexture->LockRect(0, &lockedRect, NULL, 0);
	_ASSERT(SUCCEEDED(hr));
	
	DWORD* pPixel = (DWORD*)lockedRect.pBits;
	for(size_t yIndex = 0; yIndex < m_textureHeight; yIndex++)
	{
		for(size_t xIndex = 0; xIndex < m_textureWidth; xIndex++)
		{
			if( worldMap(xIndex, yIndex) == WorldFile::OCCUPIED_CELL )
			{
				*pPixel = LAND_COLOR;
			}
			else
			{
				*pPixel = SEA_COLOR;
			}

			pPixel++;
		}
	}

	m_pMapTexture->UnlockRect(0);

	// Create and fill the round mask texture
	UINT textureWidth = 128;
	hr = pd3dDevice->CreateTexture( textureWidth, textureWidth, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pMaskTexture, NULL);
	_ASSERT(SUCCEEDED(hr));
	lockedRect;
	hr = m_pMaskTexture->LockRect(0, &lockedRect, NULL, 0);
	_ASSERT(SUCCEEDED(hr));
	
	pPixel = (DWORD*)lockedRect.pBits;
	for(size_t yIndex = 0; yIndex < textureWidth; yIndex++)
	{
		for(size_t xIndex = 0; xIndex < textureWidth; xIndex++)
		{
			if( abs(int(yIndex)-int(textureWidth)/2)<=1 && abs(int(xIndex)-int(textureWidth)/2)<=1)
			{
				// Add a playable ship marker (or center of the minimap)
				*pPixel = PLAYER_COLOR;
			}
			else
			{
				FLOAT xDist = FLOAT(xIndex) - FLOAT(textureWidth)/2.0f;
				FLOAT yDist = FLOAT(yIndex) - FLOAT(textureWidth)/2.0f;

				if( sqrt(xDist*xDist+yDist*yDist) <= FLOAT(textureWidth)/2.0f )
				{
					*pPixel = 0xa0000000;
				}
				else
				{
					*pPixel = 0x00000000;
				}
			}

			pPixel++;
		}
	}

	m_pMaskTexture->UnlockRect(0);

	// Create map quad vertex buffer
	hr = pd3dDevice->CreateVertexBuffer(4*sizeof(MAPVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_MAPVERTEX, D3DPOOL_MANAGED, &m_pVertices, NULL);
	_ASSERT(SUCCEEDED(hr));
	PositionMap();
}

MiniMap::~MiniMap()
{
	SAFE_RELEASE(m_pVertices);
	SAFE_RELEASE(m_pMapTexture);
	SAFE_RELEASE(m_pMaskTexture);
}

void MiniMap::Update(double fTime)
{
	Node::Update(fTime);
}

void MiniMap::Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	pd3dDevice->CreateStateBlock( D3DSBT_ALL, &m_pStateBlock );
	m_pStateBlock->Capture();

	PositionMap();
	MarkShipsOnMap();

	D3DVIEWPORT9 viewPort;
	pd3dDevice->GetViewport(&viewPort);

	D3DXMATRIX matRotation;
	D3DXMatrixRotationZ(&matRotation, D3DX_PI/2.0f - g_playerShip->GetShipToXAxesAngle());
	D3DXMATRIX matPosition;
	D3DXMatrixTranslation(&matPosition, viewPort.Width/2.0f - m_mapWidth, (-1.0f)*viewPort.Height/2.0f + m_mapHeight, 0.0f);
	D3DXMATRIX matTransform;
	matTransform = matRotation * matPosition;
	
	D3DXMATRIX matView;
	D3DXMatrixIdentity(&matView);

	D3DXMATRIX matOrtho2DProjection;
	D3DXMatrixOrthoLH(&matOrtho2DProjection, FLOAT(viewPort.Width), FLOAT(viewPort.Height), 0.0f, 1.0f);

	pd3dDevice->SetTransform(D3DTS_WORLD, &matTransform);
	pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
	pd3dDevice->SetTransform(D3DTS_PROJECTION, &matOrtho2DProjection);

	// Set render states
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Set texture stages
	// STAGE 0 -- set map rendering
    pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 ); 
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTexture(0, m_pMapTexture);

	// STAGE 1 -- set circlular cutout from the map
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTexture(1, m_pMaskTexture);

	// Draw minimap
	pd3dDevice->SetFVF(D3DFVF_MAPVERTEX);
	pd3dDevice->SetStreamSource(0, m_pVertices, 0, sizeof(MAPVERTEX));
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

	// Restore original device state
	m_pStateBlock->Apply();

	// Call base class
	Node::Render(pd3dDevice, viewProj);

	SAFE_RELEASE(m_pStateBlock);
}

void MiniMap::PositionMap()
{
	MAPVERTEX* pVertices = NULL;
	HRESULT hr = m_pVertices->Lock(0, 4*sizeof(MAPVERTEX), (void**)&pVertices, 0);
	_ASSERT(SUCCEEDED(hr));

	// Set positions and texture coordinates
	pVertices[0].x = pVertices[3].x = -m_mapWidth / 2.0f;
	pVertices[1].x = pVertices[2].x = m_mapWidth / 2.0f;
	pVertices[0].y = pVertices[1].y = m_mapHeight / 2.0f;
	pVertices[2].y = pVertices[3].y = -m_mapHeight / 2.0f;
	pVertices[0].z = pVertices[1].z = pVertices[2].z = pVertices[3].z = 1.0f;
	
	D3DXVECTOR3 playerPosition = g_playerShip->GetPosition();
	FLOAT playerU = playerPosition.x / WORLD_SIZE_X;
	FLOAT playerV = playerPosition.z / WORLD_SIZE_Z;
	FLOAT mapWindowHalfSize = 0.2f; // half size of the minimap in texture coordinates

	FLOAT leftU = playerU - mapWindowHalfSize;
	FLOAT rightU = playerU + mapWindowHalfSize;
	FLOAT topV = playerV - mapWindowHalfSize;
	FLOAT bottomV = playerV + mapWindowHalfSize;

	pVertices[0].tu1 = pVertices[3].tu1 = playerU - mapWindowHalfSize;
	pVertices[2].tv1 = pVertices[3].tv1 = playerV - mapWindowHalfSize;
	pVertices[1].tu1 = pVertices[2].tu1 = playerU + mapWindowHalfSize;
	pVertices[0].tv1 = pVertices[1].tv1 = playerV + mapWindowHalfSize;

	pVertices[0].tu2 = pVertices[3].tu2 = 0;
	pVertices[2].tv2 = pVertices[3].tv2 = 0;
	pVertices[1].tu2 = pVertices[2].tu2 = 1;
	pVertices[0].tv2 = pVertices[1].tv2 = 1;

	m_pVertices->Unlock();
}

void MiniMap::MarkShipsOnMap()
{
	D3DLOCKED_RECT lockedRect;
	HRESULT hr = m_pMapTexture->LockRect(0, &lockedRect, NULL, 0);
	_ASSERT(SUCCEEDED(hr));
	
	// Restore old ship positions to original texture values
	for( std::list<UINT>::iterator it = m_shipPositions.begin(); it!=m_shipPositions.end(); ++it)
	{
		((DWORD*)lockedRect.pBits)[*it] = SEA_COLOR;
	}

	m_shipPositions.clear();

	// Draw new ship positions
	for(int i=0; i<g_scene->GetNumChildren(); i++)
	{
		std::tr1::shared_ptr<Node> node = g_scene->GetChild(i);
		
		Ship* pShip = dynamic_cast<Ship*>(node.get());
		if(pShip && !pShip->IsPlayable())
		{
			D3DXVECTOR3 shipPosition = pShip->GetPosition();
			UINT shipX = UINT((shipPosition.x/WORLD_SIZE_X)*FLOAT(m_textureWidth));
			UINT shipY = UINT((shipPosition.z/WORLD_SIZE_Z)*FLOAT(m_textureHeight));
			UINT shipTexelIndex = shipY*m_textureWidth + shipX;
			((DWORD*)lockedRect.pBits)[shipTexelIndex] = NPC_COLOR;

			// Remember ship's position
			m_shipPositions.push_back(shipTexelIndex);
		}
	}

	m_pMapTexture->UnlockRect(0);
}
