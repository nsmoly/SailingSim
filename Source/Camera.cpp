#include "DXUT.h"
#if 0
#include "Camera.h"

////////////////////////////////////////////////////////////////////////////

//D3DXVECTOR3	Camera::vUp(D3DXVECTOR3(0,1,0));

Camera::Camera()
{
#if 0
	// Default location is at the origin, pointing down the +Z axis
	vPos = D3DXVECTOR3(0,0,0);
	vDir = D3DXVECTOR3(0,0,1);

	// Default projection is:
	//   45 degree FOV
	//   square aspect ratio (1.0)
	//   near clip = 0.1
	//   far clip = 10000.0
	D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(45), 1.0f, 0.1f, 10000.0f);

	// Make sure the view matrix is valid
	Update(0.f);
#endif
}


void Camera::HandleMessages(HWND /* hWnd */, UINT uMsg, WPARAM wParam, LPARAM /* lParam */)
{
    switch( uMsg )
    {
	case WM_KEYDOWN:
        // Map this key to a D3DUtil_CameraKeys enum and update the
        // state of m_aKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
        // only if the key is not down
        unsigned char mappedKey = MapKey( (UINT)wParam );
        if( mappedKey != CAM_UNKNOWN )
        {
            if( FALSE == IsKeyDown(m_aKeys[mappedKey]) )
            {
                m_aKeys[ mappedKey ] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
                ++m_cKeysDown;
            }
        }
        break;
	case WM_KEYUP:
        // Map this key to a D3DUtil_CameraKeys enum and update the
        // state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
        D3DUtil_CameraKeys mappedKey = MapKey( (UINT)wParam );
        if( mappedKey != CAM_UNKNOWN && (DWORD)mappedKey < 8 )
        {
            m_aKeys[ mappedKey ] &= ~KEY_IS_DOWN_MASK;
            --m_cKeysDown;
        }
        break;
}


void
Camera::Update(float fElapsedTime)
{
	D3DXVECTOR3	vAt = vPos + vDir;
	D3DXMatrixLookAtLH(&matView, &vPos, &vAt, &vUp);
}


////////////////////////////////////////////////////////////////////////////
#endif