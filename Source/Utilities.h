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

#define MAX_BONE_MATRICES   16
#define D3DXWhite D3DXCOLOR(1, 1, 1, 1)
#define D3DXBlack D3DXCOLOR(0, 0, 0, 1)

#define SIZEOF_ARRAY(arr) sizeof(arr)/sizeof(arr[0])

// Vertex in world (or model) coordinates to be used with vertex shaders
struct Vertex3D
{
	Vertex3D() : 
		position(D3DXVECTOR3(0,0,0)), normal(D3DXVECTOR3(0,0,0)), 
		diffuse(D3DCOLOR(0xff000000)), specular(D3DCOLOR(0xff000000)), texCoord(D3DXVECTOR2(0,0)), 
        boneIndices(D3DXVECTOR4(0,0,0,0)), boneWeights(D3DXVECTOR4(0,0,0,0))
	{ 
	}

	Vertex3D( D3DXVECTOR3 positionValue, D3DXVECTOR2 texCoordValue, D3DXVECTOR3 normalValue = D3DXVECTOR3(0,0,0), 
			  D3DCOLOR diffuseValue = D3DCOLOR(0xff000000), D3DCOLOR specularValue = D3DCOLOR(0xff000000), 
			  D3DXVECTOR4 boneIndicesValue = D3DXVECTOR4(0,0,0,0), D3DXVECTOR4 boneWeightsValue = D3DXVECTOR4(0,0,0,0)
	) : 
		position(positionValue), normal(normalValue), diffuse(diffuseValue), specular(specularValue), texCoord(texCoordValue),
        boneIndices(boneIndicesValue), boneWeights(boneWeightsValue)
	{
	}

    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DCOLOR    diffuse;
    D3DCOLOR    specular;
    D3DXVECTOR2 texCoord;
	D3DXVECTOR4 boneIndices;
    D3DXVECTOR4 boneWeights;

    static IDirect3DVertexDeclaration9* pVD;
    static D3DVERTEXELEMENT9 const declaration[];
};

#ifdef GAME_CPP

// Vertex declarations instantiation (declared as statics in Vertex3D class)
IDirect3DVertexDeclaration9* Vertex3D::pVD = NULL;
D3DVERTEXELEMENT9 const Vertex3D::declaration[] =
{
    { 0, offsetof(Vertex3D, position), D3DDECLTYPE_FLOAT3,    0, D3DDECLUSAGE_POSITION,     0 },
    { 0, offsetof(Vertex3D, normal  ), D3DDECLTYPE_FLOAT3,    0, D3DDECLUSAGE_NORMAL,       0 },
    { 0, offsetof(Vertex3D, diffuse ), D3DDECLTYPE_D3DCOLOR,  0, D3DDECLUSAGE_COLOR,        0 },
    { 0, offsetof(Vertex3D, specular), D3DDECLTYPE_D3DCOLOR,  0, D3DDECLUSAGE_COLOR,        1 },
    { 0, offsetof(Vertex3D, texCoord), D3DDECLTYPE_FLOAT2,    0, D3DDECLUSAGE_TEXCOORD,     0 },
	{ 0, offsetof(Vertex3D, boneIndices), D3DDECLTYPE_FLOAT4, 0, D3DDECLUSAGE_BLENDINDICES, 0 },
    { 0, offsetof(Vertex3D, boneWeights), D3DDECLTYPE_FLOAT4, 0, D3DDECLUSAGE_BLENDWEIGHT,  0 },
    D3DDECL_END(),
};

#endif

struct Wave
{
	Wave() : direction(D3DXVECTOR3(0,0,0)), amplitude(0), speed(0), frequency(0), steepness(0)
	{
	}

	Wave(D3DXVECTOR3 waveDirection, float waveAmplitude, float waveSpeed, float waveFrequency, float waveSteepness) :
		direction(waveDirection), amplitude(waveAmplitude), speed(waveSpeed), frequency(waveFrequency), steepness(waveSteepness)
	{
	}

	D3DXVECTOR3 direction;	// wave direction. Y component must be 0
	float amplitude;	// wave amplitude in meters
	float speed;		// wavefront speed in m/s
	float frequency;    // it is = 2.0f*D3DX_PI / wavelength
	float steepness;	// 0..1. Where 0 - round, 1 choppy
};


/**
	Misc common helper functions
*/
class Utilities
{
public:
	static HRESULT SetVertexShaderTransforms(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX& world, const D3DXMATRIX& viewProj);
    static HRESULT SetVertexShaderBoneTransforms(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX* matrixPalette, size_t matrixPaletteSize, BOOL useSkinning = TRUE);
	static HRESULT SetVertexShaderConstants(IDirect3DDevice9* pd3dDevice,  const D3DXVECTOR4& eyePt, const D3DXVECTOR4& lightDir);
    static HRESULT SetVertexShaderWaveConstants(IDirect3DDevice9* pd3dDevice, const Wave waves[4], float time, float sqVisibility);
	static HRESULT SetPixelShaderConstants(IDirect3DDevice9* pd3dDevice, const D3DXCOLOR& ambientColor, const D3DXCOLOR& directionalLightColor, const D3DXVECTOR4& lightAmbDiffSpecPower);
	static HRESULT LoadXModel(IDirect3DDevice9* pd3dDevice, const LPCWSTR szXFilePath, 	LPD3DXMESH* ppMesh, LPDIRECT3DTEXTURE9** ppTextures, DWORD* pNumberOfMaterials);
	static HRESULT CreateMesh(IDirect3DDevice9* pd3dDevice, Vertex3D* pVertices, DWORD numberOfVertices, DWORD* pMeshFaces, DWORD numberOfFaces, ID3DXMesh** ppMesh);
	static HRESULT FlipNormals(ID3DXMesh* pMesh, int subsetIndex = -1);
	static HRESULT DrawLineSegment(IDirect3DDevice9* pd3dDevice, const D3DXVECTOR3& point1, const D3DXVECTOR3& point2, const D3DCOLOR& color);
	static bool IsPointInsideFace(const D3DXVECTOR3* pPoint, const D3DXVECTOR3* pPt1, const D3DXVECTOR3* pPt2, const D3DXVECTOR3* pPt3, const D3DXVECTOR3* pPt4 = NULL);
	static float ComputePointToPlaneDistance(const D3DXVECTOR3* pPoint, const D3DXVECTOR3* pPt1, const D3DXVECTOR3* pPt2, const D3DXVECTOR3* pPt3);
	static void CreateWorldMatrix(D3DXMATRIX* pMatrix, const D3DXVECTOR3& up, const D3DXVECTOR3& direction, const D3DXVECTOR3& translation);
	
	static float NormalizeAngleInRadians(float angle);

	static inline float RandF()
	{
		return float(rand())/float(RAND_MAX);
	}
};
