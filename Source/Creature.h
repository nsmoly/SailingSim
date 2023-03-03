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

#include <vector>
#include "Node.h"
#include "Utilities.h"

struct Bone
{
    Bone(const D3DXVECTOR3& startCoord, const D3DXVECTOR3& endCoord, int parentIndex, const D3DXMATRIX& boneTransform) :
        startPos(startCoord), endPos(endCoord), parent(parentIndex), boneMatrix(boneTransform)
    {
    }

    D3DXVECTOR3 startPos;           // bone root position in model coordinates
    D3DXVECTOR3 endPos;             // bone end position in model coordinates
    int parent;                     // -1 if no parent else index of the parent bone in the bone array (packed tree)
    D3DXMATRIX boneMatrix;          // Bone matrix that transforms this bone to its position in bone space
    D3DXMATRIX skinningMatrix;      // concatenated transform matrix that moves a vertex from a bone space to a model space
                                    // It is = B*P1*P2*... where B is bone matrix, P1 is parent and P2..PN is grandparent matrices
    D3DXMATRIX invSkinningMatrix;   // inverse of skinningMatrix
	D3DXMATRIX transformMatrix;		// transform matrix to animate this bone
};

class Creature : public Node
{
public:
	Creature(IDirect3DDevice9* pd3dDevice);
	virtual ~Creature();

	virtual HRESULT	Load(IDirect3DDevice9* pd3dDevice, const LPCWSTR szDataPath);
	
	// Update traversal for physics, AI, etc.
	virtual void Update(double fTime);

	// Render traversal for drawing objects
	virtual void Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& viewProj);

private:
    void CreateSkeleton(const D3DXVECTOR3& minCorner, const D3DXVECTOR3& maxCorner);
    void ComputeSkinningMatrices();
    void AssignBones(Vertex3D* pVertices, size_t numberOfVertices, const D3DXVECTOR3& minCorner, const D3DXVECTOR3& maxCorner);
	void ComputeBoneMatrixPalette(D3DXMATRIX palette[], size_t paletteSize);
	D3DXVECTOR3 ComputeNewPosition(float deltaTime);
	int GetWrappedIndex(int i, int size);

	double m_lastTime;
    D3DXMATRIX m_world;
	D3DXMATRIX m_modelTransformMatrix;

	ID3DXMesh* m_pMesh;
	LPDIRECT3DTEXTURE9* m_ppTextures;
	DWORD m_numberOfMaterials;

	// Skeleton definition
	const float m_halfJointSizeZ;
	float m_jointLocations[3];
	std::vector<Bone> m_bones;

	// Bone animation states
	float m_boneAngles[4];
    float m_boneSpeeds[4];
    float m_angularSpeed;		// angular speed of bone movements in radians/sec
	float m_maxBoneAngle;

	// World movement animation states
	D3DXVECTOR3 m_trajectoryPoints[5];
	float m_speed;	// m/s
	D3DXVECTOR3 m_currentPos;
	D3DXVECTOR3 m_currentLegPos;
	size_t m_currentCheckPointIndex;
};

