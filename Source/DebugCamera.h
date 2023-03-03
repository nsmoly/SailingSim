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

#include "DXUT/DXUTcamera.h"

enum DebugCameraKeys
{
    DEBUG_CAM_ROTATE_LEFT = 0,
	DEBUG_CAM_ROTATE_RIGHT,
	DEBUG_CAM_MOVE_FORWARD,
	DEBUG_CAM_MOVE_BACKWARD,
	DEBUG_CAM_PITCH_UP,
	DEBUG_CAM_PITCH_DOWN,
	DEBUG_CAM_MOVE_UP,
	DEBUG_CAM_MOVE_DOWN,
	DEBUG_CAM_RESET,
	DEBUG_CAM_MAX_KEYS,
	DEBUG_CAM_UNKNOWN = 0xFF
};

class CDebugCamera : public CBaseCamera
{
public:
	CDebugCamera();
	virtual ~CDebugCamera();

	virtual LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void FrameMove(FLOAT fElapsedTime);

	// Functions to get state
	D3DXMATRIX* GetWorldMatrix() { return &m_mCameraWorld; }
	const D3DXVECTOR3* GetWorldRight() const { return (D3DXVECTOR3*)&m_mCameraWorld._11; }
	const D3DXVECTOR3* GetWorldUp() const { return (D3DXVECTOR3*)&m_mCameraWorld._21; }
	const D3DXVECTOR3* GetWorldAhead() const { return (D3DXVECTOR3*)&m_mCameraWorld._31; }
	const D3DXVECTOR3* GetEyePt() const { return (D3DXVECTOR3*)&m_mCameraWorld._41; }

private:
	DebugCameraKeys MapDebugCameraKey( UINT nKey );

private:
	D3DXMATRIX m_mCameraWorld;			// World matrix of the camera (inverse of the view matrix)
	BYTE m_aKeys[DEBUG_CAM_MAX_KEYS];	// State of control keys
};
