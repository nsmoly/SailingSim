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
#if 0
#include "DXUT/DXUTcamera.h"

////////////////////////////////////////////////////////////////////////////


#if 0
enum CameraKeys
{
    CAM_LEFT = 0, 
    CAM_RIGHT,
    CAM_UP, 
    CAM_DOWN, 
    CAM_MAX_KEYS,
    CAM_UNKNOWN = 0xFF
};

class Camera
{
public:
					Camera();

	void			HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void			Update(float fElapsedTime);

	D3DXVECTOR3		GetPos()							{return vPos;}
	void			SetPos(D3DXVECTOR3 vPos)			{this->vPos = vPos;}

	D3DXVECTOR3		GetDir()							{return vDir;}
	void			SetDir(D3DXVECTOR3 vDir)			{this->vDir = vDir;}

	D3DXMATRIX*		GetView()							{return &matView;}
	D3DXMATRIX*		GetProjection()						{return &matProj;}

protected:
	D3DXVECTOR3		vPos;
	D3DXVECTOR3		vDir;
	static D3DXVECTOR3	vUp;

	D3DXMATRIX		matView;
	D3DXMATRIX		matProj;

    BYTE            m_aKeys[CAM_MAX_KEYS];  // State of input 
};
#endif

//--------------------------------------------------------------------------------------
// Simple model viewing camera class that rotates around the object.
//--------------------------------------------------------------------------------------
class Camera : public CBaseCamera
{
public:
    Camera();

    // Call these from client and use Get*Matrix() to read new matrices
    virtual LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual void FrameMove( FLOAT fElapsedTime );
};

////////////////////////////////////////////////////////////////////////////
#endif