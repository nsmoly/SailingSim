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

#include <string>
using namespace std;

#include "Utilities.h"

HRESULT Utilities::SetVertexShaderTransforms(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& world, const D3DXMATRIX& viewProj)
{
    D3DXMATRIX vsConstants[2];
    D3DXMATRIX m;

    D3DXMatrixTranspose(&m, &viewProj);
    vsConstants[0] = m;

    D3DXMatrixTranspose(&m, &world);
	vsConstants[1] = m;

	pd3dDevice->SetVertexShaderConstantF(0, (float const*)vsConstants, 8);

    BOOL useSkinning = FALSE;
    pd3dDevice->SetVertexShaderConstantB(0, &useSkinning, 1);

    return S_OK;
}

HRESULT Utilities::SetVertexShaderBoneTransforms(
    IDirect3DDevice9* pd3dDevice, 
    const D3DXMATRIX* matrixPalette, 
    size_t matrixPaletteSize, 
    BOOL useSkinning
)
{
    if(matrixPaletteSize>MAX_BONE_MATRICES)
    {
        return E_INVALIDARG;
    }

    D3DXMATRIX m;
	UINT registerIndex = 14;
	for(size_t i=0; i<matrixPaletteSize; i++)
	{
		D3DXMatrixTranspose(&m, &matrixPalette[i]);
		pd3dDevice->SetVertexShaderConstantF(registerIndex, (float const*)m, 4);
		registerIndex += 4;
	}

    pd3dDevice->SetVertexShaderConstantB(0, &useSkinning, 1);

    return S_OK;
}

HRESULT Utilities::SetVertexShaderConstants(IDirect3DDevice9* pd3dDevice,  const D3DXVECTOR4& eyePt, const D3DXVECTOR4& lightDir)
{
    D3DXVECTOR4 vsConstants[2];
    vsConstants[0] = eyePt;
    vsConstants[1] = lightDir;

	pd3dDevice->SetVertexShaderConstantF(12, (float const*)vsConstants, 2);

    return S_OK;
}

HRESULT Utilities::SetVertexShaderWaveConstants(IDirect3DDevice9* pd3dDevice, const Wave waves[4], float time, float sqVisibility)
{
    D3DXVECTOR4 vsConstants[8];

    for(size_t i=0; i<4; i++)
    {
        vsConstants[i] = D3DXVECTOR4(waves[i].direction.x, waves[i].direction.y, waves[i].direction.z, 0);
        vsConstants[i+4] = D3DXVECTOR4(waves[i].amplitude, waves[i].speed, waves[i].frequency, waves[i].steepness);
    }

	pd3dDevice->SetVertexShaderConstantF(14, (float const*)vsConstants, 8);

    vsConstants[0] = D3DXVECTOR4(time,sqVisibility,0,0);
    pd3dDevice->SetVertexShaderConstantF(22, (float const*)vsConstants, 1);

    return S_OK;
}

HRESULT Utilities::SetPixelShaderConstants(IDirect3DDevice9* pd3dDevice, const D3DXCOLOR& ambientColor, const D3DXCOLOR& directionalLightColor, 
			const D3DXVECTOR4& lightAmbDiffSpecPower)
{
    D3DXVECTOR4 psConstants[3];
    *(D3DXCOLOR*)&psConstants[0] = ambientColor;
    *(D3DXCOLOR*)&psConstants[1] = directionalLightColor;
	psConstants[2] = lightAmbDiffSpecPower;

	pd3dDevice->SetPixelShaderConstantF(0, (float const*)psConstants, 3);

    return S_OK;
}

/*
	Loads a 3D model from a specified X file into a mesh with a custom vertex type. 
	Model's textures must be located in the same folder where .x file is. If some of are missing then the function fails.
	It creates and returns loaded ID3DXMesh mesh (with all vertices assigned correct values), 
	array of loaded textures (1 for each material) and its size (number of materials)
*/
HRESULT Utilities::LoadXModel(
	IDirect3DDevice9* pd3dDevice,
	const LPCWSTR szXFilePath, 
	LPD3DXMESH* ppMesh,
	LPDIRECT3DTEXTURE9** ppTextures,
	DWORD* pNumberOfMaterials
)
{
	LPD3DXBUFFER pMaterialBuffer;

	if(!szXFilePath || pNumberOfMaterials==NULL)
	{
		return E_INVALIDARG;
	}

	// Cut the folder out of the x file path
	wstring xFilePath(szXFilePath);
	UINT slashPos1 = xFilePath.find_last_of(L"/");
	slashPos1 = (slashPos1==wstring::npos) ? 0 : slashPos1;
	UINT slashPos2 = xFilePath.find_last_of(L"\\");
	slashPos2 = (slashPos2==wstring::npos) ? 0 : slashPos2;
	UINT slashPos = max(slashPos1, slashPos2);
	string xFilePathA(xFilePath.begin(), xFilePath.end());
	xFilePathA = xFilePathA.substr(0, slashPos);
	xFilePathA += "\\";

	ID3DXMesh* pTempMesh = NULL;
	ID3DXMesh* pCustomMesh = NULL;
	HRESULT hr = D3DXLoadMeshFromX(szXFilePath, D3DXMESH_SYSTEMMEM, pd3dDevice, NULL, &pMaterialBuffer, NULL, pNumberOfMaterials, &pTempMesh);
	pTempMesh->CloneMesh(D3DXMESH_SYSTEMMEM, Vertex3D::declaration, pd3dDevice, &pCustomMesh);
	// Sorts all vertices by their mesh subsets, so vertices that are in the same subset are contiguous in VB
	// This improves rendering perf and allows iterating over subsets to set some attributes (like material colors)
	pCustomMesh->Optimize(D3DXMESHOPT_ATTRSORT, NULL, NULL, NULL, NULL, ppMesh);
	SAFE_RELEASE(pTempMesh);
	SAFE_RELEASE(pCustomMesh);

	D3DXMATERIAL* pMaterials;
	pMaterials = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();
	*ppTextures = new LPDIRECT3DTEXTURE9[*pNumberOfMaterials];

	for(UINT i = 0; i < *pNumberOfMaterials; i++ )
	{
		(*ppTextures)[i] = NULL;
		if( pMaterials[i].pTextureFilename != NULL )
		{
			string textureFilePath(xFilePathA);
			textureFilePath	+= pMaterials[i].pTextureFilename;
			if( FAILED(D3DXCreateTextureFromFileA(pd3dDevice, textureFilePath.c_str(), &((*ppTextures)[i]))))
			{
				char message[128];
				sprintf_s(message, 128, "Could not load texture file %s\n", textureFilePath.c_str());
				OutputDebugStringA(message);
				return E_FAIL;
			}
		}
	}

	// Assign material attributes in each vertice
	DWORD numSections;
	(*ppMesh)->GetAttributeTable(NULL, &numSections);
	D3DXATTRIBUTERANGE* pAttributes = new D3DXATTRIBUTERANGE[numSections];
	(*ppMesh)->GetAttributeTable(pAttributes, &numSections);
	
	Vertex3D* pVertices;
	(*ppMesh)->LockVertexBuffer(0, (void**)&pVertices);

	D3DCOLORVALUE color;
	for(size_t i=0; i<numSections; i++)
	{
		size_t vxIndexStart = pAttributes[i].VertexStart; 
		size_t vxIndexEnd = vxIndexStart + pAttributes[i].VertexCount;
		for(size_t vxIndex = vxIndexStart; vxIndex<vxIndexEnd; vxIndex++)
		{
			color = pMaterials[pAttributes[i].AttribId].MatD3D.Diffuse;
			pVertices[vxIndex].diffuse = D3DCOLOR_COLORVALUE( color.r, color.g, color.b, color.a);
			color = pMaterials[pAttributes[i].AttribId].MatD3D.Specular;
			pVertices[vxIndex].specular = D3DCOLOR_COLORVALUE( color.r, color.g, color.b, color.a);
		}
	}

	(*ppMesh)->UnlockVertexBuffer();
	
	SAFE_DELETE_ARRAY(pAttributes);
	SAFE_RELEASE(pMaterialBuffer);

	return S_OK;
}

HRESULT Utilities::CreateMesh(IDirect3DDevice9* pd3dDevice, Vertex3D* pVertices, DWORD numberOfVertices, DWORD* pMeshFaces, DWORD numberOfFaces, ID3DXMesh** ppMesh)
{
    // Create the mesh
	if( FAILED(D3DXCreateMesh(numberOfFaces, numberOfVertices, D3DXMESH_32BIT | D3DXMESH_SYSTEMMEM | D3DXMESH_MANAGED, Vertex3D::declaration, pd3dDevice, ppMesh)) )
    {
        return E_FAIL;
    }

    // Fill the vertex buffer
	Vertex3D* pVertexBuffer;
	if( FAILED((*ppMesh)->LockVertexBuffer(0, (LPVOID*)&pVertexBuffer)) )
	{
		return E_FAIL;
	}

	memcpy( pVertexBuffer, pVertices, numberOfVertices*sizeof(Vertex3D) );
	(*ppMesh)->UnlockVertexBuffer();

	// Fill the index buffer
	DWORD* pIndexBuffer;
	if( FAILED((*ppMesh)->LockIndexBuffer(0, (LPVOID*)&pIndexBuffer)) )
	{
		return E_FAIL;
	}
	
	memcpy( pIndexBuffer, pMeshFaces, numberOfFaces*sizeof(DWORD)*3 );
	(*ppMesh)->UnlockIndexBuffer();

	return S_OK;
}

/*
	Flips all normals to the oposite direction in a give mesh. If subsetIndex == -1 it 
	flips all normals, if it is set to mesh'es subset then it flips them only in this subset.
	If subsetIndex is set, then mesh must have its vertices sorted by their subset IDs
	(can be sorted by calling Optimize() method on ID3DXMesh)
*/
HRESULT Utilities::FlipNormals(ID3DXMesh* pMesh, int subsetIndex)
{
	assert(pMesh);
	if(!pMesh)
	{
		return E_INVALIDARG;
	}

	UINT startVerticeIndex = 0;
	UINT endVerticeIndex = pMesh->GetNumVertices();

	if(subsetIndex!=-1)
	{
		DWORD numSections;
		if(FAILED(pMesh->GetAttributeTable(NULL, &numSections)))
		{
			OutputDebugStringA("Utilities::FlipNormals - Cannot get attribute table for this mesh!\n");
			return E_FAIL;
		}

		D3DXATTRIBUTERANGE* pAttributes = new D3DXATTRIBUTERANGE[numSections];
		if(FAILED(pMesh->GetAttributeTable(pAttributes, &numSections)))
		{
			OutputDebugStringA("Utilities::FlipNormals - Cannot get attribute table for this mesh!\n");
			return E_FAIL;
		}

		bool foundSection = false;
		for(size_t i=0; i<numSections; i++)
		{
			if(subsetIndex == pAttributes[i].AttribId)
			{
				startVerticeIndex = pAttributes[i].VertexStart;
				endVerticeIndex = startVerticeIndex + pAttributes[i].VertexCount;
				foundSection = true;
				break;
			}
		}

		SAFE_DELETE_ARRAY(pAttributes);

		if(!foundSection)
		{	
			OutputDebugStringA("Utilities::FlipNormals - This subset does not exist in this mesh!\n");
			return E_INVALIDARG;
		}
	}

	Vertex3D* pVertexBuffer = NULL;
	if( FAILED(pMesh->LockVertexBuffer(0, (LPVOID*)&pVertexBuffer)) )
	{
		return E_FAIL;
	}
	
	for(size_t i=startVerticeIndex; i<endVerticeIndex; i++)
	{
		pVertexBuffer[i].normal *= (-1.0f);
	}
	
	pMesh->UnlockVertexBuffer();

	return S_OK;
}

HRESULT Utilities::DrawLineSegment(IDirect3DDevice9* pd3dDevice, const D3DXVECTOR3& point1, const D3DXVECTOR3& point2, const D3DCOLOR& color)
{
	IDirect3DVertexBuffer9* pLineVB;
	HRESULT hr = pd3dDevice->CreateVertexBuffer( 2*sizeof(Vertex3D), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pLineVB, NULL );
	if( FAILED(hr) )
	{
		return hr;
	}
	
	Vertex3D* pVertices;
	hr = pLineVB->Lock( 0, 0, (void**)&pVertices, 0 );
	if( FAILED(hr) )
	{
		return hr;
	}

	pVertices[0].position = point1;
	pVertices[0].diffuse = color;

	pVertices[1].position = point2;
	pVertices[1].diffuse = color;

	pLineVB->Unlock();

	// Turn off culling
	DWORD cullState;
	pd3dDevice->GetRenderState( D3DRS_CULLMODE, &cullState );
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	pd3dDevice->SetStreamSource(0, pLineVB, 0, sizeof(Vertex3D));
	pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);

	// Restore render states
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, cullState );

	SAFE_RELEASE(pLineVB);

	return S_OK;
}

/**
	Checks if point projection onto a given face plane belongs to this face
	The function supports either 3 point face (triangle) or 4 point face (any polygon with 4 vertices)
	pt1,pt2,pt3,pt4 vertices must be given in counterclockwise order
*/
bool Utilities::IsPointInsideFace(const D3DXVECTOR3* pPoint, const D3DXVECTOR3* pPt1, const D3DXVECTOR3* pPt2, const D3DXVECTOR3* pPt3, const D3DXVECTOR3* pPt4)
{
	D3DXPLANE plane;
	D3DXPlaneFromPoints(&plane, pPt1, pPt2, pPt3);
	D3DXVECTOR3 normal(plane.a, plane.b, plane.c); // since vertices in CCW order this plane normal will point down if plane is horizontal (X-Z plane)

	D3DXVECTOR3 verticeToPointVector = *pPoint - *pPt1;
	D3DXVECTOR3 verticeToNextVerticeVector = *pPt2 - *pPt1;
	D3DXVECTOR3 crossProduct;
	D3DXVec3Cross(&crossProduct, &verticeToNextVerticeVector, &verticeToPointVector);
	float dotProduct = D3DXVec3Dot(&crossProduct, &normal);
	if(dotProduct<0)
	{
		return false;
	}

	verticeToPointVector = *pPoint - *pPt2;
	verticeToNextVerticeVector = *pPt3 - *pPt2;
	crossProduct;
	D3DXVec3Cross(&crossProduct, &verticeToNextVerticeVector, &verticeToPointVector);
	dotProduct = D3DXVec3Dot(&crossProduct, &normal);
	if(dotProduct<0)
	{
		return false;
	}

	if(pPt4)
	{
		verticeToPointVector = *pPoint - *pPt3;
		verticeToNextVerticeVector = *pPt4 - *pPt3;
	}
	else
	{
		verticeToPointVector = *pPoint - *pPt3;
		verticeToNextVerticeVector = *pPt1 - *pPt3;
	}
	crossProduct;
	D3DXVec3Cross(&crossProduct, &verticeToNextVerticeVector, &verticeToPointVector);
	dotProduct = D3DXVec3Dot(&crossProduct, &normal);
	if(dotProduct<0)
	{
		return false;
	}

	if(pPt4)
	{
		verticeToPointVector = *pPoint - *pPt4;
		verticeToNextVerticeVector = *pPt1 - *pPt4;
		crossProduct;
		D3DXVec3Cross(&crossProduct, &verticeToNextVerticeVector, &verticeToPointVector);
		dotProduct = D3DXVec3Dot(&crossProduct, &normal);
		if(dotProduct<0)
		{
			return false;
		}
	}

	return true;
}

/**
	Computes point to plane distance. The plane is given by 3 points (triangle vertices) in CCW order.
	If the returned distance is positive then the point is above the triangle plane
	If the returned distance is negative then the point is below the triangle plane
*/
float Utilities::ComputePointToPlaneDistance(const D3DXVECTOR3* pPoint, const D3DXVECTOR3* pPt1, const D3DXVECTOR3* pPt2, const D3DXVECTOR3* pPt3)
{
	D3DXPLANE plane;
	D3DXPlaneFromPoints(&plane, pPt1, pPt2, pPt3);
	D3DXVECTOR3 normal(plane.a, plane.b, plane.c); // since vertices in CCW order this plane normal will point down if plane is horizontal (X-Z plane)

	D3DXVECTOR3 pointToVerticeVector = *pPt1 - *pPoint;
	float distance = D3DXVec3Dot(&pointToVerticeVector, &normal);
	
	return distance;
}

/**
	Computes transform matrix that moves standard left handed system to a new location and orientation
*/
void Utilities::CreateWorldMatrix(D3DXMATRIX* pMatrix, const D3DXVECTOR3& up, const D3DXVECTOR3& direction, const D3DXVECTOR3& translation)
{
	D3DXVECTOR3 newRight;
	D3DXVec3Cross(&newRight, &up, &direction);
	D3DXVec3Normalize(&newRight, &newRight);

	D3DXVECTOR3 newForward;
	D3DXVec3Cross(&newForward, &newRight, &up);
	D3DXVec3Normalize(&newForward, &newForward);

	D3DXVECTOR3 newUp;
	D3DXVec3Cross(&newUp, &newForward, &newRight);
	D3DXVec3Normalize(&newUp, &newUp);

	*pMatrix = D3DXMATRIX( newRight.x,	  newRight.y,    newRight.z,	 0,
						   newUp.x,		  newUp.y,	     newUp.z,		 0,
						   newForward.x,  newForward.y,  newForward.z,	 0,
						   translation.x, translation.y, translation.z,	 1
						 );
}

/**
	Represents the given angle (in radians) as the angle in 0 to +2*PI range and returns it
*/
float Utilities::NormalizeAngleInRadians(float angle)
{
	if(angle<0)
	{
		while(angle < -2.0f*D3DX_PI)
		{
			angle += 2.0f*D3DX_PI;
		}

		angle += 2.0f*D3DX_PI;
	}
	else
	{
		while(angle > 2.0f*D3DX_PI)
		{
			angle -= 2.0f*D3DX_PI;
		}
	}

	return angle;
}
