
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
#include "DebugCamera.h"

CDebugCamera::CDebugCamera()
{
}

CDebugCamera::~CDebugCamera()
{
}

LRESULT CDebugCamera::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_KEYDOWN:
        {
            // Map this key to a DebugCameraKeys enum and update the
            // state of m_cameraKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
            // only if the key is not down
            DebugCameraKeys mappedKey = MapDebugCameraKey( (UINT)wParam );
            if( mappedKey != DEBUG_CAM_UNKNOWN && (DWORD)mappedKey < DEBUG_CAM_MAX_KEYS )
            {
                if( FALSE == IsKeyDown(m_aKeys[mappedKey]) )
                {
                    m_aKeys[ mappedKey ] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
                    ++m_cKeysDown;
                }
            }
            break;
        }

        case WM_KEYUP:
        {
            // Map this key to a D3DUtil_CameraKeys enum and update the
            // state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
            DebugCameraKeys mappedKey = MapDebugCameraKey( (UINT)wParam );
            if( mappedKey != DEBUG_CAM_UNKNOWN && (DWORD)mappedKey < DEBUG_CAM_MAX_KEYS )
            {
                m_aKeys[ mappedKey ] &= ~KEY_IS_DOWN_MASK;
                --m_cKeysDown;
            }
            break;
        }
    }

    return FALSE;
}

void CDebugCamera::FrameMove(float fElapsedTime)
{
	if( DXUTGetGlobalTimer()->IsStopped() )
	{
        fElapsedTime = 1.0f / DXUTGetFPS();
	}

	/* We can also define RESET key and reset camera like this (it was not specified in the HW assignment so is left commented):
    if( IsKeyDown(m_aKeys[DEBUG_CAM_RESET]) )
	{
        Reset();
	}
	*/

    // Process keyboard input
	m_vKeyboardDirection = D3DXVECTOR3(0,0,0);
	m_vMouseDelta = D3DXVECTOR2(0,0);

    if( IsKeyDown(m_aKeys[DEBUG_CAM_MOVE_FORWARD]) )
	{
        m_vKeyboardDirection.z += 1.0f;
	}
    
	if( IsKeyDown(m_aKeys[DEBUG_CAM_MOVE_BACKWARD]) )
	{
        m_vKeyboardDirection.z -= 1.0f;
	}

    if( IsKeyDown(m_aKeys[DEBUG_CAM_MOVE_UP]) )
	{
        m_vKeyboardDirection.y += 1.0f;
	}

    if( IsKeyDown(m_aKeys[DEBUG_CAM_MOVE_DOWN]) )
	{
        m_vKeyboardDirection.y -= 1.0f;
	}

	// We can also directly use - m_vRotVelocity for rotations instead of m_vMouseDelta
	// but in this case we cannot reuse selftuning rotation speed calculations that are done in
	// UpdateVelocity() below which is defined in CBaseCamera

    if( IsKeyDown(m_aKeys[DEBUG_CAM_ROTATE_LEFT]) )
	{
		m_vMouseDelta.x -= 1.0f;
	}

    if( IsKeyDown(m_aKeys[DEBUG_CAM_ROTATE_RIGHT]) )
	{
		m_vMouseDelta.x += 1.0f;
	}

	if( IsKeyDown(m_aKeys[DEBUG_CAM_PITCH_UP]) )
	{
		m_vMouseDelta.y -= 1.0f;
	}

    if( IsKeyDown(m_aKeys[DEBUG_CAM_PITCH_DOWN]) )
	{
		m_vMouseDelta.y += 1.0f;
	}

	// Get amount of velocity based on the keyboard input and drag (if any)
    UpdateVelocity( fElapsedTime );

	// Simple euler method to calculate position delta
    D3DXVECTOR3 vPosDelta = m_vVelocity * fElapsedTime;

    // Update the pitch & yaw angle based on mouse movement
    m_fCameraPitchAngle += m_vRotVelocity.y;
    m_fCameraYawAngle   += m_vRotVelocity.x;

    // Limit pitch to straight up or straight down
    m_fCameraPitchAngle = __max( -D3DX_PI/2.0f,  m_fCameraPitchAngle );
    m_fCameraPitchAngle = __min( +D3DX_PI/2.0f,  m_fCameraPitchAngle );

    // Make a rotation matrix based on the camera's yaw & pitch
    D3DXMATRIX mCameraRot;
    D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );

    // Transform vectors based on camera's rotation matrix
    D3DXVECTOR3 vWorldUp, vWorldAhead;
    D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
    D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
    D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &mCameraRot );
    D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &mCameraRot );

    // Move the eye position 
	D3DXVECTOR3 vPosDeltaWorld;
	D3DXMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, 0.0f, 0.0f ); // we restrict forward and backward movement in X and Z axes only (Y does not change)
	D3DXVec3TransformCoord( &vPosDeltaWorld, &vPosDelta, &mCameraRot );
	m_vEye += vPosDeltaWorld;
	
	if( m_bClipToBoundary )
	{
        ConstrainToBoundary( &m_vEye );
	}

    // Update the lookAt position based on the eye position 
    m_vLookAt = m_vEye + vWorldAhead;

    // Update the view matrix
    D3DXMatrixLookAtLH( &m_mView, &m_vEye, &m_vLookAt, &vWorldUp );

	// Invert the view matrix to get the camera matrix
    D3DXMatrixInverse( &m_mCameraWorld, NULL, &m_mView );
}

DebugCameraKeys CDebugCamera::MapDebugCameraKey( UINT nKey )
{
    switch( nKey )
    {
        case VK_LEFT:  return DEBUG_CAM_ROTATE_LEFT;
        case VK_RIGHT: return DEBUG_CAM_ROTATE_RIGHT;
        case VK_UP:    return DEBUG_CAM_MOVE_FORWARD;
        case VK_DOWN:  return DEBUG_CAM_MOVE_BACKWARD;
        case VK_PRIOR: return DEBUG_CAM_PITCH_UP;       // pgup
        case VK_NEXT:  return DEBUG_CAM_PITCH_DOWN;		// pgdn

        case 'A':      return DEBUG_CAM_MOVE_UP;
        case 'Z':      return DEBUG_CAM_MOVE_DOWN;

        // NOT used for the moment: case VK_HOME:   return DEBUG_CAM_RESET;
    }

    return DEBUG_CAM_UNKNOWN;
}