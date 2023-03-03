
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
#include "Creature.h"
#include "Ship.h"
#include "Utilities.h"

using namespace std;

extern bool	g_isInDebugging;

Creature::Creature(IDirect3DDevice9* pd3dDevice) : 
	Node(pd3dDevice), 
	m_lastTime(0),
	m_pMesh(0),
	m_ppTextures(0),
	m_numberOfMaterials(0),
	m_maxBoneAngle(D3DX_PI/12.0f),
	m_halfJointSizeZ(2.0f)
{
	m_jointLocations[0] = 2.5f;
	m_jointLocations[1] = 5.0f;
	m_jointLocations[2] = 7.5f;

	m_boneAngles[0] = 0;
	m_boneAngles[1] = 0;
	m_boneAngles[2] = 0;
	m_boneAngles[3] = 0;

	m_angularSpeed = m_maxBoneAngle; // radians/sec
	m_boneSpeeds[0] = 0;
	m_boneSpeeds[1] = m_angularSpeed;
	m_boneSpeeds[2] = m_angularSpeed;
	m_boneSpeeds[3] = m_angularSpeed;

	m_trajectoryPoints[0] = D3DXVECTOR3(20,0,20);
	m_trajectoryPoints[1] = D3DXVECTOR3(100,0,20);
	m_trajectoryPoints[2] = D3DXVECTOR3(200,0,100);
	m_trajectoryPoints[3] = D3DXVECTOR3(100,0,200);
	m_trajectoryPoints[4] = D3DXVECTOR3(50,0,100);

	m_currentLegPos = m_trajectoryPoints[1];

	m_currentCheckPointIndex = 1;
	m_currentLegPos = m_trajectoryPoints[m_currentCheckPointIndex];
	m_currentPos = m_currentLegPos;
	m_speed = 5.0f;
}

Creature::~Creature()
{
	Unload();

	if(m_ppTextures)
	{
		for(size_t i=0; i<m_numberOfMaterials; i++)
		{
			SAFE_RELEASE(m_ppTextures[i]);
		}
	}

	SAFE_DELETE_ARRAY(m_ppTextures);
	SAFE_RELEASE(m_pMesh);
}

HRESULT Creature::Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath)
{
	if(!szDataPath)
	{
		OutputDebugStringW(L"Invalid argument - path to level data file is not specified!");
		return E_INVALIDARG;
	}

	wstring xFilePath(szDataPath);
	xFilePath += L"\\Creature\\GreatShark.x";

	if(FAILED(Utilities::LoadXModel(pd3dDevice, xFilePath.c_str(), &m_pMesh, &m_ppTextures, &m_numberOfMaterials)))
	{
		wchar_t message[128];
		swprintf_s(message, 128, L"Could not load creature mesh from X file %s\n", xFilePath.c_str());
		OutputDebugStringW(message);
		return E_FAIL;
	}

	// Create bones
	Vertex3D* pVertices;
	m_pMesh->LockVertexBuffer(0, (void**)&pVertices);

	D3DXVECTOR3 minCorner, maxCorner;
	if( FAILED(D3DXComputeBoundingBox( (D3DXVECTOR3*)pVertices, m_pMesh->GetNumVertices(), m_pMesh->GetNumBytesPerVertex(), &minCorner, &maxCorner)) )
	{
		OutputDebugStringA("Cannot compute bounding box of the creature!\n");
		m_pMesh->UnlockVertexBuffer();
		return E_FAIL;
	}

    CreateSkeleton(minCorner, maxCorner);
    AssignBones(pVertices, m_pMesh->GetNumVertices(), minCorner, maxCorner);

	m_pMesh->UnlockVertexBuffer();

	// Flip model so it is facing towards positive Z
	Utilities::CreateWorldMatrix(&m_modelTransformMatrix, D3DXVECTOR3(0,1,0), D3DXVECTOR3(0,0,-1), D3DXVECTOR3(0,0,0));

    return S_OK;
}

void Creature::Update(double fTime)
{
    float deltaTime = (float)(fTime - m_lastTime);
	m_lastTime = fTime;

    // Move bones and compute corresponding transforms
	for(size_t i=0; i<SIZEOF_ARRAY(m_boneAngles); i++)
	{
		m_boneAngles[i] += float((double)m_boneSpeeds[i]*deltaTime);
		if(m_boneAngles[i]>=m_maxBoneAngle)
		{
			m_boneAngles[i] = m_maxBoneAngle;
			m_boneSpeeds[i] = -m_angularSpeed;
		}
		else if(m_boneAngles[i]<=-m_maxBoneAngle)
		{
			m_boneAngles[i] = -m_maxBoneAngle;
			m_boneSpeeds[i] = m_angularSpeed;
		}

		D3DXMatrixRotationY(&(m_bones[i].transformMatrix), m_boneAngles[i]);
	}

	// Calculate world movement
	D3DXVECTOR3 newPos = ComputeNewPosition(deltaTime);
	D3DXVECTOR3 direction = newPos - m_currentPos;
	D3DXMATRIX worldMatrix;
	Utilities::CreateWorldMatrix(&worldMatrix, D3DXVECTOR3(0,1,0), direction, D3DXVECTOR3(newPos.x,newPos.y,newPos.z));
	m_world = m_modelTransformMatrix * worldMatrix;
	m_currentPos = newPos;

	if(g_isInDebugging)
	{
		D3DXMatrixTranslation(&m_world, 10, 5, 10);
	}

	// Call base class since this node is a static landscape and does not change (yet)
	Node::Update(fTime);
}

void Creature::Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj)
{
	// Turn off culling
	DWORD cullState;
	pd3dDevice->GetRenderState( D3DRS_CULLMODE, &cullState );
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	D3DXMATRIX palette[MAX_BONE_MATRICES];
	ComputeBoneMatrixPalette(palette, SIZEOF_ARRAY(palette));

    // Setup transformation matrices
	Utilities::SetVertexShaderTransforms(pd3dDevice, m_world, viewProj);
    Utilities::SetVertexShaderBoneTransforms(pd3dDevice, palette, m_bones.size());

	for(UINT i=0; i<(UINT)m_numberOfMaterials; i++)
	{
		pd3dDevice->SetTexture(0, m_ppTextures[i]);
		m_pMesh->DrawSubset(i);
	}

	// Restore render states
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, cullState );

	// Call base class
	Node::Render(pd3dDevice, viewProj);
}

void Creature::CreateSkeleton(const D3DXVECTOR3& minCorner, const D3DXVECTOR3& maxCorner)
{
    D3DXMATRIX boneTransform;
    D3DXMatrixTranslation(&boneTransform, 0, 0, m_jointLocations[0]); // the distance from bone #1 or #2 root to the origin
    m_bones.push_back( Bone(D3DXVECTOR3(0, 0, m_jointLocations[0]), D3DXVECTOR3(0, 0, minCorner.z), -1, boneTransform) );

    m_bones.push_back( Bone(D3DXVECTOR3(0, 0, m_jointLocations[0]), D3DXVECTOR3(0, 0, m_jointLocations[1]), -1, boneTransform) );

    D3DXMatrixTranslation(&boneTransform, 0, 0, m_jointLocations[1]-m_jointLocations[0]); // the length of bone #2
    m_bones.push_back(Bone(D3DXVECTOR3(0, 0, m_jointLocations[1]), D3DXVECTOR3(0, 0, m_jointLocations[2]), 1 /* child of #1 */, boneTransform));

    D3DXMatrixTranslation(&boneTransform, 0, 0, m_jointLocations[2]-m_jointLocations[1]); // the length of bone #3
    m_bones.push_back(Bone(D3DXVECTOR3(0, 0, m_jointLocations[2]), D3DXVECTOR3(0, 0, maxCorner.z), 2 /* child of #2 */, boneTransform));

    ComputeSkinningMatrices();
}

void Creature::ComputeSkinningMatrices()
{
    for(size_t i=0; i<m_bones.size(); i++)
    {
        m_bones[i].skinningMatrix = m_bones[i].boneMatrix;
		if(m_bones[i].parent!=-1)
		{
			m_bones[i].skinningMatrix *= m_bones[m_bones[i].parent].skinningMatrix;
		}
        
        D3DXMatrixInverse(&(m_bones[i].invSkinningMatrix), NULL, &(m_bones[i].skinningMatrix));
    }
}

void Creature::ComputeBoneMatrixPalette(D3DXMATRIX palette[], size_t paletteSize)
{
	assert(paletteSize>=m_bones.size());
    for(size_t i=0; i<m_bones.size(); i++)
    {
        palette[i] = m_bones[i].invSkinningMatrix * m_bones[i].transformMatrix * m_bones[i].skinningMatrix;
		if(m_bones[i].parent!=-1)
		{
			palette[i] *= palette[m_bones[i].parent];
		}
    }
}

void Creature::AssignBones(Vertex3D* pVertices, size_t numberOfVertices, const D3DXVECTOR3& minCorner, const D3DXVECTOR3& maxCorner)
{
	float jointSizeZ = m_halfJointSizeZ * 2.0f;       // size of each joint

  	for(size_t vxIndex = 0; vxIndex<numberOfVertices; vxIndex++)
	{
		float z = pVertices[vxIndex].position.z;
		size_t i = 0;
        int bones[2] = {-1,-1};
        float weights[2] = {0,0};

        for(size_t boneIndex = 0; boneIndex<m_bones.size(); boneIndex++)
        {
            float zMin = min(m_bones[boneIndex].startPos.z, m_bones[boneIndex].endPos.z);
            float zMax = max(m_bones[boneIndex].startPos.z, m_bones[boneIndex].endPos.z);

            // Check if this vertice belongs to this bone
			if(zMin-m_halfJointSizeZ <= z && z <= zMax+m_halfJointSizeZ)
            {
                bones[i] = boneIndex;
                i++;
				if(i>=SIZEOF_ARRAY(bones))
				{
					break;
				}
            }
        }

		assert(bones[0]!=-1);
		if(bones[1]==-1)
		{
			// This vertice belongs to 1 bone
			weights[0] = 1;	
		}
		else
		{
			// This vertice belongs to 2 bones - calculate their weights
			float distToJoint = min(abs(z-m_bones[bones[0]].startPos.z), abs(z-m_bones[bones[0]].endPos.z));
			
			if((m_bones[bones[0]].startPos.z <= z && z <= m_bones[bones[0]].endPos.z) || (m_bones[bones[0]].endPos.z <= z && z <= m_bones[bones[0]].startPos.z))
			{
				// This vertice is inside bone 0 - larger weight
				weights[0] = 1 - (m_halfJointSizeZ - distToJoint) / jointSizeZ;
			}
			else
			{
				// This vertice is outside of bone 0 near the joint - smaller weight
				weights[0] = (m_halfJointSizeZ - distToJoint) / jointSizeZ;
			}

			assert(weights[0]>=0);
			weights[1] = 1-weights[0];	// the weight of the other bone is the oposite, weight[0] + weight[1] = 1
		}

        pVertices[vxIndex].boneIndices = D3DXVECTOR4(float(bones[0]),  float(bones[1]), -1.0f, -1.0f);
        pVertices[vxIndex].boneWeights = D3DXVECTOR4(weights[0], weights[1], 0.0f, 0.0f);

        // Tweak the color so it looks glossier :-)
        pVertices[vxIndex].specular = D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

D3DXVECTOR3 Creature::ComputeNewPosition(float deltaTime)
{
	D3DXVECTOR3 newPosition;
	int ind2 = m_currentCheckPointIndex;
	int ind1 = GetWrappedIndex(ind2-1, SIZEOF_ARRAY(m_trajectoryPoints));
	int ind3 = GetWrappedIndex(ind2+1, SIZEOF_ARRAY(m_trajectoryPoints));
	int ind4 = GetWrappedIndex(ind3+1, SIZEOF_ARRAY(m_trajectoryPoints));
	
	D3DXVECTOR3 legDirection = m_trajectoryPoints[ind2] - m_trajectoryPoints[ind3];
	float legLength = D3DXVec3Length(&legDirection);
	D3DXVECTOR3 currentLegDislocation = m_currentLegPos - m_trajectoryPoints[ind2];
	float legLengthTravelled = D3DXVec3Length(&currentLegDislocation);
	float s = (legLengthTravelled+deltaTime*m_speed)/legLength;
	D3DXVec3Lerp(&m_currentLegPos, &(m_trajectoryPoints[ind2]), &(m_trajectoryPoints[ind3]), s);

	D3DXVec3CatmullRom(&newPosition, 
		&(m_trajectoryPoints[ind1]), &(m_trajectoryPoints[ind2]), &(m_trajectoryPoints[ind3]), &(m_trajectoryPoints[ind4]),	s);

	if(s>=1.0f)
	{
		s = 0;
		m_currentCheckPointIndex = GetWrappedIndex(ind2+1, SIZEOF_ARRAY(m_trajectoryPoints));
		m_currentLegPos = m_trajectoryPoints[m_currentCheckPointIndex];
	}

	return newPosition;
}

int Creature::GetWrappedIndex(int i, int size)
{
	if(i<0) return size-1;
	if(i>=size) return 0;

	return i;
}