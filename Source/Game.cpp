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
#pragma warning(disable: 4995)

#define GAME_CPP

#include <time.h>

#include "VertexShader.vfxobj"
#include "WaveVertexShader.vfxobj"
#include "PixelShader.pfxobj"
#include "WavePixelShader.pfxobj"
#include "PixelShaderTexture.pfxobj"

#include "resource.h"
#include "DXUT\DXUTcamera.h"
#include "DXUT\DXUTsettingsdlg.h"
#include "DXUT\SDKmisc.h"
#include "DXUT\SDKsound.h"

#include "Game.h"
#include "FSM.h"
#include "World.h"
#include "Utilities.h"
#include "Scene.h"
#include "Land.h"
#include "Ocean.h"
#include "Ship.h"
#include "DebugCamera.h"
#include "MiniMap.h"

using namespace std;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
CFirstPersonCamera      g_Camera;               // A first person camera
//CDebugCamera			g_Camera;               // A debug camera
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
CDXUTDialog             g_HUD;                  // dialog for standard controls
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls
bool                    g_bShowHelp = true;     // If true, it renders the UI control text
bool                    g_bPlaySounds = true;   // whether to play sounds
double                  g_fLastAnimTime = 0.0;  // Time for the animations
//World					g_World;				// World for creating singletons and objects

std::tr1::shared_ptr<Node>		g_scene;
std::tr1::shared_ptr<Land>		g_worldEnvironment;
std::tr1::shared_ptr<Ocean>		g_ocean;
std::tr1::shared_ptr<Ship>		g_playerShip;
std::tr1::shared_ptr<MiniMap>	g_miniMap;
std::tr1::shared_ptr<FSMManager> g_fsmManager;
std::list<D3DXVECTOR3>			g_racePoints;

D3DXVECTOR3				g_playerShipOldPosition = D3DXVECTOR3(SHIP_LENGTH, -SHIP_LENGTH/2.0, SHIP_LENGTH);
wchar_t					g_dataPath[] = L"Data";
bool					g_showWireFrame = false;
bool					g_isInDebugging = false;

// Sound related stuff
CSoundManager*			g_pSoundManager = NULL;
CSound*					g_pAmbientOceanSound = NULL;
CSound*					g_pShipCollisionSound = NULL;

// Shaders
IDirect3DVertexShader9* g_pVertexShader = NULL;
IDirect3DVertexShader9* g_pWaveVertexShader = NULL;
IDirect3DPixelShader9*  g_pWavePixelShader = NULL;
IDirect3DPixelShader9*  g_pPixelShader = NULL;
IDirect3DPixelShader9*  g_pPixelShaderTexture = NULL;

// Control key handling
BYTE g_controlKeys[GAME_MAX_KEYS];	// State of control keys
bool IsControlKeyDown( GameControlKeys key ) { return( (g_controlKeys[key] & KEY_IS_DOWN_MASK) == KEY_IS_DOWN_MASK ); }

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_TOGGLEWIREFRAME     5
#define IDC_TOGGLEDEBUG		    6

#define IDC_NEXTVIEW            9
#define IDC_PREVVIEW            10
#define IDC_RESETCAMERA         11
#define IDC_RESETTIME           12


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------

void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );
void    RenderText();
void DetectCollisions(double fTime);

//--------------------------------------------------------------------------------------
// Initialize the app
//--------------------------------------------------------------------------------------
bool InitApp()
{
	// Initialize random seed
	srand( (unsigned int)time(NULL) );

    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F2)", 35, iY += 24, 125, 22, VK_F2 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F3)", 35, iY += 24, 125, 22, VK_F3 );
	g_HUD.AddButton( IDC_TOGGLEWIREFRAME, L"Toggle wireframe (F4)", 35, iY += 24, 125, 22, VK_F4 );
	g_HUD.AddButton( IDC_TOGGLEDEBUG, L"Toggle debugging (F5)", 35, iY += 24, 125, 22, VK_F5 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
    g_SampleUI.AddButton( IDC_NEXTVIEW, L"(N)ext View", 45, iY += 26, 120, 24, L'N' );
    g_SampleUI.AddButton( IDC_PREVVIEW, L"(P)revious View", 45, iY += 26, 120, 24, L'P' );
    g_SampleUI.AddButton( IDC_RESETCAMERA, L"(R)eset view", 45, iY += 26, 120, 24, L'R' );
    //g_SampleUI.AddButton( IDC_RESETTIME, L"Reset time", 45, iY += 26, 120, 24 );

    // Add mixed vp to the available vp choices in device settings dialog.
    DXUTGetD3D9Enumeration()->SetPossibleVertexProcessingList( true, false, false, true );
    
    // Setup the camera with view matrix
    D3DXVECTOR3 vEye(0.0f, SHIP_LENGTH, 0.0f);
    D3DXVECTOR3 vAt(SHIP_LENGTH, 0.0f, SHIP_LENGTH);
    g_Camera.SetViewParams( &vEye, &vAt );
    g_Camera.SetScalers( 0.02f, (float)max(WORLD_SIZE_X, WORLD_SIZE_Z) / 50.0f );  // Camera movement parameters

	//g_World.InitializeSingletons();

	g_racePoints.push_back(D3DXVECTOR3(50,0,50));
	g_racePoints.push_back(D3DXVECTOR3(380,0,100));
	g_racePoints.push_back(D3DXVECTOR3(480,0,900));
	g_racePoints.push_back(D3DXVECTOR3(280,0,920));
	g_racePoints.push_back(D3DXVECTOR3(270,0,780));
	g_racePoints.push_back(D3DXVECTOR3(270,0,780));
	g_racePoints.push_back(D3DXVECTOR3(540,0,510));
	g_racePoints.push_back(D3DXVECTOR3(670,0,500));
	g_racePoints.push_back(D3DXVECTOR3(850,0,800));
	g_racePoints.push_back(D3DXVECTOR3(500,0,700));
	g_racePoints.push_back(D3DXVECTOR3(380,0,110));
	g_racePoints.push_back(D3DXVECTOR3(50,0,50));

	return true;
}

//--------------------------------------------------------------------------------------
// Clean up the app
//--------------------------------------------------------------------------------------
void CleanupApp()
{
	// Do any sort of app cleanup here 
	g_playerShip.reset();
	g_worldEnvironment.reset();
	g_ocean.reset();
	g_fsmManager.reset();
	g_scene.reset();
	g_miniMap.reset();

	if(g_pAmbientOceanSound)
	{
		g_pAmbientOceanSound->Stop();
		SAFE_DELETE(g_pAmbientOceanSound);
	}

	if(g_pShipCollisionSound)
	{
		g_pShipCollisionSound->Stop();
		SAFE_DELETE(g_pShipCollisionSound);
	}

	SAFE_DELETE(g_pSoundManager);
}

bool InitShaders(IDirect3DDevice9* pd3dDevice)
{
    if(FAILED(pd3dDevice->CreateVertexShader((DWORD const*)VFX_VertexShader, &g_pVertexShader)))
    {
        OutputDebugStringA("Cannot create vertex shader!");
        return false;
    }

    if(FAILED(pd3dDevice->CreateVertexShader((DWORD const*)VFX_WaveVertexShader, &g_pWaveVertexShader)))
    {
        OutputDebugStringA("Cannot create wave vertex shader!");
        return false;
    }

    if(FAILED(pd3dDevice->CreatePixelShader((DWORD const*)PFX_PixelShader, &g_pPixelShader)))
    {
        OutputDebugStringA("Cannot create simple pixel shader!");
        return false;
    }

    if(FAILED(pd3dDevice->CreatePixelShader((DWORD const*)PFX_WavePixelShader, &g_pWavePixelShader)))
    {
        OutputDebugStringA("Cannot create wave pixel shader!");
        return false;
    }

    if(FAILED(pd3dDevice->CreatePixelShader((DWORD const*)PFX_PixelShaderTexture, &g_pPixelShaderTexture)))
    {
        OutputDebugStringA("Cannot create pixel shader that supports textures!");
        return false;
    }

    return true;
}

bool InitVertexDeclarations(IDirect3DDevice9* pd3dDevice)
{
    if(FAILED(pd3dDevice->CreateVertexDeclaration(Vertex3D::declaration, &Vertex3D::pVD)))
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
	{
        return false;
	}

    // Need to support ps 1.1
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 1, 1 ) )
	{
        return false;
	}

    // Need to support A8R8G8B8 textures
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, 0,
                                         D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8 ) ) )
	{
        return false;
	}

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the
// application to modify the device settings. The supplied pDeviceSettings parameter
// contains the settings that the framework has selected for the new device, and the
// application can make any desired changes directly to this structure.  Note however that
// DXUT will not correct invalid device settings so care must be taken
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW
    // then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // If the hardware cannot do vertex blending, use software vertex processing.
    if( pCaps->MaxVertexBlendMatrices < 2 )
	{
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

    // If using hardware vertex processing, change to mixed vertex processing
    // so there is a fallback.
    if( pDeviceSettings->d3d9.BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
	{
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
	}

    // Debugging vertex shaders requires either REF or software vertex processing
    // and debugging pixel shaders requires REF.
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning(  pDeviceSettings->ver  );
    }

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been
// created, which will happen during application initialization and windowed/full screen
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these
// resources need to be reloaded whenever the device is destroyed. Resources created
// here should be released in the OnDestroyDevice callback.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                         L"Arial", &g_pFont ) );

    // Load shaders and initialize vertex declarations
    if(!InitVertexDeclarations(pd3dDevice) || !InitShaders(pd3dDevice))
    {
        return E_FAIL;
    }

	// Initialize sound manager and load sounds
	SAFE_DELETE(g_pSoundManager);
	g_pSoundManager = new CSoundManager();
	HWND hWnd = DXUTGetHWND();
	if( !g_pSoundManager || FAILED(g_pSoundManager->Initialize(hWnd, DSSCL_PRIORITY)) )
	{
		SAFE_DELETE(g_pSoundManager);
	}
	else
	{
		// Set stereo, 22kHz, 16-bit output.
		g_pSoundManager->SetPrimaryBufferFormat(2, 22050, 16);

		// Load world sounds
		wstring ambientOceanSoundFileName(g_dataPath);
		ambientOceanSoundFileName += L"\\sounds\\oceansounds.wav";
		if( FAILED(g_pSoundManager->Create(&g_pAmbientOceanSound, LPWSTR(ambientOceanSoundFileName.c_str()), 0, GUID_NULL)) )
		{
			g_pAmbientOceanSound = NULL;
		}
		else
		{
			g_pAmbientOceanSound->Play( 0, DSBPLAY_LOOPING );
		}

		// Load ship sounds
		wstring shipCollisionFileName(g_dataPath);
		shipCollisionFileName += L"\\sounds\\shipcollision.wav";
		if( FAILED(g_pSoundManager->Create(&g_pShipCollisionSound, LPWSTR(shipCollisionFileName.c_str()), 0, GUID_NULL)) )
		{
			g_pShipCollisionSound = NULL;
		}
	}

	// Create the world
	g_fsmManager = std::tr1::shared_ptr<FSMManager>(new FSMManager());

	g_scene = std::tr1::shared_ptr<Scene>(new Scene(pd3dDevice));
	if( FAILED(g_scene->Load(pd3dDevice, g_dataPath)) )
	{
		OutputDebugString(L"Failed to create the scene\n");
		return E_FAIL;
	}

	// Create the minimap
	g_miniMap = std::tr1::shared_ptr<MiniMap>(new MiniMap(pd3dDevice, g_worldEnvironment->GetLevelDescription()));

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been
// reset, which will happen after a lost device scenario. This is the best location to
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever
// the device is lost. Resources created here should be released in the OnLostDevice
// callback.
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

	//g_World.Initialize(pd3dDevice);

	// get device caps
    D3DCAPS9 caps;
    pd3dDevice->GetDeviceCaps( & caps );

    if( g_pFont )
	{
        V_RETURN( g_pFont->OnResetDevice() );
	}

	// Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/3, fAspectRatio, 0.1f, sqrt(WORLD_SIZE_X*WORLD_SIZE_X+WORLD_SIZE_Z*WORLD_SIZE_Z) );

	// Set rendering states
	pd3dDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);
    pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE );

    // reset the timer
    g_fLastAnimTime = DXUTGetGlobalTimer()->GetTime();

    // Adjust the dialog parameters.
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-170, pBackBufferSurfaceDesc->Height-270 );
    g_SampleUI.SetSize( 170, 220 );

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not
// intended to contain actual rendering calls, which should instead be placed in the
// OnFrameRender callback.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_fLastAnimTime = fTime;

	// Detect world collisions
	DetectCollisions(fTime);

	// Update all game objects
	g_scene->Update(fTime);

	// Drive AI
	g_fsmManager->ProcessEvents(fTime);

	// TODO: should we do this? We kind of have all the game logic in the scene graph update...
	//g_World.Update();

    // Update the camera's position based on user input
	if(	g_playerShip )
	{
 		D3DXVECTOR3 shipPosition = g_playerShip->GetPosition();
		D3DXVECTOR3 shipMove = shipPosition - g_playerShipOldPosition;

		D3DXVECTOR3 cameraEyePoint(*g_Camera.GetEyePt());
		D3DXVECTOR3 cameraLookAtPoint(*g_Camera.GetLookAtPt());
		cameraEyePoint += shipMove;
		cameraLookAtPoint += shipMove;

		if(!g_isInDebugging)
		{
			g_Camera.SetViewParams(&cameraEyePoint, &cameraLookAtPoint);
		}

		g_playerShipOldPosition = shipPosition;
	}

    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the
// rendering calls for the scene, and it will also be called if the window needs to be
// repainted. After this function has returned, DXUT will call
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    HRESULT hr;

    pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                       D3DCOLOR_ARGB( 0, 0x3F, 0xAF, 0xFF ), 1.0f, 0L );

    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

        D3DXVECTOR3 eyePt = *g_Camera.GetEyePt();
        D3DXMATRIX view = *g_Camera.GetViewMatrix();
        D3DXMATRIX proj = *g_Camera.GetProjMatrix();
        D3DXMATRIX world;
        D3DXMatrixIdentity(&world);
        D3DXMATRIX viewProj = view * proj;

		// Set common shader constants for this scene
		Utilities::SetVertexShaderConstants(pd3dDevice, 
			D3DXVECTOR4(eyePt, 1), /* eye position */
			//D3DXVECTOR4(0.57f, -0.57f, 0.57f, 0.0f) /* light direction, from sun to the surface */
			D3DXVECTOR4(-0.57f, -0.57f, -0.57f, 0.0f) /* light direction, from sun to the surface */
		);

		Utilities::SetPixelShaderConstants(pd3dDevice, 
			D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),	/* ambient color */
			D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),	/* directional light color - controls dominant color of light that lights the scene */
			D3DXVECTOR4(0.6f, 0.6f, 0.8f, 8.0f) /* directional light color intensities: x=ambient, y=diffuse, z=specular, w=specular exponent, they control how light contributes to different components */
		);

        // Setup shaders
        pd3dDevice->SetVertexShader(g_pVertexShader);
        pd3dDevice->SetVertexDeclaration(Vertex3D::pVD);
		pd3dDevice->SetPixelShader(g_pPixelShaderTexture);
		
        // Render the scene graph
		DWORD originalFillMode;
		pd3dDevice->GetRenderState( D3DRS_FILLMODE, &originalFillMode);
		if(g_showWireFrame)
		{
			pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			g_scene->Render(pd3dDevice, viewProj);
		}
		else
		{
			pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
			g_scene->Render(pd3dDevice, viewProj);
		}
		
		pd3dDevice->SetRenderState( D3DRS_FILLMODE, originalFillMode);
		pd3dDevice->SetPixelShader(NULL);
		pd3dDevice->SetVertexShader(NULL);

		// Render the minimap
		if(g_miniMap)
		{
			g_miniMap->Render(pd3dDevice, viewProj);
		}

		// Output text information
		RenderText();

        V( g_HUD.OnRender( fElapsedTime ) );
		
        pd3dDevice->EndScene();
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();

    // Output statistics
    txtHelper.Begin();
    
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    
	// Write out the FPS and device stats
	txtHelper.SetInsertionPos( 5, 5 );
	txtHelper.DrawTextLine( DXUTGetFrameStats(true) );
    //txtHelper.DrawTextLine( DXUTGetDeviceStats() );
    //txtHelper.DrawFormattedTextLine( L"Time: %2.3f", DXUTGetGlobalTimer()->GetTime() );
    //txtHelper.DrawFormattedTextLine( L"Number of models: %d", g_v_pCharacters.size() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.SetInsertionPos( 5, 20 );
    txtHelper.DrawFormattedTextLine(L"Controls: Camera - A,D,W,S, mouse+leftMouseButton; Sails - C,V; Rudder - Z,X");

	if(g_isInDebugging)
	{
		txtHelper.DrawFormattedTextLine(L"Eye = (%.2f,%.2f,%.2f)", g_Camera.GetEyePt()->x, g_Camera.GetEyePt()->y, g_Camera.GetEyePt()->z);
		txtHelper.DrawFormattedTextLine(L"LookAt = (%.2f,%.2f,%.2f)", g_Camera.GetLookAtPt()->x, g_Camera.GetLookAtPt()->y, g_Camera.GetLookAtPt()->z);
	}

	if(g_playerShip)
	{
		txtHelper.DrawFormattedTextLine(L"Ship's speed = %.2f", g_playerShip->GetSpeed());
		txtHelper.DrawFormattedTextLine(L"Ship's direction = %.2f", D3DXToDegree(g_playerShip->GetShipToXAxesAngle()));

		if(g_isInDebugging)
		{
			D3DXVECTOR3 position = g_playerShip->GetPosition();
			txtHelper.DrawFormattedTextLine(L"Ship's position = (%.2f,%.2f,%.2f)", position.x, position.y, position.z);
			txtHelper.DrawFormattedTextLine(L"Sails lift force = %.2f", D3DXToDegree(g_playerShip->GetSailsLiftForce()));
    		txtHelper.DrawFormattedTextLine(L"Sails push force = %.2f", D3DXToDegree(g_playerShip->GetSailsPushForce()));
		}
	}

    if( g_bShowHelp )
    {
    }

    txtHelper.End();
}

void HandleControlMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
GameControlKeys MapGameControlKey( UINT nKey );

GameControlKeys MapGameControlKey( UINT nKey )
{
    switch( nKey )
    {
        case 'Z': 
			return TEELER_LEFT;
        case 'X': 
			return TEELER_RIGHT;
        case 'C': 
			return SAILS_LEFT;
        case 'V': 
			return SAILS_RIGHT;

        // NOT used for the moment: case VK_HOME:   return DEBUG_CAM_RESET;
    }

    return KEY_UNKNOWN;
}

void HandleControlMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_KEYDOWN:
        {
            // Map this key to a DebugCameraKeys enum and update the
            // state of m_cameraKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
            // only if the key is not down
            GameControlKeys mappedKey = MapGameControlKey( (UINT)wParam );
            if( mappedKey != KEY_UNKNOWN && (DWORD)mappedKey < GAME_MAX_KEYS )
            {
                if( FALSE == IsControlKeyDown(mappedKey) )
                {
                    g_controlKeys[ mappedKey ] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
                    //++m_cKeysDown; -- do we need this?
                }
            }
            break;
        }

        case WM_KEYUP:
        {
            // Map this key to a D3DUtil_CameraKeys enum and update the
            // state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
            GameControlKeys mappedKey = MapGameControlKey( (UINT)wParam );
            if( mappedKey != KEY_UNKNOWN && (DWORD)mappedKey < GAME_MAX_KEYS )
            {
                g_controlKeys[ mappedKey ] &= ~KEY_IS_DOWN_MASK;
                //--m_cKeysDown; -- do we need this?
            }
            break;
        }
    }
}

//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows
// messages to the application through this callback function. If the application sets
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
	{
        return 0;
	}

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
	{
        return 0;
	}

    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
	{
        return 0;
	}

    // Pass messages to camera class for camera movement if the
    // global camera is active
    //if( -1 == g_dwFollow )
        g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	// Process game controls
	HandleControlMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}

//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
				g_bShowHelp = !g_bShowHelp;
				break;

        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;

        case IDC_NEXTVIEW:
            break;

        case IDC_PREVVIEW:
            break;

        case IDC_RESETCAMERA:
            break;

        case IDC_RESETTIME:
            DXUTGetGlobalTimer()->Reset();
            g_fLastAnimTime = DXUTGetGlobalTimer()->GetTime();
            break;

		case IDC_TOGGLEWIREFRAME:
			g_showWireFrame = !g_showWireFrame;
			break;

		case IDC_TOGGLEDEBUG:
			g_isInDebugging = !g_isInDebugging;
			break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont )
	{
        g_pFont->OnLostDevice();
	}

    SAFE_RELEASE( g_pTextSprite );
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has
// been destroyed, which generally happens as a result of application termination or
// windowed/full screen toggles. Resources created in the OnCreateDevice callback
// should be released here, which generally includes all D3DPOOL_MANAGED resources.
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();

    SAFE_RELEASE(g_pFont);
	SAFE_RELEASE(Vertex3D::pVD);
	SAFE_RELEASE(g_pVertexShader);
    SAFE_RELEASE(g_pWaveVertexShader);
    SAFE_RELEASE(g_pWavePixelShader);
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pPixelShaderTexture);
}

//--------------------------------------------------------------------------------------
// Detects collisions and signals objects if they occur. Scene object then resolve 
// collisions themselves
//--------------------------------------------------------------------------------------
void DetectCollisions(double fTime)
{
	D3DXVECTOR3 shipBoundingBox[4];
	D3DXVECTOR3 otherShipBoundingBox[4];

	for(int i=0; i<g_scene->GetNumChildren(); i++)
	{
		std::tr1::shared_ptr<Node> node = g_scene->GetChild(i);
		
		Ship* pShip = dynamic_cast<Ship*>(node.get());
		if(pShip)
		{
			Ship::CollisionStatus collisionStatus = Ship::NoCollision;
			Ship::CollisionPlace collisionPlace = Ship::NoPlace;

			pShip->GetBoundingBox(shipBoundingBox);

			bool collisionCheckInvolvesPlayableShip = false;
			bool playableShipIsHit = false;

			if(pShip->GetSpeed()>=0)
			{
				if(	g_worldEnvironment->IsOutsideBorders(shipBoundingBox[0]) )
				{
					collisionStatus = Ship::WorldBorderCollision;
					collisionPlace = Ship::ForwardLeftSide;
				}
				else if( g_worldEnvironment->IsOutsideBorders(shipBoundingBox[1]) )
				{
					collisionStatus = Ship::WorldBorderCollision;
					collisionPlace = Ship::ForwardRightSide;
				}
				else if( g_worldEnvironment->IsLand(shipBoundingBox[0]) )
				{
					collisionStatus = Ship::LandCollision;
					collisionPlace = Ship::ForwardLeftSide;
				}
				else if( g_worldEnvironment->IsLand(shipBoundingBox[1]) )
				{
					collisionStatus = Ship::LandCollision;
					collisionPlace = Ship::ForwardRightSide;
				}
			}
			else
			{
				if(	g_worldEnvironment->IsOutsideBorders(shipBoundingBox[2]) )
				{
					collisionStatus = Ship::WorldBorderCollision;
					collisionPlace = Ship::BackLeftSide;
				}
				else if( g_worldEnvironment->IsOutsideBorders(shipBoundingBox[3]) )
				{
					collisionStatus = Ship::WorldBorderCollision;
					collisionPlace = Ship::BackRightSide;
				}
				else if( g_worldEnvironment->IsLand(shipBoundingBox[2]) )
				{
					collisionStatus = Ship::LandCollision;
					collisionPlace = Ship::BackLeftSide;
				}
				else if( g_worldEnvironment->IsLand(shipBoundingBox[3]) )
				{
					collisionStatus = Ship::LandCollision;
					collisionPlace = Ship::BackRightSide;
				}
			}

			if(collisionStatus==Ship::NoCollision)
			{
				// Check for collisions with other ships if it has not collided with the environment
				for(int j=0; j<g_scene->GetNumChildren(); j++)
				{
					std::tr1::shared_ptr<Node> nodeToCheck = g_scene->GetChild(j);
					Ship* pShipToCheck = dynamic_cast<Ship*>(nodeToCheck.get());
					if(pShipToCheck && pShipToCheck!=pShip)
					{
						pShipToCheck->GetBoundingBox(otherShipBoundingBox);

						if(pShip->GetSpeed()>=0)
						{
							if( Utilities::IsPointInsideFace(&shipBoundingBox[0], &otherShipBoundingBox[1], &otherShipBoundingBox[0], &otherShipBoundingBox[2], &otherShipBoundingBox[3]) )
							{
								collisionStatus = Ship::ShipCollision;
								collisionPlace = Ship::ForwardLeftSide;
							}
							else if( Utilities::IsPointInsideFace(&shipBoundingBox[1], &otherShipBoundingBox[1], &otherShipBoundingBox[0], &otherShipBoundingBox[2], &otherShipBoundingBox[3]) )
							{
								collisionStatus = Ship::ShipCollision;
								collisionPlace = Ship::ForwardRightSide;
							}
						}
						else
						{
							if( Utilities::IsPointInsideFace(&shipBoundingBox[2], &otherShipBoundingBox[1], &otherShipBoundingBox[0], &otherShipBoundingBox[2], &otherShipBoundingBox[3]) )
							{
								collisionStatus = Ship::ShipCollision;
								collisionPlace = Ship::BackLeftSide;
							}
							else if( Utilities::IsPointInsideFace(&shipBoundingBox[3], &otherShipBoundingBox[1], &otherShipBoundingBox[0], &otherShipBoundingBox[2], &otherShipBoundingBox[3]) )
							{
								collisionStatus = Ship::ShipCollision;
								collisionPlace = Ship::BackRightSide;
							}
						}

						if(collisionStatus==Ship::ShipCollision && pShipToCheck->IsPlayable())
						{
							playableShipIsHit = true;
						}
					}
				} // for each ship
			}

			if(collisionStatus!=Ship::NoCollision && pShip->IsPlayable())
			{
				playableShipIsHit = true;
			}

			pShip->SetCollisionData(collisionStatus, collisionPlace);

			// Play collision sound if there is a collision
			if(g_pShipCollisionSound && playableShipIsHit && collisionStatus!=Ship::WorldBorderCollision)
			{
				g_pShipCollisionSound->Play(0,0);
			}

		} // if ship

	} // for loop over all scene objects
}


