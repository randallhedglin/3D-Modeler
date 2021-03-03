/*
Define the following before including this file:

#define ID_APPICON		// application icon resource (may be NULL)
#define ID_APPCURSOR	// application cursor resource (may be NULL)
#define ID_APPMENU		// application menu resource (may be NULL)
#define ID_WINDOWTITLE	// title of window
#define ID_WINDOWCLASS	// class name of window

#define APP_HINST		// global HINST to hold application instance
#define APP_HWND		// global HWND to hold main window handle

#define CALL_INIT_PROC  // name of initialization routine (must return BOOL)
#define CALL_MAIN_PROC  // name of main program loop routine (no return)
#define CALL_REST_PROC  // name of resource release routine (no return)

#define ENCODE_DATA		// include data encoding functions (for production purposes)
#define MOUSE_TRACKING  // include mouse tracking functions (for testing purposes)
#define INIT_DIRECT3D   // include Direct3D functions
#define INIT_SOUND      // include sound and music functions

Add the following library files to this project:
ddraw.lib
dinput.lib
dsound.lib
d3dim.lib
d3dxof.lib

winmm.lib (for encoding only - remove from release version)
*/

// DEFINES AND INCLUDES // 

#ifdef INIT_DIRECT3D
#define D3D_OVERLOADS
#endif
#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include<objbase.h>
#include<windows.h>
#include<windowsx.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<ddraw.h>
#include<dinput.h>
#include<mmreg.h>
#include<dsound.h>
#include<wchar.h>
#include<dmdls.h>
#include<dmerror.h>
#include<dmksctrl.h>
#include<dmusici.h>
#include<dmusicc.h>
#include<dmusicf.h>
#ifdef INIT_DIRECT3D
#include<d3d.h>
#include<math.h>
#endif

#define FTD_CAPA 0
#define FTD_LOWA 26
#define FTD_DIGT 52
#define FTD_EXCL 62
#define FTD_LPAR 63
#define FTD_RPAR 64
#define FTD_DASH 65
#define FTD_COLN 66
#define FTD_SEMI 67
#define FTD_QUOT 68
#define FTD_APOS 69
#define FTD_COMA 70
#define FTD_PERD 71
#define FTD_QUES 72

#define DTF_IDNEWFILE 1
#define DTF_IDEOF 2
#define DTF_IDPALETTE 3
#define DTF_IDBITMAP 4
#define DTF_IDFONT 5
#define DTF_IDSOUND 6
#define DTF_IDMUSIC 7
#define DTF_IDRAWDATA 8

#ifdef INIT_SOUND
#define MIDI_TEMP_FILE "~mtf001_.tmp"
#endif

// GLOBALS //

HANDLE DataFile=NULL;
HANDLE VerificationFile=NULL;
LPSTR DefaultFilePath=NULL;
char GlobalStr[256];
WCHAR WideGlobalStr[256];
HGLOBAL JoystickListMem;
LPSTR JoystickList;
DWORD JoystickCount;
BOOL JoystickActive;
#ifdef INIT_DIRECT3D
HGLOBAL Direct3DListMem;
LPSTR Direct3DList;
DWORD Direct3DCount;
DWORD HardwareNum;
#endif
#ifdef MOUSE_TRACKING
	short m_xp;
	short m_yp;
	char m_bt;
#endif
BYTE ActualBitsPerPixel;

// MACROS //

#define INIT_STRUCT(s) ZeroMemory((LPVOID)&s,sizeof(s)); s.dwSize=sizeof(s);
#define SCAN_KEY(k,c) if(KeyState[k]&0x80) { *idstring=c; *keyval=k; return(TRUE); }
#define MULTI_TO_WIDE(x,y) MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,y,-1,x,_MAX_PATH);

// STRUCTURES //

typedef struct TAG_DIRECTDRAWINITSTRUCT
{
	DWORD dwScreenWidth;
	DWORD dwScreenHeight;
	DWORD dwScreenBitsPerPixel;
	DWORD dwBackBufferCount;
} DIRECTDRAWINITSTRUCT,*DIRECTDRAWINITSTRUCT_PTR;

typedef struct TAG_BITMAP_DATA
{
	RECT srcrect;
	LPDIRECTDRAWSURFACE4 bmpsurf;
	DWORD surflock;
} BITMAP_DATA,*BITMAP_DATA_PTR;

typedef struct TAG_FONT_DATA
{
	DWORD width;
	DWORD height;
	BITMAP_DATA bmpdata[73];
} FONT_DATA,*FONT_DATA_PTR;

typedef struct TAG_CONTROL_DATA
{
	DWORD up;
	DWORD down;
	DWORD left;
	DWORD right;
	DWORD btn1;
	DWORD btn2;
	DWORD btn3;
	DWORD btn4;
	DWORD esc;
} CONTROL_DATA,*CONTROL_DATA_PTR;

#ifdef INIT_SOUND
typedef struct TAG_SOUND_DATA
{
	LPDIRECTSOUNDBUFFER sound;
	DWORD SoundLoaded;
} SOUND_DATA,*SOUND_DATA_PTR;

typedef struct TAG_MUSIC_DATA
{
	IDirectMusicSegment* segment;
	IDirectMusicSegmentState* state;
	DWORD SongLoaded;
	DWORD SongPlaying;
} MUSIC_DATA,*MUSIC_DATA_PTR;
#endif

typedef struct TAG_RAW_DATA
{
	HGLOBAL hGlobal;
	LPVOID data;
} RAW_DATA,*RAW_DATA_PTR;

// CLASSES //

class CDirectDraw
{
	public:
		BOOL Initialize(DIRECTDRAWINITSTRUCT_PTR);
		LPDIRECTDRAW4 inline _fastcall GetDirectDrawInterfacePointer(void);
		LPDIRECTDRAWPALETTE inline _fastcall GetDirectDrawPaletteInterfacePointer(void);
		LPDIRECTDRAWSURFACE4 inline _fastcall GetPrimaryDirectDrawSurfaceInterfacePointer(void);
		LPDIRECTDRAWSURFACE4 inline _fastcall GetSecondaryDirectDrawSurfaceInterfacePointer(void);
		LPDIRECTDRAWCLIPPER inline _fastcall GetDirectDrawClipperInterfacePointer(void);
		LPDIRECTDRAWSURFACE4 inline _fastcall GetBitmapSurfaceInterfacePointer(BITMAP_DATA_PTR);
		LPRECT inline _fastcall GetBitmapRect(BITMAP_DATA_PTR);
		BOOL _fastcall LockPrimaryDirectDrawSurface(LPSTR*,LPDWORD);
		BOOL _fastcall UnlockPrimaryDirectDrawSurface(void);
		BOOL _fastcall LockSecondaryDirectDrawSurface(LPSTR*,LPDWORD);
		BOOL _fastcall UnlockSecondaryDirectDrawSurface(void);
		BOOL inline _fastcall PerformPageFlip(void);
		BOOL _fastcall ClearPrimaryDirectDrawSurface(void);
		BOOL _fastcall ClearSecondaryDirectDrawSurface(void);
		BOOL _fastcall SetDirectDrawPaletteEntry(BYTE,BYTE,BYTE,BYTE);
		BOOL _fastcall GetDirectDrawPaletteEntry(BYTE,LPBYTE,LPBYTE,LPBYTE);
		BOOL _fastcall UpdateDirectDrawPalette(DWORD,DWORD);
		BOOL LoadPaletteFromDataFile(void);
		BOOL LoadBitmapFromDataFile(BITMAP_DATA_PTR);
		BOOL ReleaseBitmap(BITMAP_DATA_PTR);
		BOOL _fastcall LockBitmapSurface(BITMAP_DATA_PTR,LPSTR*,LPDWORD);
		BOOL _fastcall UnlockBitmapSurface(BITMAP_DATA_PTR);
		BOOL inline _fastcall IsPrimaryDirectDrawSurfaceLocked(void);
		BOOL inline _fastcall IsSecondaryDirectDrawSurfaceLocked(void);
		BOOL _fastcall DisplayBitmap(BITMAP_DATA_PTR,DWORD,DWORD);
		BOOL _fastcall DisplayBitmap(BITMAP_DATA_PTR,LPRECT);
		BOOL _fastcall DisplayBitmap(BITMAP_DATA_PTR,DWORD,DWORD,DWORD);
		BOOL _fastcall BufferBitmap(BITMAP_DATA_PTR,DWORD,DWORD);
		BOOL _fastcall BufferBitmap(BITMAP_DATA_PTR,LPRECT);
		BOOL _fastcall BufferBitmap(BITMAP_DATA_PTR,DWORD,DWORD,DWORD);
		BOOL LoadFontFromDataFile(FONT_DATA_PTR);
		BOOL ReleaseFont(FONT_DATA_PTR);
		BOOL _fastcall DisplayCharacter(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall DisplayString(FONT_DATA_PTR,char*,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall BufferCharacter(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall BufferString(FONT_DATA_PTR,char*,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall FadePaletteOut(DWORD,DWORD,DWORD,BOOL);
		BOOL _fastcall FadePaletteIn(DWORD,DWORD,DWORD,BOOL);
		BOOL _fastcall LockFontSurface(FONT_DATA_PTR,BYTE,LPSTR*,LPDWORD);
		BOOL _fastcall UnlockFontSurface(FONT_DATA_PTR,BYTE);
		BOOL Release(void);
		BOOL _fastcall ClearPrimaryRect(DWORD,DWORD,DWORD,DWORD);
		BOOL _fastcall ClearSecondaryRect(DWORD,DWORD,DWORD,DWORD);
		BOOL _fastcall SetFontColor(FONT_DATA_PTR,DWORD);
		BOOL _fastcall SetFontColor15(FONT_DATA_PTR,DWORD);
		BOOL _fastcall SetFontColor16(FONT_DATA_PTR,DWORD);
		BOOL _fastcall SetFontColor24(FONT_DATA_PTR,DWORD);
		BOOL _fastcall SetFontColor32(FONT_DATA_PTR,DWORD);
	protected:
		LPDIRECTDRAW4 lpDirectDraw;
		LPPALETTEENTRY Palette;
		HGLOBAL PaletteMem;
		LPPALETTEENTRY SavePalette;
		HGLOBAL SavePaletteMem;
		LPDIRECTDRAWPALETTE lpDirectDrawPalette;
		DDSURFACEDESC2 ddsd;
		LPDIRECTDRAWSURFACE4 lpPrimaryDirectDrawSurface;
		DWORD PrimarySurfaceLocked;
		LPDIRECTDRAWSURFACE4 lpSecondaryDirectDrawSurface;
		DWORD SecondarySurfaceLocked;
		DDBLTFX ddbltfx;
		LPDIRECTDRAWCLIPPER lpDirectDrawClipper;
		LPRGNDATA lpRgnData;
		RECT rect;
		DDCOLORKEY colorkey;
		DDPIXELFORMAT ddpixelformat;

		BOOL LoadPaletteFromDataFile15(void);
		BOOL LoadPaletteFromDataFile16(void);
		BOOL LoadPaletteFromDataFile24(void);
		BOOL LoadPaletteFromDataFile32(void);
		BOOL LoadBitmapFromDataFile15(BITMAP_DATA_PTR);
		BOOL LoadBitmapFromDataFile16(BITMAP_DATA_PTR);
		BOOL LoadBitmapFromDataFile24(BITMAP_DATA_PTR);
		BOOL LoadBitmapFromDataFile32(BITMAP_DATA_PTR);
		BOOL LoadFontFromDataFile15(FONT_DATA_PTR);
		BOOL LoadFontFromDataFile16(FONT_DATA_PTR);
		BOOL LoadFontFromDataFile24(FONT_DATA_PTR);
		BOOL LoadFontFromDataFile32(FONT_DATA_PTR);
		BOOL _fastcall DisplayCharacter15(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall DisplayCharacter16(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall DisplayCharacter24(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall DisplayCharacter32(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall BufferCharacter15(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall BufferCharacter16(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall BufferCharacter24(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
		BOOL _fastcall BufferCharacter32(FONT_DATA_PTR,BYTE,DWORD,DWORD,BYTE,BOOL);
};

///////////////////////////////////////////////////////

class CDirectInput
{
	public:
		BOOL Initialize(CDirectDraw*);
		BOOL inline _fastcall AcquireKeyboard(void);
		BOOL _fastcall GetUserInput(CONTROL_DATA_PTR,CONTROL_DATA_PTR);
		LPDIRECTINPUT inline _fastcall GetDirectInputInterfacePointer(void);
		BOOL _fastcall SetKeyStructure(CONTROL_DATA_PTR);
		BOOL _fastcall GetKeyStructure(CONTROL_DATA_PTR);
		BOOL _fastcall ScanKeyboard(LPSTR*,LPDWORD);
		BOOL DetectJoysticks(void);
		BOOL SelectJoystick(DWORD);
		LPSTR ExtractJoystickName(DWORD);
		BOOL _fastcall SetJoystickButtons(CONTROL_DATA_PTR);
		BOOL _fastcall GetJoystickButtons(CONTROL_DATA_PTR);
		BOOL _fastcall ReadJoystick(void);
		BOOL Release(void);
		BOOL inline _fastcall IsKeyUp(BYTE);
		BOOL inline _fastcall IsKeyDown(BYTE);
	protected:
		CDirectDraw* lpDirectDraw;
		LPDIRECTINPUT lpDirectInput;
		LPDIRECTINPUTDEVICE lpKeyboard;
		CONTROL_DATA ControlKeys;
		CONTROL_DATA KeyData;
		HGLOBAL KeyStateMem;
		LPSTR KeyState;
		LPDIRECTINPUTDEVICE2 lpJoystick;
		DIPROPRANGE JoystickRange;
		DIPROPDWORD JoystickDeadZone;
		CONTROL_DATA JoystickButtons;
		CONTROL_DATA JoystickData;
		DIJOYSTATE JoystickState;
		CONTROL_DATA OldData;

		BOOL inline _fastcall ReadKeyboard(void);
};

///////////////////////////////////////////////////////

#ifdef INIT_SOUND
class CDirectSound
{
	public:
		BOOL Initialize(CDirectDraw*);
		BOOL LoadSoundFromDataFile(SOUND_DATA_PTR);
		BOOL _fastcall Play(SOUND_DATA_PTR,BYTE);
		BOOL _fastcall Stop(SOUND_DATA_PTR);
		BOOL ReleaseSound(SOUND_DATA_PTR);
		BOOL ActivateSoundFX(void);
		BOOL DeactivateSoundFX(void);
		BOOL inline _fastcall IsSoundInitialized(void);
		LPDIRECTSOUND inline _fastcall GetDirectSoundInterfacePointer(void);
		BOOL Release(void);
	protected:
		CDirectDraw *lpDirectDraw;
		LPDIRECTSOUND lpDirectSound;
		BOOL SoundInit;
		BOOL SoundActive;
		DSBUFFERDESC dsbufferdesc;
		WAVEFORMATEX wfx;
};		

///////////////////////////////////////////////////////

class CDirectMusic
{
	public:
		BOOL Initialize(CDirectDraw*,CDirectSound*);
		BOOL LoadMusicFromDataFile(MUSIC_DATA_PTR);
		BOOL ReleaseMusic(MUSIC_DATA_PTR);
		BOOL _fastcall Play(MUSIC_DATA_PTR);
		BOOL _fastcall Stop(MUSIC_DATA_PTR);
		BOOL ActivateMusic(void);
		BOOL DeactivateMusic(void);
		BOOL inline _fastcall IsMusicInitialized(void);
		IDirectMusicPerformance* lpDirectMusic;
		IDirectMusicLoader* lpDirectMusicLoader;
		BOOL _fastcall Update(MUSIC_DATA_PTR);
		BOOL Release(void);
	protected:
		CDirectDraw* lpDirectDraw;
		CDirectSound* lpDirectSound;
		BOOL MusicInit;
		BOOL MusicActive;
		DMUS_OBJECTDESC dmusobjdesc;
		DWORD MIDICount;
};
#endif

///////////////////////////////////////////////////////

#ifdef INIT_DIRECT3D
class CDirect3D
{
	public:
		D3DMATRIX IdentityMatrix;
		
		BOOL Initialize(DIRECTDRAWINITSTRUCT_PTR,CDirectDraw*);
		LPDIRECT3D3 inline _fastcall GetDirect3DInterfacePointer(void);
		BOOL Release(void);
		BOOL inline _fastcall IsHardwareAccelerationActive(void);
		BOOL SelectDirect3DDevice(DWORD);
		LPSTR GetDirect3DDeviceName(DWORD);
		BOOL inline BeginScene(void);
		BOOL inline EndScene(void);
		LPDIRECT3DDEVICE3 inline _fastcall GetDirect3DDeviceInterfacePointer(void);
		BOOL inline _fastcall CreateMatrix(LPD3DMATRIX,float,float,float,float,float,float,BYTE,BYTE,BYTE);
		BOOL _fastcall SetViewMatrix(LPD3DMATRIX);
		BOOL _fastcall SetProjectionMatrix(LPD3DMATRIX);
		BOOL _fastcall SetWorldMatrix(LPD3DMATRIX);
		BOOL _fastcall PositionCamera(float,float,float,float,float,float,BYTE,BYTE,BYTE);
		BOOL _fastcall PositionObject(float,float,float,float,float,float,BYTE,BYTE,BYTE);
		BOOL _fastcall ClearViewport(void);
		BOOL _fastcall DrawPointList(LPVOID,DWORD);
		BOOL _fastcall ProjectCamera(float);
		BOOL _fastcall TransformPoint(LPD3DMATRIX,float,float,float,float*,float*,float*);
	protected:
		CDirectDraw* lpDirectDraw;
		LPDIRECT3D3 lpDirect3D;
		LPDIRECT3DDEVICE3 lpDirect3DDevice;
		BOOL HardwareAccelActive;
		float sintab[256];
		float costab[256];
		D3DMATRIX matrix;
		LPDIRECT3DVIEWPORT3 lpDirect3DViewport;
		D3DVIEWPORT2 d3dviewport;
		D3DRECT ScreenRect;
};
#endif

// MISCELLANEOUS FUNCTION PROTOTYPES //

void DisplayErrorMessage(char*,char*,DWORD,CDirectDraw*);
BOOL OpenDataFile(char*,CDirectDraw*,BOOL);
BOOL CloseDataFile(CDirectDraw*);
BOOL SetDefaultFilePath(char*);
BOOL VerifyDataFiles(char*,CDirectDraw*);
BOOL OpenVerificationFile(char*,CDirectDraw*);
BOOL CloseVerificationFile(CDirectDraw*);
BOOL inline _fastcall Swap(long*,long*);
BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE,LPVOID);
BOOL LoadRawDataFromDataFile(RAW_DATA_PTR,CDirectDraw*);
BOOL ReleaseRawData(RAW_DATA_PTR,CDirectDraw*);
#ifdef INIT_DIRECT3D
	HRESULT WINAPI Enum3DDevices(LPGUID,LPSTR,LPSTR,LPD3DDEVICEDESC,LPD3DDEVICEDESC,LPVOID);
#endif

// REQ'D FUNCTION PROTOTYPES //

BOOL CALL_INIT_PROC(void);
void CALL_MAIN_PROC(void);
void CALL_REST_PROC(void);

// BASIC WINDOWS APP //

HINSTANCE APP_HINST=NULL;
HWND APP_HWND=NULL;

LRESULT CALLBACK WindowProc(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	switch(Msg)
	{
#ifdef MOUSE_TRACKING
	case(WM_MOUSEMOVE):
		m_xp=LOWORD(lParam);
		m_yp=HIWORD(lParam);
		m_bt=wParam&MK_LBUTTON;
		return(0);
		break;
#endif
	case(WM_CLOSE):
		PostMessage(hWnd,WM_DESTROY,NULL,NULL);
		return(0);
		break;
	case(WM_DESTROY):
		PostQuitMessage(NULL);
		return(0);
		break;
	default:
		break;
	}
	return(DefWindowProc(hWnd,Msg,wParam,lParam));
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	WNDCLASSEX WndClassEx;
	MSG Msg;
	
	SetLastError(NULL);
	APP_HWND=NULL;
	APP_HINST=hInstance;
	WndClassEx.cbSize=sizeof(WNDCLASSEX);
	WndClassEx.style=CS_DBLCLKS|CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WndClassEx.lpfnWndProc=WindowProc;
	WndClassEx.cbClsExtra=NULL;
	WndClassEx.cbWndExtra=NULL;
	WndClassEx.hInstance=hInstance;
	if(ID_APPICON)
	{
		WndClassEx.hIcon=LoadIcon(hInstance,ID_APPICON);
		WndClassEx.hIconSm=LoadIcon(hInstance,ID_APPICON);
	}
	else
	{
		WndClassEx.hIcon=LoadIcon(NULL,IDI_APPLICATION);
		WndClassEx.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
	}
	if(ID_APPCURSOR)
		WndClassEx.hCursor=LoadCursor(hInstance,ID_APPCURSOR);
	else
		WndClassEx.hCursor=LoadCursor(NULL,IDC_ARROW);
	WndClassEx.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClassEx.lpszMenuName=ID_APPMENU;
	WndClassEx.lpszClassName=ID_WINDOWCLASS;
	if(!RegisterClassEx(&WndClassEx))
	{
		DisplayErrorMessage("Failed to register window class.\nProgram will now exit.",
							"Fatal Error",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(0);
	}
	APP_HWND=CreateWindowEx(NULL,
						    ID_WINDOWCLASS,
						    ID_WINDOWTITLE,
						    WS_POPUP|WS_VISIBLE,
						    CW_USEDEFAULT,
						    CW_USEDEFAULT,
						    CW_USEDEFAULT,
						    CW_USEDEFAULT,
						    NULL,
						    NULL,
						    hInstance,
						    NULL);
	if(!APP_HWND)
	{
		DisplayErrorMessage("Failed to create window.\nProgram will now exit.",
							"Fatal Error",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(0);
	}
	if(FAILED(CoInitialize(NULL)))
	{
		DisplayErrorMessage("Failed to initialize component object module required for DirectMusic.\nMusic will not function",
							"Error - WinMain()",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	if(!CALL_INIT_PROC())
	{
		DisplayErrorMessage("A fatal error has occurred.\nProgram will now exit.",
							"Fatal Error",
							MB_OK|MB_ICONSTOP,
							NULL);
		CALL_REST_PROC();
		CoUninitialize();
		return(0);
	}
	while(TRUE)
	{
		if(PeekMessage(&Msg,hAppWnd,0,0,PM_REMOVE))
		{
			if(Msg.message==WM_QUIT)
				break;
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		CALL_MAIN_PROC();
	}
	CALL_REST_PROC();
	CoUninitialize();
	return(Msg.wParam);
}

// DIRECTDRAW FUNCTIONS //

CDirectDraw::Initialize(DIRECTDRAWINITSTRUCT_PTR lpddis)
{
	DWORD count;
	HGLOBAL RgnDataMem;
	LPDIRECTDRAW lpDDTemp=NULL;
	DWORD flags;
	
	lpDirectDraw=NULL;
	lpDirectDrawPalette=NULL;
	lpPrimaryDirectDrawSurface=NULL;
	PrimarySurfaceLocked=FALSE;
	lpSecondaryDirectDrawSurface=NULL;
	SecondarySurfaceLocked=FALSE;
	lpDirectDrawClipper=NULL;
	Palette=NULL;
	PaletteMem=NULL;
	SavePalette=NULL;
	SavePaletteMem=NULL;
	ActualBitsPerPixel=(BYTE)lpddis->dwScreenBitsPerPixel;
	if(FAILED(DirectDrawCreate(NULL,
							   &lpDDTemp,
							   NULL)))
	{
		DisplayErrorMessage("Could not create DirectDraw object.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	if(FAILED(lpDDTemp->QueryInterface(IID_IDirectDraw4,
									   (LPVOID*)&lpDirectDraw)))
	{
		DisplayErrorMessage("Could not create DirectDraw interface.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	if(lpDDTemp)
	{
		lpDDTemp->Release();
		lpDDTemp=NULL;
	}
	flags=DDSCL_FULLSCREEN|
		  DDSCL_ALLOWMODEX|
		  DDSCL_EXCLUSIVE|
		  DDSCL_ALLOWREBOOT;
#ifdef INIT_DIRECT3D
	flags|=DDSCL_FPUSETUP;
#endif
	if(FAILED(lpDirectDraw->SetCooperativeLevel(APP_HWND,
												flags)))
	{
		DisplayErrorMessage("Failed to set Windows cooperative level.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(FAILED(lpDirectDraw->SetDisplayMode(lpddis->dwScreenWidth,
										   lpddis->dwScreenHeight,
										   lpddis->dwScreenBitsPerPixel,
										   NULL,
										   NULL)))
	{
		if(lpddis->dwScreenBitsPerPixel==32)
		{
			lpddis->dwScreenBitsPerPixel=24;
			ActualBitsPerPixel=24;
			if(FAILED(lpDirectDraw->SetDisplayMode(lpddis->dwScreenWidth,
												   lpddis->dwScreenHeight,
												   lpddis->dwScreenBitsPerPixel,
												   NULL,
												   NULL)))
			{
				lpddis->dwScreenBitsPerPixel=16;
				ActualBitsPerPixel=16;
				if(FAILED(lpDirectDraw->SetDisplayMode(lpddis->dwScreenWidth,
													   lpddis->dwScreenHeight,
													   lpddis->dwScreenBitsPerPixel,
													   NULL,
													   NULL)))
				{
					DisplayErrorMessage("Failed to set video display mode.",
										"Error - CDirectDraw::Initialize()",
										MB_OK|MB_ICONSTOP,
										this);
					return(FALSE);
				}
			}
		}
		else
		{
			DisplayErrorMessage("Failed to set video display mode.",
								"Error - CDirectDraw::Initialize()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
	}
	PaletteMem=GlobalAlloc(GHND,sizeof(PALETTEENTRY)*256);
	if(!PaletteMem)
	{
		DisplayErrorMessage("Failed to allocate memory for primary palette data.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	Palette=(LPPALETTEENTRY)GlobalLock(PaletteMem);
	if(!Palette)
	{
		DisplayErrorMessage("Failed to lock memory for primary palette data.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	SavePaletteMem=GlobalAlloc(GHND,sizeof(PALETTEENTRY)*256);
	if(!SavePaletteMem)
	{
		DisplayErrorMessage("Failed to allocate memory for stored palette data.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	SavePalette=(LPPALETTEENTRY)GlobalLock(SavePaletteMem);
	if(!SavePalette)
	{
		DisplayErrorMessage("Failed to lock memory for stored palette data.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	SetDirectDrawPaletteEntry(0,0,0,0);
	for(count=1;count<255;count++)
		SetDirectDrawPaletteEntry((unsigned char)count,rand()%255,rand()%255,rand()%255);
	SetDirectDrawPaletteEntry(255,255,255,255);
	if(lpddis->dwScreenBitsPerPixel==8)
	{
		if(FAILED(lpDirectDraw->CreatePalette(DDPCAPS_8BIT|
											  DDPCAPS_ALLOW256|
											  DDPCAPS_INITIALIZE,
											  Palette,
											  &lpDirectDrawPalette,
											  NULL)))
			DisplayErrorMessage("Failed to create default palette.\nProgram will function, but colors may not display properly.",
								"Error - CDirectDraw::Initialize()",
								MB_OK|MB_ICONINFORMATION,
								this);
	}
ReAttemptSurfaceCreation:;
	INIT_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS|
				 DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE|
						DDSCAPS_COMPLEX|
						DDSCAPS_FLIP;
#ifdef INIT_DIRECT3D
	ddsd.ddsCaps.dwCaps|=DDSCAPS_3DDEVICE;
#endif
	ddsd.dwBackBufferCount=lpddis->dwBackBufferCount;
	if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
										  &lpPrimaryDirectDrawSurface,
										  NULL)))
	{
		if(lpddis->dwBackBufferCount==1)
		{
			DisplayErrorMessage("Failed to create primary DirectDraw display surface.",
								"Error - CDirectDraw::Initialize()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
		else
		{
			lpddis->dwBackBufferCount--;
			goto ReAttemptSurfaceCreation;
		}
	}
	ddsd.ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
	if(FAILED(lpPrimaryDirectDrawSurface->GetAttachedSurface(&ddsd.ddsCaps,
															 &lpSecondaryDirectDrawSurface)))
		{
			DisplayErrorMessage("Failed to create secondary DirectDraw display surface.",
								"Error - CDirectDraw::Initialize()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
	if(lpddis->dwScreenBitsPerPixel==16)
	{
		INIT_STRUCT(ddpixelformat)
		if(FAILED(lpPrimaryDirectDrawSurface->GetPixelFormat(&ddpixelformat)))
		{
			DisplayErrorMessage("Failed to read pixel format for 16-bit display surface.",
								"Error - CDirectDraw::Initialize()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
		lpddis->dwScreenBitsPerPixel=ddpixelformat.dwRGBBitCount;
		ActualBitsPerPixel=(BYTE)ddpixelformat.dwRGBBitCount;
	}
	if(lpddis->dwScreenBitsPerPixel==8)
		if(FAILED(lpPrimaryDirectDrawSurface->SetPalette(lpDirectDrawPalette)))
		{
			DisplayErrorMessage("Failed to attach default palette.\nProgram will function, but colors may not display properly.",
								"Error - CDirectDraw::Initialize()",
								MB_OK|MB_ICONINFORMATION,
								this);
		}
	ClearPrimaryDirectDrawSurface();
	ClearSecondaryDirectDrawSurface();
	if(FAILED(lpDirectDraw->CreateClipper(NULL,
										  &lpDirectDrawClipper,
										  NULL)))
	{
		DisplayErrorMessage("Failed to create DirectDraw clipper.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	RgnDataMem=GlobalAlloc(GHND,
						   sizeof(RGNDATAHEADER)+sizeof(RECT));
	if(!RgnDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for DirectDraw clip list.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	lpRgnData=(LPRGNDATA)GlobalLock(RgnDataMem);
	if(!lpRgnData)
	{
		DisplayErrorMessage("Failed to lock memory for DirectDraw clip list.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		GlobalFree(RgnDataMem);
		return(FALSE);
	}
	lpRgnData->rdh.dwSize=sizeof(RGNDATAHEADER);
	lpRgnData->rdh.iType=RDH_RECTANGLES;
	lpRgnData->rdh.nCount=1;
	lpRgnData->rdh.nRgnSize=sizeof(RECT);
	lpRgnData->rdh.rcBound.left=0;
	lpRgnData->rdh.rcBound.top=0;
	lpRgnData->rdh.rcBound.right=lpddis->dwScreenWidth;
	lpRgnData->rdh.rcBound.bottom=lpddis->dwScreenHeight;
	rect.left=0;
	rect.top=0;
	rect.right=lpddis->dwScreenWidth;
	rect.bottom=lpddis->dwScreenHeight;
	CopyMemory(&lpRgnData->Buffer,&rect,sizeof(RECT));
	if(FAILED(lpDirectDrawClipper->SetClipList(lpRgnData,NULL)))
	{
		DisplayErrorMessage("Failed to initialize DirectDraw clip list.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		GlobalUnlock(RgnDataMem);
		GlobalFree(RgnDataMem);
		return(FALSE);
	}
	GlobalUnlock(RgnDataMem);
	if(GlobalFree(RgnDataMem))
	{
		DisplayErrorMessage("Failed to free memory for DirectDraw clip list.\nSystem may become unstable.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(FAILED(lpSecondaryDirectDrawSurface->SetClipper(lpDirectDrawClipper)))
	{
		DisplayErrorMessage("Failed to attach DirectDraw clipper.",
							"Error - CDirectDraw::Initialize()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	return(TRUE);
}

LPDIRECTDRAW4 inline _fastcall CDirectDraw::GetDirectDrawInterfacePointer(void)
{
	return(lpDirectDraw);
}

LPDIRECTDRAWPALETTE inline _fastcall CDirectDraw::GetDirectDrawPaletteInterfacePointer(void)
{
	return(lpDirectDrawPalette);
}

LPDIRECTDRAWSURFACE4 inline _fastcall CDirectDraw::GetPrimaryDirectDrawSurfaceInterfacePointer(void)
{
	return(lpPrimaryDirectDrawSurface);
}

BOOL _fastcall CDirectDraw::LockPrimaryDirectDrawSurface(LPSTR* VideoMemoryPtr,LPDWORD PitchPtr)
{
	if(PrimarySurfaceLocked)
	{
		DisplayErrorMessage("Primary DirectDraw display surface is already locked.",
							"Error - CDirectDraw::LockPrimaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(TRUE);
	}
	INIT_STRUCT(ddsd);
	if(FAILED(lpPrimaryDirectDrawSurface->Lock(NULL,
											   &ddsd,
											   DDLOCK_SURFACEMEMORYPTR|
											   DDLOCK_WAIT,
											   NULL)))
	{
		DisplayErrorMessage("Failed to lock primary DirectDraw display surface.",
							"Error - CDirectDraw::LockPrimaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	PrimarySurfaceLocked=TRUE;
	*VideoMemoryPtr=(LPSTR)ddsd.lpSurface;
	*PitchPtr=ddsd.lPitch;
	return(TRUE);
}

BOOL _fastcall CDirectDraw::UnlockPrimaryDirectDrawSurface(void)
{
	if(PrimarySurfaceLocked)
	{
		if(FAILED(lpPrimaryDirectDrawSurface->Unlock(NULL)))
		{
			DisplayErrorMessage("Failed to unlock primary DirectDraw display surface.",
								"Error - CDirectDraw::UnlockPrimaryDirectDrawSurface()",
								MB_OK|MB_ICONINFORMATION,
								this);
			return(FALSE);
		}
	PrimarySurfaceLocked=FALSE;
	}
	else
		DisplayErrorMessage("Primary DirectDraw display surface is not locked.",
							"Error - CDirectDraw::UnlockPrimaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
	return(TRUE);
}

LPDIRECTDRAWSURFACE4 inline _fastcall CDirectDraw::GetSecondaryDirectDrawSurfaceInterfacePointer(void)
{
	return(lpSecondaryDirectDrawSurface);
}

BOOL _fastcall CDirectDraw::LockSecondaryDirectDrawSurface(LPSTR* VideoMemoryPtr,LPDWORD PitchPtr)
{
	if(SecondarySurfaceLocked)
	{
		DisplayErrorMessage("Secondary DirectDraw display surface is already locked.",
							"Error - CDirectDraw::LockSecondaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(TRUE);
	}
	INIT_STRUCT(ddsd);
	if(FAILED(lpSecondaryDirectDrawSurface->Lock(NULL,
												 &ddsd,
											     DDLOCK_SURFACEMEMORYPTR|
											     DDLOCK_WAIT,
											     NULL)))
	{
		DisplayErrorMessage("Failed to lock secondary DirectDraw display surface.",
							"Error - CDirectDraw::LockSecondaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	SecondarySurfaceLocked=TRUE;
	*VideoMemoryPtr=(LPSTR)ddsd.lpSurface;
	*PitchPtr=ddsd.lPitch;
	return(TRUE);
}

BOOL _fastcall CDirectDraw::UnlockSecondaryDirectDrawSurface(void)
{
	if(SecondarySurfaceLocked)
	{
		if(FAILED(lpSecondaryDirectDrawSurface->Unlock(NULL)))
		{
			DisplayErrorMessage("Failed to unlock secondary DirectDraw display surface.",
								"Error - CDirectDraw::UnlockSecondaryDirectDrawSurface()",
								MB_OK|MB_ICONINFORMATION,
								this);
			return(FALSE);
		}
	SecondarySurfaceLocked=FALSE;
	}
	else
		DisplayErrorMessage("Secondary DirectDraw display surface is not locked.",
							"Error - CDirectDraw::UnlockSecondaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
	return(TRUE);
}

BOOL inline _fastcall CDirectDraw::PerformPageFlip(void)
{
	if(PrimarySurfaceLocked||SecondarySurfaceLocked)
	{
		DisplayErrorMessage("Failed to perform page flip.",
							"Error - CDirectDraw::PerformPageFlip()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	else
		while(FAILED(lpPrimaryDirectDrawSurface->Flip(NULL,DDFLIP_WAIT)))
		{}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::ClearPrimaryDirectDrawSurface(void)
{
	INIT_STRUCT(ddbltfx);
	ddbltfx.dwFillColor=0;
	if(FAILED(lpPrimaryDirectDrawSurface->Blt(NULL,
											  NULL,
											  NULL,
											  DDBLT_COLORFILL|
											  DDBLT_WAIT,
											  &ddbltfx)))
	{
		DisplayErrorMessage("Blit operation failed.",
							"Error - CDirectDraw::ClearPrimaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::ClearSecondaryDirectDrawSurface(void)
{
	INIT_STRUCT(ddbltfx);
	ddbltfx.dwFillColor=0;
	if(FAILED(lpSecondaryDirectDrawSurface->Blt(NULL,
											    NULL,
											    NULL,
											    DDBLT_COLORFILL|
											    DDBLT_WAIT,
											    &ddbltfx)))
	{
		DisplayErrorMessage("Blit operation failed.",
							"Error - CDirectDraw::ClearSecondaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::ClearPrimaryRect(DWORD x1,DWORD y1,DWORD x2,DWORD y2)
{
	RECT destrect;

	INIT_STRUCT(ddbltfx);
	ddbltfx.dwFillColor=0;
	destrect.left=x1;
	destrect.right=x2;
	destrect.top=y1;
	destrect.bottom=y2;
	if(FAILED(lpPrimaryDirectDrawSurface->Blt(&destrect,
											  NULL,
											  NULL,
											  DDBLT_COLORFILL|
											  DDBLT_WAIT,
											  &ddbltfx)))
	{
		DisplayErrorMessage("Blit operation failed.",
							"Error - CDirectDraw::ClearPrimaryRect()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::ClearSecondaryRect(DWORD x1,DWORD y1,DWORD x2,DWORD y2)
{
	RECT destrect;

	INIT_STRUCT(ddbltfx);
	ddbltfx.dwFillColor=0;
	destrect.left=x1;
	destrect.right=x2;
	destrect.top=y1;
	destrect.bottom=y2;
	if(FAILED(lpSecondaryDirectDrawSurface->Blt(&destrect,
											    NULL,
											    NULL,
											    DDBLT_COLORFILL|
											    DDBLT_WAIT,
											    &ddbltfx)))
	{
		DisplayErrorMessage("Blit operation failed.",
							"Error - CDirectDraw::ClearSecondaryRect()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

LPDIRECTDRAWCLIPPER inline _fastcall CDirectDraw::GetDirectDrawClipperInterfacePointer(void)
{
	return(lpDirectDrawClipper);
}

BOOL _fastcall CDirectDraw::SetDirectDrawPaletteEntry(BYTE index,BYTE red,BYTE green,BYTE blue)
{
	static ErrorAlreadyDisplayed=FALSE;
	
	if(Palette)
	{
		Palette[index].peRed=red;
		Palette[index].peGreen=green;
		Palette[index].peBlue=blue;
		Palette[index].peFlags=PC_NOCOLLAPSE;
	}
	else
		if(!ErrorAlreadyDisplayed)
		{
			DisplayErrorMessage("Invalid palette pointer.",
								"CDirectDraw::SetDirectDrawPaletteEntry",
								MB_OK|MB_ICONINFORMATION,
								this);
			ErrorAlreadyDisplayed=TRUE;
		}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::GetDirectDrawPaletteEntry(BYTE index,LPBYTE lpred,LPBYTE lpgreen,LPBYTE lpblue)
{
	*lpred=Palette[index].peRed;
	*lpgreen=Palette[index].peGreen;
	*lpblue=Palette[index].peBlue;
	return(TRUE);
}

BOOL _fastcall CDirectDraw::UpdateDirectDrawPalette(DWORD start,DWORD end)
{
	if(ActualBitsPerPixel!=8)
		return(TRUE);
	if(FAILED(lpDirectDrawPalette->SetEntries(NULL,
											  start,
											  end,
											  Palette)))
	{
		DisplayErrorMessage("Failed to set DirectDraw palette.",
							"Error - CDirectDraw::SetDirectDrawPalette()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadPaletteFromDataFile(void)
{
	if(ActualBitsPerPixel==15)
		return(LoadPaletteFromDataFile15());
	if(ActualBitsPerPixel==16)
		return(LoadPaletteFromDataFile16());
	if(ActualBitsPerPixel==24)
		return(LoadPaletteFromDataFile24());
	if(ActualBitsPerPixel==32)
		return(LoadPaletteFromDataFile32());
	
	BYTE DataType;
	HGLOBAL PaletteDataMem;
	LPSTR PaletteData;
	DWORD count;
	DWORD BytesRead;
		
	PaletteDataMem=GlobalAlloc(GHND,768);
	if(!PaletteDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	PaletteData=(LPSTR)GlobalLock(PaletteDataMem);
	if(!PaletteData)
	{
		DisplayErrorMessage("Failed to lock memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette ID from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDPALETTE)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)PaletteData,
				 768,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette data from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	for(count=0;count<256;count++)
		SetDirectDrawPaletteEntry((BYTE)count,
								  PaletteData[count*3],
								  PaletteData[(count*3)+1],
								  PaletteData[(count*3)+2]);
	UpdateDirectDrawPalette(0,256);
	GlobalUnlock(PaletteDataMem);
	if(GlobalFree(PaletteDataMem))
	{
		DisplayErrorMessage("Failed to free memory for palette resource. System may become unstable.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadPaletteFromDataFile15(void)
{
	BYTE DataType;
	HGLOBAL PaletteDataMem;
	LPSTR PaletteData;
	DWORD count;
	DWORD BytesRead;
		
	PaletteDataMem=GlobalAlloc(GHND,768);
	if(!PaletteDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	PaletteData=(LPSTR)GlobalLock(PaletteDataMem);
	if(!PaletteData)
	{
		DisplayErrorMessage("Failed to lock memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette ID from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDPALETTE)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)PaletteData,
				 768,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette data from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	for(count=0;count<256;count++)
		SetDirectDrawPaletteEntry((BYTE)count,
								  PaletteData[count*3],
								  PaletteData[(count*3)+1],
								  PaletteData[(count*3)+2]);
	GlobalUnlock(PaletteDataMem);
	if(GlobalFree(PaletteDataMem))
	{
		DisplayErrorMessage("Failed to free memory for palette resource. System may become unstable.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadPaletteFromDataFile16(void)
{
	BYTE DataType;
	HGLOBAL PaletteDataMem;
	LPSTR PaletteData;
	DWORD count;
	DWORD BytesRead;
		
	PaletteDataMem=GlobalAlloc(GHND,768);
	if(!PaletteDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	PaletteData=(LPSTR)GlobalLock(PaletteDataMem);
	if(!PaletteData)
	{
		DisplayErrorMessage("Failed to lock memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette ID from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDPALETTE)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)PaletteData,
				 768,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette data from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	for(count=0;count<256;count++)
		SetDirectDrawPaletteEntry((BYTE)count,
								  PaletteData[count*3],
								  PaletteData[(count*3)+1],
								  PaletteData[(count*3)+2]);
	GlobalUnlock(PaletteDataMem);
	if(GlobalFree(PaletteDataMem))
	{
		DisplayErrorMessage("Failed to free memory for palette resource. System may become unstable.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadPaletteFromDataFile24(void)
{
	BYTE DataType;
	HGLOBAL PaletteDataMem;
	LPSTR PaletteData;
	DWORD count;
	DWORD BytesRead;
		
	PaletteDataMem=GlobalAlloc(GHND,768);
	if(!PaletteDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	PaletteData=(LPSTR)GlobalLock(PaletteDataMem);
	if(!PaletteData)
	{
		DisplayErrorMessage("Failed to lock memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette ID from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDPALETTE)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)PaletteData,
				 768,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette data from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	for(count=0;count<256;count++)
		SetDirectDrawPaletteEntry((BYTE)count,
								  PaletteData[count*3],
								  PaletteData[(count*3)+1],
								  PaletteData[(count*3)+2]);
	GlobalUnlock(PaletteDataMem);
	if(GlobalFree(PaletteDataMem))
	{
		DisplayErrorMessage("Failed to free memory for palette resource. System may become unstable.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadPaletteFromDataFile32(void)
{
	BYTE DataType;
	HGLOBAL PaletteDataMem;
	LPSTR PaletteData;
	DWORD count;
	DWORD BytesRead;
		
	PaletteDataMem=GlobalAlloc(GHND,768);
	if(!PaletteDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	PaletteData=(LPSTR)GlobalLock(PaletteDataMem);
	if(!PaletteData)
	{
		DisplayErrorMessage("Failed to lock memory for palette resource.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette ID from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDPALETTE)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)PaletteData,
				 768,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read palette data from disk.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	for(count=0;count<256;count++)
		SetDirectDrawPaletteEntry((BYTE)count,
								  PaletteData[count*3],
								  PaletteData[(count*3)+1],
								  PaletteData[(count*3)+2]);
	GlobalUnlock(PaletteDataMem);
	if(GlobalFree(PaletteDataMem))
	{
		DisplayErrorMessage("Failed to free memory for palette resource. System may become unstable.",
							"Error - CDirectDraw::LoadPaletteFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadBitmapFromDataFile(BITMAP_DATA_PTR bmpdataptr)
{
	if(ActualBitsPerPixel==15)
		return(LoadBitmapFromDataFile15(bmpdataptr));
	if(ActualBitsPerPixel==16)
		return(LoadBitmapFromDataFile16(bmpdataptr));
	if(ActualBitsPerPixel==24)
		return(LoadBitmapFromDataFile24(bmpdataptr));
	if(ActualBitsPerPixel==32)
		return(LoadBitmapFromDataFile32(bmpdataptr));

	BYTE DataType;
	DWORD sizedata;
	BYTE data;
	BYTE data2;
	DWORD count;
	LPSTR buffer=NULL;
	DWORD pitch=0;
	DWORD x;
	DWORD y;
	DWORD BytesRead;
	
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap ID from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDBITMAP)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.left=0;
	bmpdataptr->srcrect.top=0;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap width from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.right=sizedata;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap height from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.bottom=sizedata;
	bmpdataptr->bmpsurf=NULL;
	INIT_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
	ddsd.dwWidth=bmpdataptr->srcrect.right;
	ddsd.dwHeight=bmpdataptr->srcrect.bottom;
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
						DDSCAPS_VIDEOMEMORY;
	if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
										  &bmpdataptr->bmpsurf,
										  NULL)))
	{
		bmpdataptr->bmpsurf=NULL;
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=bmpdataptr->srcrect.right;
		ddsd.dwHeight=bmpdataptr->srcrect.bottom;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_SYSTEMMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &bmpdataptr->bmpsurf,
											  NULL)))
		{
			DisplayErrorMessage("Failed to create offscreen surface for bitmap storage.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
	}
	bmpdataptr->surflock=FALSE;
	colorkey.dwColorSpaceLowValue=0;
	colorkey.dwColorSpaceHighValue=0;
	if(FAILED(bmpdataptr->bmpsurf->SetColorKey(DDCKEY_SRCBLT,
											   &colorkey)))
		DisplayErrorMessage("Failed to set color key for bitmap storage.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
	LockBitmapSurface(bmpdataptr,&buffer,&pitch);
	x=0;
	y=0;
	while(y<(DWORD)bmpdataptr->srcrect.bottom)
	{
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
				buffer[x+(y*pitch)]=0;
				x++;
				if(x>=(DWORD)bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
		if(y>=(DWORD)bmpdataptr->srcrect.bottom)
			break;
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
			if(!ReadFile(DataFile,
						 (LPVOID)&data2,
						 sizeof(BYTE),
						 &BytesRead,
						 NULL))
				{
					DisplayErrorMessage("Failed to read bitmap data from disk.",
										"Error - CDirectDraw::LoadBitmapFromDataFile()",
										MB_OK|MB_ICONINFORMATION,
										this);
					UnlockBitmapSurface(bmpdataptr);
					return(FALSE);
				}
				buffer[x+(y*pitch)]=(unsigned char)data2;
				x++;
				if(x>=(DWORD)bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
	}
	UnlockBitmapSurface(bmpdataptr);
	return(TRUE);
}

BOOL CDirectDraw::LoadBitmapFromDataFile15(BITMAP_DATA_PTR bmpdataptr)
{
	BYTE DataType;
	DWORD sizedata;
	BYTE data;
	BYTE data2;
	DWORD count;
	LPSTR buffer=NULL;
	DWORD pitch=0;
	long x;
	long y;
	DWORD BytesRead;
	
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap ID from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDBITMAP)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.left=0;
	bmpdataptr->srcrect.top=0;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap width from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.right=sizedata;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap height from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.bottom=sizedata;
	bmpdataptr->bmpsurf=NULL;
	INIT_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
	ddsd.dwWidth=bmpdataptr->srcrect.right;
	ddsd.dwHeight=bmpdataptr->srcrect.bottom;
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
						DDSCAPS_VIDEOMEMORY;
	if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
										  &bmpdataptr->bmpsurf,
										  NULL)))
	{
		bmpdataptr->bmpsurf=NULL;
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=bmpdataptr->srcrect.right;
		ddsd.dwHeight=bmpdataptr->srcrect.bottom;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_SYSTEMMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &bmpdataptr->bmpsurf,
											  NULL)))
		{
			DisplayErrorMessage("Failed to create offscreen surface for bitmap storage.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
	}
	bmpdataptr->surflock=FALSE;
	colorkey.dwColorSpaceLowValue=0;
	colorkey.dwColorSpaceHighValue=0;
	if(FAILED(bmpdataptr->bmpsurf->SetColorKey(DDCKEY_SRCBLT,
											   &colorkey)))
		DisplayErrorMessage("Failed to set color key for bitmap storage.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
	LockBitmapSurface(bmpdataptr,&buffer,&pitch);
	x=0;
	y=0;
	while(y<bmpdataptr->srcrect.bottom)
	{
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
				long pos=(x*2)+(y*pitch);
				buffer[pos]=0;
				buffer[pos+1]=0;
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
		if(y>=bmpdataptr->srcrect.bottom)
			break;
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
			if(!ReadFile(DataFile,
						 (LPVOID)&data2,
						 sizeof(BYTE),
						 &BytesRead,
						 NULL))
				{
					DisplayErrorMessage("Failed to read bitmap data from disk.",
										"Error - CDirectDraw::LoadBitmapFromDataFile()",
										MB_OK|MB_ICONINFORMATION,
										this);
					UnlockBitmapSurface(bmpdataptr);
					return(FALSE);
				}
				long pos=(x*2)+(y*pitch);
				PALETTEENTRY clr=Palette[data2];
				BYTE blue=(clr.peBlue>>3)&31;
				BYTE green=(clr.peGreen>>3)&31;
				BYTE red=(clr.peRed>>3)&31;
				buffer[pos]=blue+((green<<5)&224);
				buffer[pos+1]=((green>>3)&3)+((red<<2)&124);
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
	}
	UnlockBitmapSurface(bmpdataptr);
	return(TRUE);
}

BOOL CDirectDraw::LoadBitmapFromDataFile16(BITMAP_DATA_PTR bmpdataptr)
{
	BYTE DataType;
	DWORD sizedata;
	BYTE data;
	BYTE data2;
	DWORD count;
	LPSTR buffer=NULL;
	DWORD pitch=0;
	long x;
	long y;
	DWORD BytesRead;
	
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap ID from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDBITMAP)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.left=0;
	bmpdataptr->srcrect.top=0;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap width from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.right=sizedata;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap height from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.bottom=sizedata;
	bmpdataptr->bmpsurf=NULL;
	INIT_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
	ddsd.dwWidth=bmpdataptr->srcrect.right;
	ddsd.dwHeight=bmpdataptr->srcrect.bottom;
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
						DDSCAPS_VIDEOMEMORY;
	if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
										  &bmpdataptr->bmpsurf,
										  NULL)))
	{
		bmpdataptr->bmpsurf=NULL;
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=bmpdataptr->srcrect.right;
		ddsd.dwHeight=bmpdataptr->srcrect.bottom;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_SYSTEMMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &bmpdataptr->bmpsurf,
											  NULL)))
		{
			DisplayErrorMessage("Failed to create offscreen surface for bitmap storage.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
	}
	bmpdataptr->surflock=FALSE;
	colorkey.dwColorSpaceLowValue=0;
	colorkey.dwColorSpaceHighValue=0;
	if(FAILED(bmpdataptr->bmpsurf->SetColorKey(DDCKEY_SRCBLT,
											   &colorkey)))
		DisplayErrorMessage("Failed to set color key for bitmap storage.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
	LockBitmapSurface(bmpdataptr,&buffer,&pitch);
	x=0;
	y=0;
	while(y<bmpdataptr->srcrect.bottom)
	{
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
				long pos=(x*2)+(y*pitch);
				buffer[pos]=0;
				buffer[pos+1]=0;
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
		if(y>=bmpdataptr->srcrect.bottom)
			break;
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
			if(!ReadFile(DataFile,
						 (LPVOID)&data2,
						 sizeof(BYTE),
						 &BytesRead,
						 NULL))
				{
					DisplayErrorMessage("Failed to read bitmap data from disk.",
										"Error - CDirectDraw::LoadBitmapFromDataFile()",
										MB_OK|MB_ICONINFORMATION,
										this);
					UnlockBitmapSurface(bmpdataptr);
					return(FALSE);
				}
				long pos=(x*2)+(y*pitch);
				PALETTEENTRY clr=Palette[data2];
				BYTE blue=(clr.peBlue>>3)&31;
				BYTE green=(clr.peGreen>>2)&63;
				BYTE red=(clr.peRed>>3)&31;
				buffer[pos]=blue+((green<<5)&224);
				buffer[pos+1]=((green>>3)&7)+((red<<3)&248);
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
	}
	UnlockBitmapSurface(bmpdataptr);
	return(TRUE);
}

BOOL CDirectDraw::LoadBitmapFromDataFile24(BITMAP_DATA_PTR bmpdataptr)
{
	BYTE DataType;
	DWORD sizedata;
	BYTE data;
	BYTE data2;
	DWORD count;
	LPSTR buffer=NULL;
	DWORD pitch=0;
	long x;
	long y;
	DWORD BytesRead;
	
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap ID from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDBITMAP)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.left=0;
	bmpdataptr->srcrect.top=0;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap width from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.right=sizedata;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap height from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.bottom=sizedata;
	bmpdataptr->bmpsurf=NULL;
	INIT_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
	ddsd.dwWidth=bmpdataptr->srcrect.right;
	ddsd.dwHeight=bmpdataptr->srcrect.bottom;
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
						DDSCAPS_VIDEOMEMORY;
	if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
										  &bmpdataptr->bmpsurf,
										  NULL)))
	{
		bmpdataptr->bmpsurf=NULL;
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=bmpdataptr->srcrect.right;
		ddsd.dwHeight=bmpdataptr->srcrect.bottom;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_SYSTEMMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &bmpdataptr->bmpsurf,
											  NULL)))
		{
			DisplayErrorMessage("Failed to create offscreen surface for bitmap storage.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
	}
	bmpdataptr->surflock=FALSE;
	colorkey.dwColorSpaceLowValue=0;
	colorkey.dwColorSpaceHighValue=0;
	if(FAILED(bmpdataptr->bmpsurf->SetColorKey(DDCKEY_SRCBLT,
											   &colorkey)))
		DisplayErrorMessage("Failed to set color key for bitmap storage.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
	LockBitmapSurface(bmpdataptr,&buffer,&pitch);
	x=0;
	y=0;
	while(y<bmpdataptr->srcrect.bottom)
	{
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
				long pos=(x*3)+(y*pitch);
				buffer[pos]=0;
				buffer[pos+1]=0;
				buffer[pos+2]=0;
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
		if(y>=bmpdataptr->srcrect.bottom)
			break;
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
			if(!ReadFile(DataFile,
						 (LPVOID)&data2,
						 sizeof(BYTE),
						 &BytesRead,
						 NULL))
				{
					DisplayErrorMessage("Failed to read bitmap data from disk.",
										"Error - CDirectDraw::LoadBitmapFromDataFile()",
										MB_OK|MB_ICONINFORMATION,
										this);
					UnlockBitmapSurface(bmpdataptr);
					return(FALSE);
				}
				long pos=(x*3)+(y*pitch);
				PALETTEENTRY clr=Palette[data2];
				buffer[pos]=clr.peBlue;
				buffer[pos+1]=clr.peGreen;
				buffer[pos+2]=clr.peRed;
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
	}
	UnlockBitmapSurface(bmpdataptr);
	return(TRUE);
}

BOOL CDirectDraw::LoadBitmapFromDataFile32(BITMAP_DATA_PTR bmpdataptr)
{
	BYTE DataType;
	DWORD sizedata;
	BYTE data;
	BYTE data2;
	DWORD count;
	LPSTR buffer=NULL;
	DWORD pitch=0;
	long x;
	long y;
	DWORD BytesRead;
	
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap ID from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDBITMAP)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.left=0;
	bmpdataptr->srcrect.top=0;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap width from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.right=sizedata;
	if(!ReadFile(DataFile,
				 (LPVOID)&sizedata,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read bitmap height from disk.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->srcrect.bottom=sizedata;
	bmpdataptr->bmpsurf=NULL;
	INIT_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
	ddsd.dwWidth=bmpdataptr->srcrect.right;
	ddsd.dwHeight=bmpdataptr->srcrect.bottom;
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
						DDSCAPS_VIDEOMEMORY;
	if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
										  &bmpdataptr->bmpsurf,
										  NULL)))
	{
		bmpdataptr->bmpsurf=NULL;
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=bmpdataptr->srcrect.right;
		ddsd.dwHeight=bmpdataptr->srcrect.bottom;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_SYSTEMMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &bmpdataptr->bmpsurf,
											  NULL)))
		{
			DisplayErrorMessage("Failed to create offscreen surface for bitmap storage.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
			return(FALSE);
		}
	}
	bmpdataptr->surflock=FALSE;
	colorkey.dwColorSpaceLowValue=0;
	colorkey.dwColorSpaceHighValue=0;
	if(FAILED(bmpdataptr->bmpsurf->SetColorKey(DDCKEY_SRCBLT,
											   &colorkey)))
		DisplayErrorMessage("Failed to set color key for bitmap storage.",
							"Error - CDirectDraw::LoadBitmapFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
	LockBitmapSurface(bmpdataptr,&buffer,&pitch);
	x=0;
	y=0;
	while(y<bmpdataptr->srcrect.bottom)
	{
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
				long pos=(x*4)+(y*pitch);
				buffer[pos]=0;
				buffer[pos+1]=0;
				buffer[pos+2]=0;
				buffer[pos+3]=0;
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
		if(y>=bmpdataptr->srcrect.bottom)
			break;
		if(!ReadFile(DataFile,
					 (LPVOID)&data,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read bitmap data from disk.",
								"Error - CDirectDraw::LoadBitmapFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								this);
			UnlockBitmapSurface(bmpdataptr);
			return(FALSE);
		}
		if(data)
			for(count=0;count<data;count++)
			{
			if(!ReadFile(DataFile,
						 (LPVOID)&data2,
						 sizeof(BYTE),
						 &BytesRead,
						 NULL))
				{
					DisplayErrorMessage("Failed to read bitmap data from disk.",
										"Error - CDirectDraw::LoadBitmapFromDataFile()",
										MB_OK|MB_ICONINFORMATION,
										this);
					UnlockBitmapSurface(bmpdataptr);
					return(FALSE);
				}
				long pos=(x*4)+(y*pitch);
				PALETTEENTRY clr=Palette[data2];
				buffer[pos]=clr.peBlue;
				buffer[pos+1]=clr.peGreen;
				buffer[pos+2]=clr.peRed;
				buffer[pos+3]=0;
				x++;
				if(x>=bmpdataptr->srcrect.right)
				{
					x=0;
					y++;
				}
			}
	}
	UnlockBitmapSurface(bmpdataptr);
	return(TRUE);
}

BOOL CDirectDraw::ReleaseBitmap(BITMAP_DATA_PTR bmpdataptr)
{
	if(bmpdataptr->bmpsurf)
	{
		bmpdataptr->bmpsurf->Release();
		bmpdataptr->bmpsurf=NULL;
	}
	return(TRUE);
}

LPDIRECTDRAWSURFACE4 inline _fastcall CDirectDraw::GetBitmapSurfaceInterfacePointer(BITMAP_DATA_PTR bmpdataptr)
{
	return(bmpdataptr->bmpsurf);
}

LPRECT inline _fastcall CDirectDraw::GetBitmapRect(BITMAP_DATA_PTR bmpdataptr)
{
	return(&bmpdataptr->srcrect);
}

BOOL _fastcall CDirectDraw::LockBitmapSurface(BITMAP_DATA_PTR bmpdataptr,LPSTR* buffer,LPDWORD pitch)
{
	if(bmpdataptr->surflock)
	{
		DisplayErrorMessage("Bitmap surface already locked.",
							"Error - CDirectDraw::LockBitmapSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(TRUE);
	}
	INIT_STRUCT(ddsd);
	if(FAILED(bmpdataptr->bmpsurf->Lock(NULL,
										&ddsd,
										DDLOCK_SURFACEMEMORYPTR|
										DDLOCK_WAIT,
										NULL)))
	{	
		DisplayErrorMessage("Failed to lock primary bitmap surface.",
							"Error - CDirectDraw::LockBitmapSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->surflock=TRUE;
	*buffer=(LPSTR)ddsd.lpSurface;
	*pitch=ddsd.lPitch;
	return(TRUE);
}

BOOL _fastcall CDirectDraw::UnlockBitmapSurface(BITMAP_DATA_PTR bmpdataptr)
{
	if(!bmpdataptr->surflock)
	{
		DisplayErrorMessage("Bitmap surface not currently locked.",
							"Error - CDirectDraw::UnockBitmapSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(TRUE);
	}
	if(FAILED(bmpdataptr->bmpsurf->Unlock(NULL)))
	{
		DisplayErrorMessage("Failed to unlock secondary DirectDraw display surface.",
							"Error - CDirectDraw::UnlockSecondaryDirectDrawSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	bmpdataptr->surflock=FALSE;
	return(TRUE);
}

BOOL inline _fastcall CDirectDraw::IsPrimaryDirectDrawSurfaceLocked(void)
{
	return((BOOL)PrimarySurfaceLocked);
}

BOOL inline _fastcall CDirectDraw::IsSecondaryDirectDrawSurfaceLocked(void)
{
	return((BOOL)SecondarySurfaceLocked);
}

BOOL _fastcall CDirectDraw::DisplayBitmap(BITMAP_DATA_PTR bmpdataptr,DWORD x,DWORD y)
{
	rect.left=x;
	rect.top=y;
	rect.right=x+GetBitmapRect(bmpdataptr)->right;
	rect.bottom=x+GetBitmapRect(bmpdataptr)->bottom;
	if(FAILED(lpPrimaryDirectDrawSurface->Blt(&rect,
											  GetBitmapSurfaceInterfacePointer(bmpdataptr),
											  GetBitmapRect(bmpdataptr),
											  DDBLT_KEYSRC|
											  DDBLT_WAIT,
											  NULL)))
	{
		DisplayErrorMessage("Blit operation failed",
							"Error - CDirectDraw::DisplayBitmap(DWORD,DWORD)",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::DisplayBitmap(BITMAP_DATA_PTR bmpdataptr,LPRECT lprect)
{
	if(FAILED(lpPrimaryDirectDrawSurface->Blt(lprect,
											  GetBitmapSurfaceInterfacePointer(bmpdataptr),
											  GetBitmapRect(bmpdataptr),
											  DDBLT_KEYSRC|
											  DDBLT_WAIT,
											  NULL)))
	{
		DisplayErrorMessage("Blit operation failed",
							"Error - CDirectDraw::DisplayBitmap(LPRECT)",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}


BOOL _fastcall CDirectDraw::DisplayBitmap(BITMAP_DATA_PTR bmpdataptr,DWORD x,DWORD y,DWORD scale)
{
	DWORD wf;
	DWORD hf;

	wf=((bmpdataptr->srcrect.right>>1)*scale)>>16;
	hf=((bmpdataptr->srcrect.bottom>>1)*scale)>>16;
	rect.left=x-wf;
	rect.top=y-hf;
	rect.right=x+wf;
	rect.bottom=y+hf;
	if(FAILED(lpPrimaryDirectDrawSurface->Blt(&rect,
											  GetBitmapSurfaceInterfacePointer(bmpdataptr),
											  GetBitmapRect(bmpdataptr),
											  DDBLT_KEYSRC|
											  DDBLT_WAIT,
											  NULL)))
	{
		DisplayErrorMessage("Blit operation failed",
							"Error - CDirectDraw::DisplayBitmap(DWORD,DWORD,DWORD)",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferBitmap(BITMAP_DATA_PTR bmpdataptr,DWORD x,DWORD y)
{
	rect.left=x;
	rect.top=y;
	rect.right=x+GetBitmapRect(bmpdataptr)->right;
	rect.bottom=y+GetBitmapRect(bmpdataptr)->bottom;
	if(FAILED(lpSecondaryDirectDrawSurface->Blt(&rect,
												GetBitmapSurfaceInterfacePointer(bmpdataptr),
												GetBitmapRect(bmpdataptr),
												DDBLT_KEYSRC|
												DDBLT_WAIT,
												NULL)))
	{
		DisplayErrorMessage("Blit operation failed",
							"Error - CDirectDraw::BufferBitmap(DWORD,DWORD)",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferBitmap(BITMAP_DATA_PTR bmpdataptr,LPRECT lprect)
{
	if(FAILED(lpSecondaryDirectDrawSurface->Blt(lprect,
												GetBitmapSurfaceInterfacePointer(bmpdataptr),
												GetBitmapRect(bmpdataptr),
												DDBLT_KEYSRC|
												DDBLT_WAIT,
												NULL)))
	{
		DisplayErrorMessage("Blit operation failed",
							"Error - CDirectDraw::BufferBitmap(LPRECT)",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferBitmap(BITMAP_DATA_PTR bmpdataptr,DWORD x,DWORD y,DWORD scale)
{
	DWORD wf;
	DWORD hf;

	wf=((bmpdataptr->srcrect.right>>1)*scale)>>16;
	hf=((bmpdataptr->srcrect.bottom>>1)*scale)>>16;
	rect.left=x-wf;
	rect.top=y-hf;
	rect.right=x+wf;
	rect.bottom=y+hf;
	if(FAILED(lpSecondaryDirectDrawSurface->Blt(&rect,
												GetBitmapSurfaceInterfacePointer(bmpdataptr),
												GetBitmapRect(bmpdataptr),
												DDBLT_KEYSRC|
												DDBLT_WAIT,
												NULL)))
	{
		DisplayErrorMessage("Blit operation failed",
							"Error - CDirectDraw::BufferBitmap(DWORD,DWORD,DWORD)",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadFontFromDataFile(FONT_DATA_PTR fontdataptr)
{
	if(ActualBitsPerPixel==15)
		return(LoadFontFromDataFile15(fontdataptr));
	if(ActualBitsPerPixel==16)
		return(LoadFontFromDataFile16(fontdataptr));
	if(ActualBitsPerPixel==24)
		return(LoadFontFromDataFile24(fontdataptr));
	if(ActualBitsPerPixel==32)
		return(LoadFontFromDataFile32(fontdataptr));
	
	BYTE DataType;
	DWORD BytesRead;
	DWORD dwInput;
	DWORD BytesToRead;
	DWORD count;
	HGLOBAL hGlobal;
	LPSTR data;
	LPSTR buffer;
	DWORD pitch;
	DWORD bitmask;
	DWORD readpos;
	DWORD x;
	DWORD y;
		
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font ID from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDFONT)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font width from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->width=dwInput;
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font height from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->height=dwInput;
	BytesToRead=(DWORD)(((fontdataptr->width*fontdataptr->height)/8)*73);
	if((fontdataptr->width*fontdataptr->height)&0x07)
		BytesToRead++;
	hGlobal=GlobalAlloc(GHND,BytesToRead);
	if(!hGlobal)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	data=(LPSTR)GlobalLock(hGlobal);
	if(!data)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)data,
				 BytesToRead,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font data from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	readpos=0;
	bitmask=1;
	for(count=0;count<73;count++)
	{
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=fontdataptr->width;
		ddsd.dwHeight=fontdataptr->height;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_VIDEOMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &fontdataptr->bmpdata[count].bmpsurf,
											  NULL)))
		{
			fontdataptr->bmpdata[count].bmpsurf=NULL;
			INIT_STRUCT(ddsd);
			ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
			ddsd.dwWidth=fontdataptr->width;
			ddsd.dwHeight=fontdataptr->height;
			ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
								DDSCAPS_SYSTEMMEMORY;
			if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
												  &fontdataptr->bmpdata[count].bmpsurf,
												  NULL)))
			{
				DisplayErrorMessage("Failed to create offscreen surface for font storage.",
									"Error - CDirectDraw::LoadFontFromDataFile()",
									MB_OK|MB_ICONSTOP,
									this);
				return(FALSE);
			}
		}
		colorkey.dwColorSpaceLowValue=0;
		colorkey.dwColorSpaceHighValue=0;
		if(FAILED(fontdataptr->bmpdata[count].bmpsurf->SetColorKey(DDCKEY_SRCBLT,
																   &colorkey)))
			DisplayErrorMessage("Failed to set color key for font storage.",
								"Error - CDirectDraw::LoadFontFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
		fontdataptr->bmpdata[count].surflock=FALSE;
		LockFontSurface(fontdataptr,(BYTE)count,&buffer,&pitch);
		for(y=0;y<fontdataptr->height;y++)
			for(x=0;x<fontdataptr->width;x++)
			{
				if(data[readpos]&(BYTE)bitmask)
					buffer[x+(y*pitch)]=1;
				else
					buffer[x+(y*pitch)]=0;
				bitmask*=2;
				if(bitmask>255)
				{
					bitmask=1;
					readpos++;
				}
			}
		UnlockFontSurface(fontdataptr,(BYTE)count);
		fontdataptr->bmpdata[count].srcrect.left=0;
		fontdataptr->bmpdata[count].srcrect.right=fontdataptr->width;
		fontdataptr->bmpdata[count].srcrect.top=0;
		fontdataptr->bmpdata[count].srcrect.bottom=fontdataptr->height;
	}
	GlobalUnlock(hGlobal);
	if(GlobalFree(hGlobal))
	{
		DisplayErrorMessage("Failed to free font data from memory.\nSystem may become unstable.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadFontFromDataFile15(FONT_DATA_PTR fontdataptr)
{
	BYTE DataType;
	DWORD BytesRead;
	DWORD dwInput;
	DWORD BytesToRead;
	DWORD count;
	HGLOBAL hGlobal;
	LPSTR data;
	LPSTR buffer;
	DWORD pitch;
	DWORD bitmask;
	DWORD readpos;
	DWORD x;
	DWORD y;
		
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font ID from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDFONT)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font width from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->width=dwInput;
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font height from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->height=dwInput;
	BytesToRead=(DWORD)(((fontdataptr->width*fontdataptr->height)/8)*73);
	if((fontdataptr->width*fontdataptr->height)&0x07)
		BytesToRead++;
	hGlobal=GlobalAlloc(GHND,BytesToRead);
	if(!hGlobal)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	data=(LPSTR)GlobalLock(hGlobal);
	if(!data)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)data,
				 BytesToRead,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font data from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	readpos=0;
	bitmask=1;
	for(count=0;count<73;count++)
	{
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=fontdataptr->width;
		ddsd.dwHeight=fontdataptr->height;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_VIDEOMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &fontdataptr->bmpdata[count].bmpsurf,
											  NULL)))
		{
			fontdataptr->bmpdata[count].bmpsurf=NULL;
			INIT_STRUCT(ddsd);
			ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
			ddsd.dwWidth=fontdataptr->width;
			ddsd.dwHeight=fontdataptr->height;
			ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
								DDSCAPS_SYSTEMMEMORY;
			if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
												  &fontdataptr->bmpdata[count].bmpsurf,
												  NULL)))
			{
				DisplayErrorMessage("Failed to create offscreen surface for font storage.",
									"Error - CDirectDraw::LoadFontFromDataFile()",
									MB_OK|MB_ICONSTOP,
									this);
				return(FALSE);
			}
		}
		colorkey.dwColorSpaceLowValue=0;
		colorkey.dwColorSpaceHighValue=0;
		if(FAILED(fontdataptr->bmpdata[count].bmpsurf->SetColorKey(DDCKEY_SRCBLT,
																   &colorkey)))
			DisplayErrorMessage("Failed to set color key for font storage.",
								"Error - CDirectDraw::LoadFontFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
		fontdataptr->bmpdata[count].surflock=FALSE;
		LockFontSurface(fontdataptr,(BYTE)count,&buffer,&pitch);
		for(y=0;y<fontdataptr->height;y++)
			for(x=0;x<fontdataptr->width;x++)
			{
				if(data[readpos]&(BYTE)bitmask)
				{
					long pos=(x*2)+(y*pitch);
					buffer[pos]=(BYTE)255;
					buffer[pos+1]=(BYTE)127;
				}
				else
				{
					long pos=(x*2)+(y*pitch);
					buffer[pos]=0;
					buffer[pos+1]=0;
				}
				bitmask*=2;
				if(bitmask>255)
				{
					bitmask=1;
					readpos++;
				}
			}
		UnlockFontSurface(fontdataptr,(BYTE)count);
		fontdataptr->bmpdata[count].srcrect.left=0;
		fontdataptr->bmpdata[count].srcrect.right=fontdataptr->width;
		fontdataptr->bmpdata[count].srcrect.top=0;
		fontdataptr->bmpdata[count].srcrect.bottom=fontdataptr->height;
	}
	GlobalUnlock(hGlobal);
	if(GlobalFree(hGlobal))
	{
		DisplayErrorMessage("Failed to free font data from memory.\nSystem may become unstable.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadFontFromDataFile16(FONT_DATA_PTR fontdataptr)
{
	BYTE DataType;
	DWORD BytesRead;
	DWORD dwInput;
	DWORD BytesToRead;
	DWORD count;
	HGLOBAL hGlobal;
	LPSTR data;
	LPSTR buffer;
	DWORD pitch;
	DWORD bitmask;
	DWORD readpos;
	DWORD x;
	DWORD y;
		
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font ID from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDFONT)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font width from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->width=dwInput;
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font height from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->height=dwInput;
	BytesToRead=(DWORD)(((fontdataptr->width*fontdataptr->height)/8)*73);
	if((fontdataptr->width*fontdataptr->height)&0x07)
		BytesToRead++;
	hGlobal=GlobalAlloc(GHND,BytesToRead);
	if(!hGlobal)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	data=(LPSTR)GlobalLock(hGlobal);
	if(!data)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)data,
				 BytesToRead,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font data from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	readpos=0;
	bitmask=1;
	for(count=0;count<73;count++)
	{
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=fontdataptr->width;
		ddsd.dwHeight=fontdataptr->height;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_VIDEOMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &fontdataptr->bmpdata[count].bmpsurf,
											  NULL)))
		{
			fontdataptr->bmpdata[count].bmpsurf=NULL;
			INIT_STRUCT(ddsd);
			ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
			ddsd.dwWidth=fontdataptr->width;
			ddsd.dwHeight=fontdataptr->height;
			ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
								DDSCAPS_SYSTEMMEMORY;
			if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
												  &fontdataptr->bmpdata[count].bmpsurf,
												  NULL)))
			{
				DisplayErrorMessage("Failed to create offscreen surface for font storage.",
									"Error - CDirectDraw::LoadFontFromDataFile()",
									MB_OK|MB_ICONSTOP,
									this);
				return(FALSE);
			}
		}
		colorkey.dwColorSpaceLowValue=0;
		colorkey.dwColorSpaceHighValue=0;
		if(FAILED(fontdataptr->bmpdata[count].bmpsurf->SetColorKey(DDCKEY_SRCBLT,
																   &colorkey)))
			DisplayErrorMessage("Failed to set color key for font storage.",
								"Error - CDirectDraw::LoadFontFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
		fontdataptr->bmpdata[count].surflock=FALSE;
		LockFontSurface(fontdataptr,(BYTE)count,&buffer,&pitch);
		for(y=0;y<fontdataptr->height;y++)
			for(x=0;x<fontdataptr->width;x++)
			{
				if(data[readpos]&(BYTE)bitmask)
				{
					long pos=(x*2)+(y*pitch);
					buffer[pos]=(BYTE)255;
					buffer[pos+1]=(BYTE)255;
				}
				else
				{
					long pos=(x*2)+(y*pitch);
					buffer[pos]=0;
					buffer[pos+1]=0;
				}
				bitmask*=2;
				if(bitmask>255)
				{
					bitmask=1;
					readpos++;
				}
			}
		UnlockFontSurface(fontdataptr,(BYTE)count);
		fontdataptr->bmpdata[count].srcrect.left=0;
		fontdataptr->bmpdata[count].srcrect.right=fontdataptr->width;
		fontdataptr->bmpdata[count].srcrect.top=0;
		fontdataptr->bmpdata[count].srcrect.bottom=fontdataptr->height;
	}
	GlobalUnlock(hGlobal);
	if(GlobalFree(hGlobal))
	{
		DisplayErrorMessage("Failed to free font data from memory.\nSystem may become unstable.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadFontFromDataFile24(FONT_DATA_PTR fontdataptr)
{
	BYTE DataType;
	DWORD BytesRead;
	DWORD dwInput;
	DWORD BytesToRead;
	DWORD count;
	HGLOBAL hGlobal;
	LPSTR data;
	LPSTR buffer;
	DWORD pitch;
	DWORD bitmask;
	DWORD readpos;
	DWORD x;
	DWORD y;
		
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font ID from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDFONT)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font width from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->width=dwInput;
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font height from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->height=dwInput;
	BytesToRead=(DWORD)(((fontdataptr->width*fontdataptr->height)/8)*73);
	if((fontdataptr->width*fontdataptr->height)&0x07)
		BytesToRead++;
	hGlobal=GlobalAlloc(GHND,BytesToRead);
	if(!hGlobal)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	data=(LPSTR)GlobalLock(hGlobal);
	if(!data)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)data,
				 BytesToRead,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font data from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	readpos=0;
	bitmask=1;
	for(count=0;count<73;count++)
	{
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=fontdataptr->width;
		ddsd.dwHeight=fontdataptr->height;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_VIDEOMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &fontdataptr->bmpdata[count].bmpsurf,
											  NULL)))
		{
			fontdataptr->bmpdata[count].bmpsurf=NULL;
			INIT_STRUCT(ddsd);
			ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
			ddsd.dwWidth=fontdataptr->width;
			ddsd.dwHeight=fontdataptr->height;
			ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
								DDSCAPS_SYSTEMMEMORY;
			if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
												  &fontdataptr->bmpdata[count].bmpsurf,
												  NULL)))
			{
				DisplayErrorMessage("Failed to create offscreen surface for font storage.",
									"Error - CDirectDraw::LoadFontFromDataFile()",
									MB_OK|MB_ICONSTOP,
									this);
				return(FALSE);
			}
		}
		colorkey.dwColorSpaceLowValue=0;
		colorkey.dwColorSpaceHighValue=0;
		if(FAILED(fontdataptr->bmpdata[count].bmpsurf->SetColorKey(DDCKEY_SRCBLT,
																   &colorkey)))
			DisplayErrorMessage("Failed to set color key for font storage.",
								"Error - CDirectDraw::LoadFontFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
		fontdataptr->bmpdata[count].surflock=FALSE;
		LockFontSurface(fontdataptr,(BYTE)count,&buffer,&pitch);
		for(y=0;y<fontdataptr->height;y++)
			for(x=0;x<fontdataptr->width;x++)
			{
				if(data[readpos]&(BYTE)bitmask)
				{
					long pos=(x*3)+(y*pitch);
					buffer[pos]=(BYTE)255;
					buffer[pos+1]=(BYTE)255;
					buffer[pos+2]=(BYTE)255;
				}
				else
				{
					long pos=(x*3)+(y*pitch);
					buffer[pos]=0;
					buffer[pos+1]=0;
					buffer[pos+2]=0;
				}
				bitmask*=2;
				if(bitmask>255)
				{
					bitmask=1;
					readpos++;
				}
			}
		UnlockFontSurface(fontdataptr,(BYTE)count);
		fontdataptr->bmpdata[count].srcrect.left=0;
		fontdataptr->bmpdata[count].srcrect.right=fontdataptr->width;
		fontdataptr->bmpdata[count].srcrect.top=0;
		fontdataptr->bmpdata[count].srcrect.bottom=fontdataptr->height;
	}
	GlobalUnlock(hGlobal);
	if(GlobalFree(hGlobal))
	{
		DisplayErrorMessage("Failed to free font data from memory.\nSystem may become unstable.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::LoadFontFromDataFile32(FONT_DATA_PTR fontdataptr)
{
	BYTE DataType;
	DWORD BytesRead;
	DWORD dwInput;
	DWORD BytesToRead;
	DWORD count;
	HGLOBAL hGlobal;
	LPSTR data;
	LPSTR buffer;
	DWORD pitch;
	DWORD bitmask;
	DWORD readpos;
	DWORD x;
	DWORD y;
		
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font ID from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	if(DataType!=DTF_IDFONT)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONSTOP,
							this);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font width from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->width=dwInput;
	if(!ReadFile(DataFile,
				 (LPVOID)&dwInput,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font height from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	fontdataptr->height=dwInput;
	BytesToRead=(DWORD)(((fontdataptr->width*fontdataptr->height)/8)*73);
	if((fontdataptr->width*fontdataptr->height)&0x07)
		BytesToRead++;
	hGlobal=GlobalAlloc(GHND,BytesToRead);
	if(!hGlobal)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	data=(LPSTR)GlobalLock(hGlobal);
	if(!data)
	{
		DisplayErrorMessage("Failed to allocate memory for font.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)data,
				 BytesToRead,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read font data from disk.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		return(FALSE);
	}
	readpos=0;
	bitmask=1;
	for(count=0;count<73;count++)
	{
		INIT_STRUCT(ddsd);
		ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.dwWidth=fontdataptr->width;
		ddsd.dwHeight=fontdataptr->height;
		ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
							DDSCAPS_VIDEOMEMORY;
		if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
											  &fontdataptr->bmpdata[count].bmpsurf,
											  NULL)))
		{
			fontdataptr->bmpdata[count].bmpsurf=NULL;
			INIT_STRUCT(ddsd);
			ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
			ddsd.dwWidth=fontdataptr->width;
			ddsd.dwHeight=fontdataptr->height;
			ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
								DDSCAPS_SYSTEMMEMORY;
			if(FAILED(lpDirectDraw->CreateSurface(&ddsd,
												  &fontdataptr->bmpdata[count].bmpsurf,
												  NULL)))
			{
				DisplayErrorMessage("Failed to create offscreen surface for font storage.",
									"Error - CDirectDraw::LoadFontFromDataFile()",
									MB_OK|MB_ICONSTOP,
									this);
				return(FALSE);
			}
		}
		colorkey.dwColorSpaceLowValue=0;
		colorkey.dwColorSpaceHighValue=0;
		if(FAILED(fontdataptr->bmpdata[count].bmpsurf->SetColorKey(DDCKEY_SRCBLT,
																   &colorkey)))
			DisplayErrorMessage("Failed to set color key for font storage.",
								"Error - CDirectDraw::LoadFontFromDataFile()",
								MB_OK|MB_ICONSTOP,
								this);
		fontdataptr->bmpdata[count].surflock=FALSE;
		LockFontSurface(fontdataptr,(BYTE)count,&buffer,&pitch);
		for(y=0;y<fontdataptr->height;y++)
			for(x=0;x<fontdataptr->width;x++)
			{
				if(data[readpos]&(BYTE)bitmask)
				{
					long pos=(x*4)+(y*pitch);
					buffer[pos]=(BYTE)255;
					buffer[pos+1]=(BYTE)255;
					buffer[pos+2]=(BYTE)255;
					buffer[pos+2]=0;
				}
				else
				{
					long pos=(x*4)+(y*pitch);
					buffer[pos]=0;
					buffer[pos+1]=0;
					buffer[pos+2]=0;
					buffer[pos+3]=0;
				}
				bitmask*=2;
				if(bitmask>255)
				{
					bitmask=1;
					readpos++;
				}
			}
		UnlockFontSurface(fontdataptr,(BYTE)count);
		fontdataptr->bmpdata[count].srcrect.left=0;
		fontdataptr->bmpdata[count].srcrect.right=fontdataptr->width;
		fontdataptr->bmpdata[count].srcrect.top=0;
		fontdataptr->bmpdata[count].srcrect.bottom=fontdataptr->height;
	}
	GlobalUnlock(hGlobal);
	if(GlobalFree(hGlobal))
	{
		DisplayErrorMessage("Failed to free font data from memory.\nSystem may become unstable.",
							"Error - CDirectDraw::LoadFontFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::ReleaseFont(FONT_DATA_PTR fontdataptr)
{
	DWORD count;
	
	for(count=0;count<73;count++)
		if(fontdataptr->bmpdata[count].bmpsurf)
		{
			fontdataptr->bmpdata[count].bmpsurf->Release();
			fontdataptr->bmpdata[count].bmpsurf=NULL;
		}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::DisplayCharacter(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	if(ActualBitsPerPixel==15)
		return(DisplayCharacter15(fontdataptr,output,x,y,newcolor,transflag));
	if(ActualBitsPerPixel==16)
		return(DisplayCharacter16(fontdataptr,output,x,y,newcolor,transflag));
	if(ActualBitsPerPixel==24)
		return(DisplayCharacter24(fontdataptr,output,x,y,newcolor,transflag));
	if(ActualBitsPerPixel==32)
		return(DisplayCharacter32(fontdataptr,output,x,y,newcolor,transflag));

	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[xc+(yc*pitch)])
					buffer[xc+(yc*pitch)]=newcolor;
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearPrimaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!DisplayBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::DisplayCharacter15(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*2)+(yc*pitch)])
				{
					long pos=(xc*2)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					BYTE blue=(clr.peBlue>>3)&31;
					BYTE green=(clr.peGreen>>3)&31;
					BYTE red=(clr.peRed>>3)&31;
					buffer[pos]=blue+((green<<5)&224);
					buffer[pos+1]=((green>>2)&3)+((red<<2)&124);
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearPrimaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!DisplayBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::DisplayCharacter16(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*2)+(yc*pitch)])
				{
					long pos=(xc*2)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					BYTE blue=(clr.peBlue>>3)&31;
					BYTE green=(clr.peGreen>>2)&63;
					BYTE red=(clr.peRed>>3)&31;
					buffer[pos]=blue+((green<<5)&224);
					buffer[pos+1]=((green>>3)&7)+((red<<3)&248);
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearPrimaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!DisplayBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::DisplayCharacter24(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*3)+(yc*pitch)])
				{
					long pos=(xc*3)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					buffer[pos]=clr.peBlue;
					buffer[pos+1]=clr.peGreen;
					buffer[pos+2]=clr.peRed;
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearPrimaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!DisplayBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::DisplayCharacter32(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<(fontdataptr->width*4);xc++)
				if(buffer[(xc*4)+(yc*pitch)])
				{
					long pos=(xc*4)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					buffer[pos]=clr.peBlue;
					buffer[pos+1]=clr.peGreen;
					buffer[pos+2]=clr.peRed;
					buffer[pos+3]=0;
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearPrimaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!DisplayBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::DisplayString(FONT_DATA_PTR fontdataptr,char* output,DWORD x,DWORD y,BYTE color,BOOL transflag)
{
	DWORD count;

	for(count=0;count<=strlen(output);count++)
		if(!DisplayCharacter(fontdataptr,
							 output[count],
							 x+(count*fontdataptr->width),
							 y,
							 color,
							 transflag))
			return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferCharacter(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	if(ActualBitsPerPixel==15)
		return(BufferCharacter15(fontdataptr,output,x,y,newcolor,transflag));
	if(ActualBitsPerPixel==16)
		return(BufferCharacter16(fontdataptr,output,x,y,newcolor,transflag));
	if(ActualBitsPerPixel==24)
		return(BufferCharacter24(fontdataptr,output,x,y,newcolor,transflag));
	if(ActualBitsPerPixel==32)
		return(BufferCharacter32(fontdataptr,output,x,y,newcolor,transflag));

	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[xc+(yc*pitch)])
					buffer[xc+(yc*pitch)]=newcolor;
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearSecondaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!BufferBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferCharacter15(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*2)+(yc*pitch)])
				{
					long pos=(xc*2)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					BYTE blue=(clr.peBlue>>3)&31;
					BYTE green=(clr.peGreen>>3)&31;
					BYTE red=(clr.peRed>>3)&31;
					buffer[pos]=blue+((green<<5)&224);
					buffer[pos+1]=((green>>2)&3)+((red<<2)&124);
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearSecondaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!BufferBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferCharacter16(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*2)+(yc*pitch)])
				{
					long pos=(xc*2)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					BYTE blue=(clr.peBlue>>3)&31;
					BYTE green=(clr.peGreen>>2)&63;
					BYTE red=(clr.peRed>>3)&31;
					buffer[pos]=blue+((green<<5)&224);
					buffer[pos+1]=((green>>3)&7)+((red<<3)&248);
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearSecondaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!BufferBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferCharacter24(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*3)+(yc*pitch)])
				{
					long pos=(xc*3)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					buffer[pos]=clr.peBlue;
					buffer[pos+1]=clr.peGreen;
					buffer[pos+2]=clr.peRed;
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearSecondaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!BufferBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferCharacter32(FONT_DATA_PTR fontdataptr,BYTE output,DWORD x,DWORD y,BYTE newcolor,BOOL transflag)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	BYTE realoutput=(BYTE)0xFF;
	
	if(output>='A'&&output<='Z')
		realoutput=(output-'A')+FTD_CAPA;
	if(output>='a'&&output<='z')
		realoutput=(output-'a')+FTD_LOWA;
	if(output>='0'&&output<='9')
		realoutput=(output-'0')+FTD_DIGT;
	if(output=='!')
		realoutput=FTD_EXCL;
	if(output=='(')
		realoutput=FTD_LPAR;
	if(output==')')
		realoutput=FTD_RPAR;
	if(output=='-')
		realoutput=FTD_DASH;
	if(output==':')
		realoutput=FTD_COLN;
	if(output==';')
		realoutput=FTD_SEMI;
	if(output=='^')
		realoutput=FTD_QUOT;
	if(output=='~')
		realoutput=FTD_APOS;
	if(output==',')
		realoutput=FTD_COMA;
	if(output=='.')
		realoutput=FTD_PERD;
	if(output=='?')
		realoutput=FTD_QUES;
	if(realoutput==0xFF)
		return(TRUE);
	if(newcolor)
	{
		LockFontSurface(fontdataptr,realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<(fontdataptr->width*4);xc++)
				if(buffer[(xc*4)+(yc*pitch)])
				{
					long pos=(xc*4)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					buffer[pos]=clr.peBlue;
					buffer[pos+1]=clr.peGreen;
					buffer[pos+2]=clr.peRed;
					buffer[pos+3]=0;
				}
		UnlockFontSurface(fontdataptr,realoutput);
	}
	if(!transflag)
		ClearSecondaryRect(x,y,x+fontdataptr->width,y+fontdataptr->height);
	if(!BufferBitmap(&fontdataptr->bmpdata[realoutput],x,y))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::BufferString(FONT_DATA_PTR fontdataptr,char* output,DWORD x,DWORD y,BYTE color,BOOL transflag)
{
	DWORD count;

	for(count=0;count<=strlen(output);count++)
		if(!BufferCharacter(fontdataptr,
							output[count],
							x+(count*fontdataptr->width),
							y,
							color,
							transflag))
			return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::SetFontColor(FONT_DATA_PTR fontdataptr,DWORD newcolor)
{
	if(ActualBitsPerPixel==15)
		return(SetFontColor15(fontdataptr,newcolor));
	if(ActualBitsPerPixel==16)
		return(SetFontColor16(fontdataptr,newcolor));
	if(ActualBitsPerPixel==24)
		return(SetFontColor24(fontdataptr,newcolor));
	if(ActualBitsPerPixel==32)
		return(SetFontColor32(fontdataptr,newcolor));
	
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	DWORD realoutput;

	for(realoutput=0;realoutput<73;realoutput++)
	{
		LockFontSurface(fontdataptr,(BYTE)realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[xc+(yc*pitch)])
					buffer[xc+(yc*pitch)]=(BYTE)newcolor;
		UnlockFontSurface(fontdataptr,(BYTE)realoutput);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::SetFontColor15(FONT_DATA_PTR fontdataptr,DWORD newcolor)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	DWORD realoutput;

	for(realoutput=0;realoutput<73;realoutput++)
	{
		LockFontSurface(fontdataptr,(BYTE)realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*2)+(yc*pitch)])
				{
					long pos=(xc*2)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					BYTE blue=(clr.peBlue>>3)&31;
					BYTE green=(clr.peGreen>>3)&31;
					BYTE red=(clr.peRed>>3)&31;
					buffer[pos]=blue+((green<<5)&224);
					buffer[pos+1]=((green>>2)&3)+((red<<2)&124);
				}
		UnlockFontSurface(fontdataptr,(BYTE)realoutput);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::SetFontColor16(FONT_DATA_PTR fontdataptr,DWORD newcolor)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	DWORD realoutput;

	for(realoutput=0;realoutput<73;realoutput++)
	{
		LockFontSurface(fontdataptr,(BYTE)realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*2)+(yc*pitch)])
				{
					long pos=(xc*2)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					BYTE blue=(clr.peBlue>>3)&31;
					BYTE green=(clr.peGreen>>2)&63;
					BYTE red=(clr.peRed>>3)&31;
					buffer[pos]=blue+((green<<5)&224);
					buffer[pos+1]=((green>>3)&7)+((red<<3)&248);
				}
		UnlockFontSurface(fontdataptr,(BYTE)realoutput);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::SetFontColor24(FONT_DATA_PTR fontdataptr,DWORD newcolor)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	DWORD realoutput;

	for(realoutput=0;realoutput<73;realoutput++)
	{
		LockFontSurface(fontdataptr,(BYTE)realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<fontdataptr->width;xc++)
				if(buffer[(xc*3)+(yc*pitch)])
				{
					long pos=(xc*3)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					buffer[pos]=clr.peBlue;
					buffer[pos+1]=clr.peGreen;
					buffer[pos+2]=clr.peRed;
				}
		UnlockFontSurface(fontdataptr,(BYTE)realoutput);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::SetFontColor32(FONT_DATA_PTR fontdataptr,DWORD newcolor)
{
	DWORD xc;
	DWORD yc;
	LPSTR buffer;
	DWORD pitch;
	DWORD realoutput;

	for(realoutput=0;realoutput<73;realoutput++)
	{
		LockFontSurface(fontdataptr,(BYTE)realoutput,&buffer,&pitch);
		for(yc=0;yc<fontdataptr->height;yc++)
			for(xc=0;xc<(fontdataptr->width*4);xc++)
				if(buffer[(xc*4)+(yc*pitch)])
				{
					long pos=(xc*4)+(yc*pitch);
					PALETTEENTRY clr=Palette[newcolor];
					buffer[pos]=clr.peBlue;
					buffer[pos+1]=clr.peGreen;
					buffer[pos+2]=clr.peRed;
					buffer[pos+3]=0;
				}
		UnlockFontSurface(fontdataptr,(BYTE)realoutput);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::FadePaletteOut(DWORD start,DWORD num,DWORD length,BOOL PrepareFlag)
{
	static DWORD TotalLength;
	static DWORD CurrentStep;
	DWORD count;
	float fCurrentStep;
	float fTotalLength;
	float fPercent;
	float fRed;
	float fGreen;
	float fBlue;
	BYTE red;
	BYTE green;
	BYTE blue;
		
	if(PrepareFlag)
	{
		CopyMemory((LPVOID)&SavePalette[start],
				   (LPVOID)&Palette[start],
				   num*sizeof(PALETTEENTRY));
		TotalLength=length;
		CurrentStep=length;
		return(TRUE);
	}
	if(CurrentStep==0)
		return(FALSE);
	CurrentStep--;
	for(count=start;count<start+num;count++)
	{
		fCurrentStep=(float)CurrentStep;
		fTotalLength=(float)TotalLength;
		fPercent=fCurrentStep/fTotalLength;
		fRed=(float)SavePalette[count].peRed;
		fGreen=(float)SavePalette[count].peGreen;
		fBlue=(float)SavePalette[count].peBlue;
		fRed*=fPercent;
		fGreen*=fPercent;
		fBlue*=fPercent;
		red=(BYTE)fRed;
		green=(BYTE)fGreen;
		blue=(BYTE)fBlue;
		SetDirectDrawPaletteEntry((BYTE)count,red,green,blue);
	}
	UpdateDirectDrawPalette(start,start+num);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::FadePaletteIn(DWORD start,DWORD num,DWORD length,BOOL PrepareFlag)
{
	static DWORD TotalLength;
	static DWORD CurrentStep;
	DWORD count;
	float fCurrentStep;
	float fTotalLength;
	float fPercent;
	float fRed;
	float fGreen;
	float fBlue;
	BYTE red;
	BYTE green;
	BYTE blue;
		
	if(PrepareFlag)
	{
		CopyMemory((LPVOID)&SavePalette[start],
				   (LPVOID)&Palette[start],
				   num*sizeof(PALETTEENTRY));
		ZeroMemory((LPVOID)&Palette[start],
				   num*sizeof(PALETTEENTRY));
		TotalLength=length;
		CurrentStep=0;
		if(!UpdateDirectDrawPalette(start,start+num))
			return(FALSE);
		return(TRUE);
	}
	if(CurrentStep==TotalLength)
		return(FALSE);
	CurrentStep++;
	for(count=start;count<start+num;count++)
	{
		fCurrentStep=(float)CurrentStep;
		fTotalLength=(float)TotalLength;
		fPercent=fCurrentStep/fTotalLength;
		fRed=(float)SavePalette[count].peRed;
		fGreen=(float)SavePalette[count].peGreen;
		fBlue=(float)SavePalette[count].peBlue;
		fRed*=fPercent;
		fGreen*=fPercent;
		fBlue*=fPercent;
		red=(BYTE)fRed;
		green=(BYTE)fGreen;
		blue=(BYTE)fBlue;
		SetDirectDrawPaletteEntry((BYTE)count,red,green,blue);
	}
	UpdateDirectDrawPalette(start,start+num);
	return(TRUE);
}

BOOL _fastcall CDirectDraw::LockFontSurface(FONT_DATA_PTR fontdataptr,BYTE realoutput,LPSTR *buffer,LPDWORD pitch)
{
	if(!LockBitmapSurface(&fontdataptr->bmpdata[realoutput],
						  buffer,
						  pitch))
	{
		DisplayErrorMessage("Failed to lock font surface.",
							"Error - CDirectDraw::LockFontSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectDraw::UnlockFontSurface(FONT_DATA_PTR fontdataptr,BYTE realoutput)
{
	if(!UnlockBitmapSurface(&fontdataptr->bmpdata[realoutput]))
	{
		DisplayErrorMessage("Failed to unlock font surface.",
							"Error - CDirectDraw::UnlockFontSurface()",
							MB_OK|MB_ICONINFORMATION,
							this);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectDraw::Release(void)
{
	if(lpDirectDrawClipper)
	{
		lpDirectDrawClipper->Release();
		lpDirectDrawClipper=NULL;
	}
	if(lpPrimaryDirectDrawSurface)
	{
		lpPrimaryDirectDrawSurface->Release();
		lpPrimaryDirectDrawSurface=NULL;
	}
	if(SavePalette)
	{
		GlobalUnlock(SavePaletteMem);
		SavePalette=NULL;
	}
	if(SavePaletteMem)
	{
		if(GlobalFree(SavePaletteMem))
			DisplayErrorMessage("Failed to free stored palette memory.\nSystem may become unstable.",
								"Error - CDirectDraw::~CDirectDraw()",
								MB_OK|MB_ICONINFORMATION,
								this);
		else
			SavePaletteMem=NULL;
	}
	if(Palette)
	{
		GlobalUnlock(PaletteMem);
		Palette=NULL;
	}
	if(PaletteMem)
	{
		if(GlobalFree(PaletteMem))
			DisplayErrorMessage("Failed to free primary palette memory.\nSystem may become unstable.",
								"Error - CDirectDraw::~CDirectDraw()",
								MB_OK|MB_ICONINFORMATION,
								this);
		else
			PaletteMem=NULL;
	}
	if(lpDirectDraw)
	{
		lpDirectDraw->Release();
		lpDirectDraw=NULL;
	}
	return(TRUE);
}

// MISCELLANEOUS FUNCTIONS //

void DisplayErrorMessage(char *message,char *caption,DWORD flags,CDirectDraw *dd)
{
	char s[256];
	DWORD error=GetLastError();

	if(dd)
		if(dd->IsPrimaryDirectDrawSurfaceLocked())
			dd->UnlockPrimaryDirectDrawSurface();
	ShowCursor(TRUE);
	MessageBox(APP_HWND,message,caption,flags);
	if(error)
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					  NULL,
				      error,
				      NULL,
				      s,
				      255,
				      NULL);
		MessageBox(APP_HWND,s,"Extended Error Information",MB_OK|MB_ICONINFORMATION);
	}
	ShowCursor(FALSE);
	SetLastError(NULL);
}

BOOL OpenDataFile(char *filename,CDirectDraw* dd,BOOL Verify)
{
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	if(Verify)
		DataFile=CreateFile(GlobalStr,
							GENERIC_READ,
							NULL,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL|
							FILE_FLAG_RANDOM_ACCESS,
							NULL);
	else
		DataFile=CreateFile(GlobalStr,
							GENERIC_READ,
							NULL,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL|
							FILE_FLAG_SEQUENTIAL_SCAN,
							NULL);
	if(DataFile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Failed to open data file 'intro.dat'.",
							"Fatal Error - OpenDataFile()",
							MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CloseDataFile(CDirectDraw* dd)
{
	if(!DataFile)
		return(TRUE);
	if(!CloseHandle(DataFile))
	{
		DisplayErrorMessage("Failed to close data file.\nSystem may become unstable.",
							"Error - CloseDataFile()",
							MB_OK|MB_ICONINFORMATION,
							dd);
		return(FALSE);
	}
	DataFile=NULL;
	return(TRUE);
}

BOOL SetDefaultFilePath(char *filename)
{
	DefaultFilePath=filename;
	return(TRUE);
}

BOOL VerifyDataFiles(char* filename,CDirectDraw* dd)
{
	BYTE DataType;
	BYTE Input;
	DWORD BytesRead;
	DWORD DataPos;
	char DataFileName[13];
		
	if(!OpenVerificationFile(filename,dd))
		return(FALSE);
	if(!ReadFile(VerificationFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read initial record file data type.",
							"Error - VerifyDataFiles()",
							MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	while(DataType!=DTF_IDEOF)
	{
		switch(DataType)
		{
		case(DTF_IDNEWFILE):
			CloseDataFile(dd);
			DataPos=0;
			if(!ReadFile(VerificationFile,
						 (LPVOID)&Input,
						 sizeof(BYTE),
						 &BytesRead,
						 NULL))
			{
				DisplayErrorMessage("Failed to read initial record filename data.",
									"Error - VerifyDataFiles()",
									MB_OK|MB_ICONSTOP,
									dd);
				return(FALSE);
			}
			while(Input)
			{
				DataFileName[DataPos++]=Input;
				if(!ReadFile(VerificationFile,
							 (LPVOID)&Input,
							 sizeof(BYTE),
							 &BytesRead,
							 NULL))
				{
					DisplayErrorMessage("Failed to read record filename data.",
										"Error - VerifyDataFiles()",
										MB_OK|MB_ICONSTOP,
										dd);
					return(FALSE);
				}
			}
			DataFileName[DataPos]=0;
			if(!OpenDataFile(DataFileName,dd,TRUE))
				return(FALSE);
			break;
		default:
			if(!ReadFile(VerificationFile,
						 (LPVOID)&DataPos,
						 sizeof(DWORD),
						 &BytesRead,
						 NULL))
			{
				DisplayErrorMessage("Failed to read record file check position.",
									"Error - VerifyDataFiles()",
									MB_OK|MB_ICONSTOP,
									dd);
				return(FALSE);
			}
			if(SetFilePointer(DataFile,
							  (DWORD)DataPos,
							  NULL,
							  FILE_BEGIN)!=DataPos)
			{
				DisplayErrorMessage("Failed to seek to data file check position.",
									"Error - VerifyDataFiles()",
									MB_OK|MB_ICONSTOP,
									dd);
				return(FALSE);
			}			
			if(!ReadFile(DataFile,
						 (LPVOID)&Input,
						 sizeof(BYTE),
						 &BytesRead,
						 NULL))
			{
				DisplayErrorMessage("Failed to read data file identifier.",
									"Error - VerifyDataFiles()",
									MB_OK|MB_ICONSTOP,
									dd);
				return(FALSE);
			}
			if(DataType!=Input)
			{
				DisplayErrorMessage("Invalid data in file.",
									"Error - VerifyDataFiles()",
									MB_OK|MB_ICONSTOP,
									dd);
				return(FALSE);
			}
			break;
		}
		if(!ReadFile(VerificationFile,
					 (LPVOID)&DataType,
					 sizeof(BYTE),
					 &BytesRead,
					 NULL))
		{
			DisplayErrorMessage("Failed to read record file data type.",
								"Error - VerifyDataFiles()",
								MB_OK|MB_ICONSTOP,
								dd);
			return(FALSE);
		}
	}
	CloseDataFile(dd);
	CloseVerificationFile(dd);
	return(TRUE);
}

BOOL OpenVerificationFile(char* filename,CDirectDraw *dd)
{
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	VerificationFile=CreateFile(GlobalStr,
								GENERIC_READ,
								NULL,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL|
								FILE_FLAG_SEQUENTIAL_SCAN,
								NULL);
	if(VerificationFile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Failed to open verifiaction file.",
							"Error - OpenVerificationFile()",
							MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CloseVerificationFile(CDirectDraw *dd)
{
	if(!VerificationFile)
		return(TRUE);
	if(!CloseHandle(VerificationFile))
	{
		DisplayErrorMessage("Failed to close verifiaction file.\nSystem may become unstable.",
							"Error - CloseVerificationFile()",
							MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	VerificationFile=NULL;
	return(TRUE);
}

BOOL inline _fastcall Swap(long *a,long *b)
{
	long temp;

	temp=*a;
	*a=*b;
	*b=temp;
	return(TRUE);
}

BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE lpdid,LPVOID guidptr)
{
	static BOOL Initialized=FALSE;
	HGLOBAL hGlobalTemp;

	if(lpdid)
	{
		if(!Initialized)
		{
			JoystickListMem=GlobalAlloc(GHND,
										sizeof(GUID)+41);
			if(!JoystickListMem)
			{
				DisplayErrorMessage("Failed to allocate memory for joystick data.\nJoystick list may be incomplete.",
									"Error - CDirectInput::EnumJoysticks()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(DIENUM_CONTINUE);
			}
			JoystickList=(LPSTR)GlobalLock(JoystickListMem);
			if(!JoystickList)
			{
				DisplayErrorMessage("Failed to lock memory for joystick data.\nJoystick list may be incomplete.",
									"Error - CDirectInput::EnumJoysticks()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(DIENUM_CONTINUE);
			}
			CopyMemory(JoystickList,
					   &lpdid->guidInstance,
					   sizeof(GUID));
			CopyMemory(JoystickList+sizeof(GUID),
					   &lpdid->tszProductName,
					   40);
			JoystickList[sizeof(GUID)+40]=0;
			Initialized=TRUE;
			JoystickCount=1;
		}
		else
		{
			GlobalUnlock(JoystickListMem);
			hGlobalTemp=GlobalReAlloc(JoystickListMem,
									  (sizeof(GUID)+41)*(JoystickCount+1),
									  NULL);
			if(!hGlobalTemp)
			{
				DisplayErrorMessage("Failed to re-allocate memory for joystick data.\nJoystick list may be incomplete.",
									"Error - CDirectInput::EnumJoysticks()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(DIENUM_CONTINUE);
			}
			JoystickListMem=hGlobalTemp;
			JoystickList=(LPSTR)GlobalLock(JoystickListMem);
			if(!JoystickList)
			{
				DisplayErrorMessage("Failed to re-lock memory for joystick data.\nJoystick list may be incomplete.",
									"Error - CDirectInput::EnumJoysticks()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(DIENUM_CONTINUE);
			}
			CopyMemory(JoystickList+((sizeof(GUID)+41)*(JoystickCount)),
					   &lpdid->guidInstance,
					   sizeof(GUID));
			CopyMemory(JoystickList+((sizeof(GUID)+41)*(JoystickCount))+sizeof(GUID),
					   &lpdid->tszProductName,
					   40);
			JoystickList[((sizeof(GUID)+41)*(JoystickCount))+sizeof(GUID)+40]=0;
			JoystickCount++;
		}
	}
	else
	{
		if(Initialized)
		{
			if(JoystickList)
				GlobalUnlock(JoystickListMem);
			if(JoystickListMem)
				if(GlobalFree(JoystickListMem))
				{
					DisplayErrorMessage("Failed to free memory for joystick list.\nSystem may become unstable.",
										"Error - CDirectInput::EnumJoysticks()",
										MB_OK|MB_ICONINFORMATION,
										NULL);
					return(DIENUM_CONTINUE);
				}
		}
	}
	return(DIENUM_CONTINUE);
}

// DIRECTDRAW ENCODING FUNCTIONS //

#ifdef ENCODE_DATA
#define READFILE(InputFile,input,BytesRead) if(!ReadFile(InputFile,(LPVOID)&input,sizeof(input),&BytesRead,NULL)) { DisplayErrorMessage("Encode error 3","Error - EncodeFont()",MB_OK|MB_ICONSTOP,dd); CloseHandle(InputFile); return(FALSE); }
#include<io.h>
#include<malloc.h>
#define BITMAP_ID 0x4D42
#define MAX_COLORS_PALETTE 256
typedef struct BITMAP_FILE_TAG
        {
        BITMAPFILEHEADER bitmapfileheader;
        BITMAPINFOHEADER bitmapinfoheader;
        PALETTEENTRY palette[256];
        UCHAR *buffer;
        } BITMAP_FILE, *BITMAP_FILE_PTR;
int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height);
int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename);
int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap);
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }
int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename)
{
int file_handle,
    index;
UCHAR *temp_buffer=NULL;
OFSTRUCT file_data;
if((file_handle=OpenFile(filename,&file_data,OF_READ))==-1)
   return(0);
_lread(file_handle, &bitmap->bitmapfileheader,sizeof(BITMAPFILEHEADER));
if (bitmap->bitmapfileheader.bfType!=BITMAP_ID)
   {
   _lclose(file_handle);
   return(0);
   }
_lread(file_handle, &bitmap->bitmapinfoheader,sizeof(BITMAPINFOHEADER));
if (bitmap->bitmapinfoheader.biBitCount == 8)
   {
   _lread(file_handle, &bitmap->palette,MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));
   for (index=0; index < MAX_COLORS_PALETTE; index++)
       {
       int temp_color=bitmap->palette[index].peRed;
       bitmap->palette[index].peRed=bitmap->palette[index].peBlue;
       bitmap->palette[index].peBlue=temp_color;
       bitmap->palette[index].peFlags=PC_NOCOLLAPSE;
       }
    }
_lseek(file_handle,-(int)(bitmap->bitmapinfoheader.biSizeImage),SEEK_END);
if (bitmap->bitmapinfoheader.biBitCount==8 || bitmap->bitmapinfoheader.biBitCount==16 || 
    bitmap->bitmapinfoheader.biBitCount==24)
   {
   if (!(bitmap->buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
      {
      _lclose(file_handle);
      return(0);
      }
   _lread(file_handle,bitmap->buffer,bitmap->bitmapinfoheader.biSizeImage);
   }
else
   {
   return(0);
   }
_lclose(file_handle);
Flip_Bitmap(bitmap->buffer, 
            bitmap->bitmapinfoheader.biWidth*(bitmap->bitmapinfoheader.biBitCount/8), 
            bitmap->bitmapinfoheader.biHeight);
return(1);
}
int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap)
{
if (bitmap->buffer)
   {
   free(bitmap->buffer);
   bitmap->buffer = NULL;
   }
return(1);
}
int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height)
{
UCHAR *buffer;
int index;
if(!(buffer=(UCHAR*)malloc(bytes_per_line*height)))
   return(0);
memcpy(buffer,image,bytes_per_line*height);
for (index=0; index < height; index++)
    memcpy(&image[((height-1) - index)*bytes_per_line],
           &buffer[index*bytes_per_line], bytes_per_line);
free(buffer);
return(1);
}

HANDLE OutputFile;
DWORD WritePos;
HANDLE RecordFile;

BOOL OpenEncoder(char *filename)
{
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	OutputFile=CreateFile(GlobalStr,
						  GENERIC_WRITE,
						  NULL,
						  NULL,
						  CREATE_ALWAYS,
						  FILE_ATTRIBUTE_NORMAL,
						  NULL);
	if(OutputFile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Encode error",
				   "Error - OpenEncoder()",
				   MB_OK|MB_ICONSTOP,
					NULL);
		return(FALSE);
	}
	DWORD BytesWritten;
	BYTE output;
	output=DTF_IDNEWFILE;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - OpenEncoder()",
				   MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	if(!WriteFile(RecordFile,
				  filename,
				  strlen(filename)+1,
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 2",
				   "Error - OpenEncoder()",
				   MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	return(TRUE);
}

BOOL EncodePalette(char *filename,CDirectDraw* dd)
{
	BITMAP_FILE bmpfile;
	DWORD BytesWritten;
	BYTE output;
	output=DTF_IDPALETTE;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - EncodePalette()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	WritePos=SetFilePointer(OutputFile,0,NULL,FILE_CURRENT);
	if(!WriteFile(RecordFile,
				  (char*)&WritePos,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 2",
				   "Error - EncodePalette()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	if(!Load_Bitmap_File(&bmpfile,filename))
	{
		DisplayErrorMessage("Encode error 1",
				   "Error - EncodePalette()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	for(int count=0;count<256;count++)
		dd->SetDirectDrawPaletteEntry(count,
									  bmpfile.palette[count].peRed,
									  bmpfile.palette[count].peGreen,
									  bmpfile.palette[count].peBlue);
//	dd->UpdateDirectDrawPalette(0,256);
	output=DTF_IDPALETTE;
	if(!WriteFile(OutputFile,
				  (char*)&output,
				  sizeof(char),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error A",
				   "Error - EncodePalette()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	for(count=0;count<256;count++)
	{
		if(!WriteFile(OutputFile,
						  (char*)&bmpfile.palette[count],
						  3,
						  &BytesWritten,
						  NULL))
			{
				DisplayErrorMessage("Encode error 3",
						   "Error - EncodePalette()",
						   MB_OK|MB_ICONSTOP,
								dd);
				return(FALSE);
			}
	}
	if(!Unload_Bitmap_File(&bmpfile))
	{
		DisplayErrorMessage("Encode error 4",
				   "Error - EncodePalette()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	DisplayErrorMessage("Palette successfully encoded.",
			   "Information",
			   MB_OK|MB_ICONINFORMATION,
							dd);
	return(TRUE);
}

BOOL EncodeBitmap(char *filename,CDirectDraw* dd)
{
	BITMAP_FILE bmpfile;
//	LPSTR vd;
//	DWORD p;
	DWORD StopPos;
	DWORD ReadPos;
	DWORD ReadPos2;
	DWORD count;
	DWORD BytesWritten;
	BYTE output=DTF_IDBITMAP;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - EncodeBitmap()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	WritePos=SetFilePointer(OutputFile,0,NULL,FILE_CURRENT);
	if(!WriteFile(RecordFile,
				  (char*)&WritePos,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 2",
				   "Error - EncodeBitmap()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	if(!Load_Bitmap_File(&bmpfile,filename))
	{
		DisplayErrorMessage("Encode error 1",
				   "Error - EncodeBitmap()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
//	dd->LockPrimaryDirectDrawSurface(&vd,&p);
//	long x,y;
//	for(y=0;y<bmpfile.bitmapinfoheader.biHeight;y++)
//		for(x=0;x<bmpfile.bitmapinfoheader.biWidth;x++)
//			vd[x+(y*p)]=bmpfile.buffer[x+(y*bmpfile.bitmapinfoheader.biWidth)];
//	dd->UnlockPrimaryDirectDrawSurface();
	output=DTF_IDBITMAP;
	if(!WriteFile(OutputFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error A",
				   "Error - EncodeBitmap()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	if(!WriteFile(OutputFile,
				  (char*)&bmpfile.bitmapinfoheader.biWidth,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error 2",
				   "Error - EncodeBitmap()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	if(!WriteFile(OutputFile,
				  (char*)&bmpfile.bitmapinfoheader.biHeight,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error 3",
				   "Error - EncodeBitmap()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	ReadPos=0;
	char ExitFlag=0;
	StopPos=bmpfile.bitmapinfoheader.biWidth*
			bmpfile.bitmapinfoheader.biHeight;
	while(ReadPos<StopPos)
	{
		count=0;
		while(bmpfile.buffer[ReadPos]==0)
			if(ReadPos>=StopPos)
			{
				ExitFlag=1;
				break;
			}
			else
			{
				ReadPos++;
				count++;
			}
		if(ExitFlag)
			break;
		if(count>255)
			while(count>255)
			{
				output=255;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 4",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				output=0;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 5",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				count-=255;
			}
		output=(BYTE)count;
		if(!WriteFile(OutputFile,
					  (char*)&output,
					  sizeof(BYTE),
					  &BytesWritten,
					  NULL))
		{
			DisplayErrorMessage("Encode error 6",
					   "Error - EncodeBitmap()",
					   MB_OK|MB_ICONSTOP,
							dd);
			return(FALSE);
		}
		if(ReadPos>=StopPos)
			break;
		ReadPos2=ReadPos;
		count=0;
		while(bmpfile.buffer[ReadPos]!=0)
			if(ReadPos>=StopPos)
			{
				ExitFlag=2;
				break;
			}
			else
			{
				ReadPos++;
				count++;
			}
		if(ExitFlag)
			break;
		if(count>255)
			while(count>255)
			{
				output=255;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 7",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				if(!WriteFile(OutputFile,
							  ((char*)bmpfile.buffer)+ReadPos2,
							  255,
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 8",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				output=0;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 9",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				ReadPos2+=255;
				count-=255;
			}
		output=(BYTE)count;
		if(!WriteFile(OutputFile,
					  (char*)&output,
					  sizeof(BYTE),
					  &BytesWritten,
					  NULL))
		{
			DisplayErrorMessage("Encode error 10",
					   "Error - EncodeBitmap()",
					   MB_OK|MB_ICONSTOP,
							dd);
			return(FALSE);
		}
		if(count)
		{
			if(!WriteFile(OutputFile,
						  ((char*)bmpfile.buffer)+ReadPos2,
						  count,
						  &BytesWritten,
						  NULL))
			{
				DisplayErrorMessage("Encode error 11",
						   "Error - EncodeBitmap()",
						   MB_OK|MB_ICONSTOP,
							dd);
				return(FALSE);
			}
		}
	}
	if(ExitFlag==1)
	{
		if(count>255)
			while(count>255)
			{
				output=255;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 12",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				output=0;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 13",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				count-=255;
			}
		output=(BYTE)count;
		if(!WriteFile(OutputFile,
					  (char*)&output,
					  sizeof(BYTE),
					  &BytesWritten,
					  NULL))
		{
			DisplayErrorMessage("Encode error 14",
					   "Error - EncodeBitmap()",
					   MB_OK|MB_ICONSTOP,
							dd);
			return(FALSE);
		}
	}
	else if(ExitFlag==2)
	{
		if(count>255)
			while(count>255)
			{
				output=255;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 15",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				if(!WriteFile(OutputFile,
							  ((char*)bmpfile.buffer)+ReadPos2,
							  255,
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 16",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				output=0;
				if(!WriteFile(OutputFile,
							  (char*)&output,
							  sizeof(BYTE),
							  &BytesWritten,
							  NULL))
				{
					DisplayErrorMessage("Encode error 17",
							   "Error - EncodeBitmap()",
							   MB_OK|MB_ICONSTOP,
							dd);
					return(FALSE);
				}
				ReadPos2+=255;
				count-=255;
			}
		output=(BYTE)count;
		if(!WriteFile(OutputFile,
					  (char*)&output,
					  sizeof(BYTE),
					  &BytesWritten,
					  NULL))
		{
			DisplayErrorMessage("Encode error 18",
					   "Error - EncodeBitmap()",
					   MB_OK|MB_ICONSTOP,
							dd);
			return(FALSE);
		}
		if(count)
		{
			if(!WriteFile(OutputFile,
						  ((char*)bmpfile.buffer)+ReadPos2,
						  count,
						  &BytesWritten,
						  NULL))
			{
				DisplayErrorMessage("Encode error 19",
						   "Error - EncodeBitmap()",
						   MB_OK|MB_ICONSTOP,
							dd);
				return(FALSE);
			}
		}
	}
	if(!Unload_Bitmap_File(&bmpfile))
	{
		DisplayErrorMessage("Encode error 20",
				   "Error - EncodeBitmap()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	DisplayErrorMessage("Bitmap successfully encoded.",
			   "Information",
			   MB_OK|MB_ICONINFORMATION,
							dd);
	return(TRUE);
}

BOOL CloseEncoder(void)
{
	if(!CloseHandle(OutputFile))
	{
		DisplayErrorMessage("File close failure.",
				   "CloseEncoder()",
				   MB_OK,
							NULL);
	}
	return(TRUE);
}

BOOL OpenRecorder(char *filename)
{
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	RecordFile=CreateFile(GlobalStr,
						  GENERIC_WRITE,
						  NULL,
						  NULL,
						  CREATE_ALWAYS,
						  FILE_ATTRIBUTE_NORMAL,
						  NULL);
	if(RecordFile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Record error",
				   "Error - OpenRecorder()",
				   MB_OK|MB_ICONSTOP,
					NULL);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CloseRecorder(void)
{
	DWORD BytesWritten;
	BYTE output=DTF_IDEOF;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - CloseRecorder()",
				   MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	if(!CloseHandle(RecordFile))
	{
		DisplayErrorMessage("File close failure.",
				   "CloseRecorder()",
				   MB_OK,
							NULL);
	}
	return(TRUE);
}

BOOL EncodeFont(char *filename,CDirectDraw* dd)
{
	DWORD BytesRead;
	HANDLE InputFile=NULL;
	BYTE output=DTF_IDFONT;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesRead,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - EncodeFont()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	WritePos=SetFilePointer(OutputFile,0,NULL,FILE_CURRENT);
	if(!WriteFile(RecordFile,
				  (char*)&WritePos,
				  sizeof(DWORD),
				  &BytesRead,
				  NULL))
	{
		DisplayErrorMessage("Record error 2",
				   "Error - EncodeFont()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	InputFile=CreateFile(GlobalStr,
						 GENERIC_READ,
						 NULL,
						 NULL,
						 OPEN_EXISTING,
						 FILE_ATTRIBUTE_NORMAL|
						 FILE_FLAG_SEQUENTIAL_SCAN,
						 NULL);
	if(InputFile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Encode Error 1",
				   "Error - EncodeFont()",
				   MB_OK|MB_ICONSTOP,
					NULL);
		return(FALSE);
	}
	output=DTF_IDFONT;
	if(!WriteFile(OutputFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesRead,
				  NULL))
	{
		DisplayErrorMessage("Encode error 2",
				   "Error - EncodeFont()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	BOOL done=FALSE;
	BYTE input;
	char command[3];
	WORD output2;
	WORD bitnum;
	BOOL dataread=FALSE;
	BYTE output3;
	DWORD width,height;
	DWORD BytesWritten=0;

	READFILE(InputFile,input,BytesRead);
	while(!done)
	{
		if(input=='/')
		{
			READFILE(InputFile,input,BytesRead);
			if(input=='/')
			{
				READFILE(InputFile,input,BytesRead);
				command[0]=input;
				READFILE(InputFile,input,BytesRead);
				command[1]=input;
				command[2]=0;
				if(strcmp(command,"BC")==0)
				{
ReReadComment:;
					while(input!='/')
						READFILE(InputFile,input,BytesRead);
					READFILE(InputFile,input,BytesRead);
					if(input=='/')
					{
						READFILE(InputFile,input,BytesRead);
						command[0]=input;
						READFILE(InputFile,input,BytesRead);
						command[1]=input;
						command[2]=0;
						if(strcmp(command,"EC")!=0)
							goto ReReadComment;
					}
					else
						goto ReReadComment;
				}
				if(strcmp(command,"SZ")==0)
				{
					while(input<'0'||input>'9')
						READFILE(InputFile,input,BytesRead);
					width=(DWORD)((input-'0')*10);
					READFILE(InputFile,input,BytesRead);
					while(input<'0'||input>'9')
						READFILE(InputFile,input,BytesRead);
					width+=(DWORD)(input-'0');
					READFILE(InputFile,input,BytesRead);
					while(input<'0'||input>'9')
						READFILE(InputFile,input,BytesRead);
					height=(DWORD)((input-'0')*10);
					READFILE(InputFile,input,BytesRead);
					while(input<'0'||input>'9')
						READFILE(InputFile,input,BytesRead);
					height+=(DWORD)(input-'0');
					if(!WriteFile(OutputFile,
								  (LPVOID)&width,
								  sizeof(DWORD),
								  &BytesRead,
								  NULL))
					{
						DisplayErrorMessage("Encode Error A",
								   "Error - EncodeFont()",
								   MB_OK|MB_ICONSTOP,
									NULL);
						CloseHandle(InputFile);
						return(FALSE);
					}
					if(!WriteFile(OutputFile,
								  (LPVOID)&height,
								  sizeof(DWORD),
								  &BytesRead,
								  NULL))
					{
						DisplayErrorMessage("Encode Error B",
								   "Error - EncodeFont()",
								   MB_OK|MB_ICONSTOP,
									NULL);
						CloseHandle(InputFile);
						return(FALSE);
					}
				}
				if(strcmp(command,"BD")==0)
				{
					output2=0;
					bitnum=0;
					dataread=TRUE;
				}
				if(strcmp(command,"EF")==0)
				{
					if(bitnum)
					{
						BytesWritten++;
						output3=(char)output2;
						if(!WriteFile(OutputFile,
									  &output3,
									  sizeof(BYTE),
									  &BytesRead,
									  NULL))
						{
							DisplayErrorMessage("Encode Error 4a",
									   "Error - EncodeFont()",
									   MB_OK|MB_ICONSTOP,
										NULL);
							CloseHandle(InputFile);
							return(FALSE);
						}
					}
					dataread=FALSE;
					done=1;
				}
			}
		}
		else
			READFILE(InputFile,input,BytesRead);
		if(dataread)
		{
			if(input=='0')
			{
				output2+=(1<<bitnum);
				bitnum++;
			}	
			else if(input=='.')
				bitnum++;
			if(bitnum>7)
			{
				BytesWritten++;
				output3=(char)output2;
				output2=0;
				bitnum=0;
				if(!WriteFile(OutputFile,
							  &output3,
							  sizeof(BYTE),
							  &BytesRead,
							  NULL))
				{
					DisplayErrorMessage("Encode Error 4",
							   "Error - EncodeFont()",
							   MB_OK|MB_ICONSTOP,
								NULL);
					CloseHandle(InputFile);
					return(FALSE);
				}
			}
		}
	}
	CloseHandle(InputFile);
	MessageBox(APP_HWND,
			   "Font successfully encoded.",
			   "Information",
			   MB_OK|MB_ICONINFORMATION);
	return(TRUE);
}
#endif

// DIRECTINPUT FUNCTIONS //

BOOL CDirectInput::Initialize(CDirectDraw* lpdd)
{
	lpDirectInput=NULL;
	lpKeyboard=NULL;
	KeyStateMem=NULL;
	KeyState=NULL;
	JoystickListMem=NULL;
	JoystickList=NULL;
	JoystickCount=0;
	JoystickActive=FALSE;
	lpJoystick=NULL;
	lpDirectDraw=lpdd;
	if(FAILED(DirectInputCreate(APP_HINST,
								DIRECTINPUT_VERSION,
								&lpDirectInput,
								NULL)))
	{
		DisplayErrorMessage("Could create DirectInput object.",
							"Error - CDirectInput::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(FAILED(lpDirectInput->CreateDevice(GUID_SysKeyboard,
										  &lpKeyboard,
										  NULL)))
	{
		DisplayErrorMessage("Could not create DirectDraw keyboard interface.",
							"Error - CDirectInput::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(FAILED(lpKeyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		DisplayErrorMessage("Could not set data format for DirectInput keyboard interface.",
							"Error - CDirectInput::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(FAILED(lpKeyboard->SetCooperativeLevel(APP_HWND,
											  DISCL_FOREGROUND|
											  DISCL_NONEXCLUSIVE)))
	{
		DisplayErrorMessage("Could not set Windows cooperative level for DirectDraw keyboard interface.\nAnother application may be using it at this time.\nPlease close all other applications and try again.",
							"Error - CDirectInput::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(!AcquireKeyboard())
		return(FALSE);
	SetKeyStructure(NULL);
	KeyStateMem=GlobalAlloc(GHND,
							256);
	if(!KeyStateMem)
	{
		DisplayErrorMessage("Could not allocate memory for keyboard interface data.",
							"Error - CDirectInput::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	KeyState=(LPSTR)GlobalLock(KeyStateMem);
	if(!KeyState)
	{
		DisplayErrorMessage("Could not lock memory for keyboard interface data.",
							"Error - CDirectInput::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL inline _fastcall CDirectInput::AcquireKeyboard(void)
{
	if(FAILED(lpKeyboard->Acquire()))
	{
		DisplayErrorMessage("Failed to acquire keyboard device.",
							"Error - CDirectInput::AcquireKeyboard()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL inline _fastcall CDirectInput::ReadKeyboard(void)
{
	HRESULT hResult;

	while(hResult=lpKeyboard->GetDeviceState(256,
								 			 (LPVOID)KeyState)==DIERR_INPUTLOST)
	{
		if(!AcquireKeyboard())
			return(FALSE);
	}
	if(FAILED(hResult))
	{
		DisplayErrorMessage("Failed to read keyboard data.",
							"Error - CDirectInput::ReadKeyboard()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

#define COMPARE_INPUT_DATA(v,n,o,r) if(n->v&!o.v) r->v=TRUE; else r->v=FALSE; o.v=n->v;

BOOL _fastcall CDirectInput::GetUserInput(CONTROL_DATA_PTR NewData,CONTROL_DATA_PTR ReturnData)
{
	static BOOL FirstTime=TRUE;

	if(FirstTime)
	{
		OldData.up=FALSE;
		OldData.down=FALSE;
		OldData.left=FALSE;
		OldData.right=FALSE;
		OldData.btn1=FALSE;
		OldData.btn2=FALSE;
		OldData.btn3=FALSE;
		OldData.btn4=FALSE;
		OldData.esc=FALSE;
		FirstTime=FALSE;
	}
	NewData->up=FALSE;
	NewData->down=FALSE;
	NewData->left=FALSE;
	NewData->right=FALSE;
	NewData->btn1=FALSE;
	NewData->btn2=FALSE;
	NewData->btn3=FALSE;
	NewData->btn4=FALSE;
	NewData->esc=FALSE;
	if(!ReadKeyboard())
		return(FALSE);
	if(KeyState[ControlKeys.up]&0x80)
		NewData->up=TRUE;
	if(KeyState[ControlKeys.down]&0x80)
		NewData->down=TRUE;
	if(KeyState[ControlKeys.left]&0x80)
		NewData->left=TRUE;
	if(KeyState[ControlKeys.right]&0x80)
		NewData->right=TRUE;
	if(KeyState[ControlKeys.btn1]&0x80)
		NewData->btn1=TRUE;
	if(KeyState[ControlKeys.btn2]&0x80)
		NewData->btn2=TRUE;
	if(KeyState[ControlKeys.btn3]&0x80)
		NewData->btn3=TRUE;
	if(KeyState[ControlKeys.btn4]&0x80)
		NewData->btn4=TRUE;
	if(KeyState[ControlKeys.esc]&0x80)
		NewData->esc=TRUE;
	if(ReadJoystick())
	{
		if(JoystickState.lX<-2)
			NewData->left=TRUE;
		if(JoystickState.lX>2)
			NewData->right=TRUE;
		if(JoystickState.lY<-2)
			NewData->up=TRUE;
		if(JoystickState.lY>2)
			NewData->down=TRUE;
		if(JoystickState.rgbButtons[JoystickButtons.btn1]&0x80)
			NewData->btn1=TRUE;
		if(JoystickState.rgbButtons[JoystickButtons.btn2]&0x80)
			NewData->btn2=TRUE;
		if(JoystickState.rgbButtons[JoystickButtons.btn3]&0x80)
			NewData->btn3=TRUE;
		if(JoystickState.rgbButtons[JoystickButtons.btn4]&0x80)
			NewData->btn4=TRUE;
	}
	if(NewData->left&&NewData->right)
	{
		NewData->left=FALSE;
		NewData->right=FALSE;
	}
	if(NewData->up&&NewData->down)
	{
		NewData->up=FALSE;
		NewData->down=FALSE;
	}
	COMPARE_INPUT_DATA(up,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(down,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(left,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(right,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(btn1,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(btn2,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(btn3,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(btn4,NewData,OldData,ReturnData);
	COMPARE_INPUT_DATA(esc,NewData,OldData,ReturnData);
	return(TRUE);
}

LPDIRECTINPUT inline _fastcall CDirectInput::GetDirectInputInterfacePointer()
{
	return lpDirectInput;
}

BOOL _fastcall CDirectInput::SetKeyStructure(CONTROL_DATA_PTR controldataptr)
{
	if(controldataptr)
	{
		ControlKeys.up=controldataptr->up;
		ControlKeys.down=controldataptr->down;
		ControlKeys.left=controldataptr->left;
		ControlKeys.right=controldataptr->right;
		ControlKeys.btn1=controldataptr->btn1;
		ControlKeys.btn2=controldataptr->btn2;
		ControlKeys.btn3=controldataptr->btn3;
		ControlKeys.btn4=controldataptr->btn4;
		ControlKeys.esc=controldataptr->esc;
	}
	else
	{
		ControlKeys.up=DIK_UPARROW;
		ControlKeys.down=DIK_DOWNARROW;
		ControlKeys.left=DIK_LEFTARROW;
		ControlKeys.right=DIK_RIGHTARROW;
		ControlKeys.btn1=DIK_SPACE;
		ControlKeys.btn2=DIK_RALT;
		ControlKeys.btn3=DIK_RCONTROL;
		ControlKeys.btn4=DIK_RSHIFT;
		ControlKeys.esc=DIK_ESCAPE;
	}
	return(TRUE);
}

BOOL _fastcall CDirectInput::GetKeyStructure(CONTROL_DATA_PTR controldataptr)
{
	controldataptr->up=ControlKeys.up;
	controldataptr->down=ControlKeys.down;
	controldataptr->left=ControlKeys.left;
	controldataptr->right=ControlKeys.right;
	controldataptr->btn1=ControlKeys.btn1;
	controldataptr->btn2=ControlKeys.btn2;
	controldataptr->btn3=ControlKeys.btn3;
	controldataptr->btn4=ControlKeys.btn4;
	controldataptr->esc=ControlKeys.esc;
	return(TRUE);
}

BOOL _fastcall CDirectInput::ScanKeyboard(LPSTR *idstring,LPDWORD keyval)
{
	*idstring=" ";
	*keyval=0;	
	if(!ReadKeyboard())
		return(FALSE);
	SCAN_KEY(DIK_ESCAPE,"Escape");
	SCAN_KEY(DIK_1,"1");
	SCAN_KEY(DIK_2,"2");
	SCAN_KEY(DIK_3,"3");
	SCAN_KEY(DIK_4,"4");
	SCAN_KEY(DIK_5,"5");
	SCAN_KEY(DIK_6,"6");
	SCAN_KEY(DIK_7,"7");
	SCAN_KEY(DIK_8,"8");
	SCAN_KEY(DIK_9,"9");
	SCAN_KEY(DIK_0,"0");
	SCAN_KEY(DIK_MINUS,"Minus");
	SCAN_KEY(DIK_EQUALS,"Equals");
	SCAN_KEY(DIK_BACKSPACE,"Backspace");
	SCAN_KEY(DIK_TAB,"Tab");
	SCAN_KEY(DIK_Q,"Q");
	SCAN_KEY(DIK_W,"W");
	SCAN_KEY(DIK_E,"E");
	SCAN_KEY(DIK_R,"R");
	SCAN_KEY(DIK_T,"T");
	SCAN_KEY(DIK_Y,"Y");
	SCAN_KEY(DIK_U,"U");
	SCAN_KEY(DIK_I,"I");
	SCAN_KEY(DIK_O,"O");
	SCAN_KEY(DIK_P,"P");
	SCAN_KEY(DIK_LBRACKET,"Left Bracket");
	SCAN_KEY(DIK_RBRACKET,"Right Bracket");
	SCAN_KEY(DIK_RETURN,"Enter");
	SCAN_KEY(DIK_LCONTROL,"Left Control");
	SCAN_KEY(DIK_A,"A");
	SCAN_KEY(DIK_S,"S");
	SCAN_KEY(DIK_D,"D");
	SCAN_KEY(DIK_F,"F");
	SCAN_KEY(DIK_G,"G");
	SCAN_KEY(DIK_H,"H");
	SCAN_KEY(DIK_J,"J");
	SCAN_KEY(DIK_K,"K");
	SCAN_KEY(DIK_L,"L");
	SCAN_KEY(DIK_SEMICOLON,"Semicolon");
	SCAN_KEY(DIK_APOSTROPHE,"Apostrophe");
	SCAN_KEY(DIK_GRAVE,"^Grave^ Accent");
	SCAN_KEY(DIK_LSHIFT,"Left Shift");
	SCAN_KEY(DIK_BACKSLASH,"Backslash");
	SCAN_KEY(DIK_Z,"Z");
	SCAN_KEY(DIK_X,"X");
	SCAN_KEY(DIK_C,"C");
	SCAN_KEY(DIK_V,"V");
	SCAN_KEY(DIK_B,"B");
	SCAN_KEY(DIK_N,"N");
	SCAN_KEY(DIK_M,"M");
	SCAN_KEY(DIK_COMMA,"Comma");
	SCAN_KEY(DIK_PERIOD,"Period");
	SCAN_KEY(DIK_SLASH,"Slash");
	SCAN_KEY(DIK_RSHIFT,"Right Shift");
	SCAN_KEY(DIK_NUMPADSTAR,"Asterisk (Keypad)");
	SCAN_KEY(DIK_LALT,"Left Alt");
	SCAN_KEY(DIK_SPACE,"Spacebar");
	SCAN_KEY(DIK_CAPSLOCK,"Caps Lock");
	SCAN_KEY(DIK_F1,"F1");
	SCAN_KEY(DIK_F2,"F2");
	SCAN_KEY(DIK_F3,"F3");
	SCAN_KEY(DIK_F4,"F4");
	SCAN_KEY(DIK_F5,"F5");
	SCAN_KEY(DIK_F6,"F6");
	SCAN_KEY(DIK_F7,"F7");
	SCAN_KEY(DIK_F8,"F8");
	SCAN_KEY(DIK_F9,"F9");
	SCAN_KEY(DIK_F10,"F10");
	SCAN_KEY(DIK_NUMLOCK,"Num Lock");
	SCAN_KEY(DIK_SCROLL,"Scroll Lock");
	SCAN_KEY(DIK_NUMPAD7,"7 (Keypad)");
	SCAN_KEY(DIK_NUMPAD8,"8 (Keypad)");
	SCAN_KEY(DIK_NUMPAD9,"9 (Keypad)");
	SCAN_KEY(DIK_NUMPADMINUS,"Minus (Keypad)");
	SCAN_KEY(DIK_NUMPAD4,"4 (Keypad)");
	SCAN_KEY(DIK_NUMPAD5,"5 (Keypad)");
	SCAN_KEY(DIK_NUMPAD6,"6 (Keypad)");
	SCAN_KEY(DIK_NUMPADPLUS,"Plus (Keypad)");
	SCAN_KEY(DIK_NUMPAD1,"1 (Keypad)");
	SCAN_KEY(DIK_NUMPAD2,"2 (Keypad)");
	SCAN_KEY(DIK_NUMPAD3,"3 (Keypad)");
	SCAN_KEY(DIK_NUMPAD0,"0 (Keypad)");
	SCAN_KEY(DIK_NUMPADPERIOD,"Period (Keypad)");
	SCAN_KEY(DIK_F11,"F11");
	SCAN_KEY(DIK_F12,"F12");
	SCAN_KEY(DIK_F13,"F13");
	SCAN_KEY(DIK_F14,"F14");
	SCAN_KEY(DIK_F15,"F15");
	SCAN_KEY(DIK_KANA,"Kana");
	SCAN_KEY(DIK_CONVERT,"Convert");
	SCAN_KEY(DIK_NOCONVERT,"No Convert");
	SCAN_KEY(DIK_YEN,"Yen");
	SCAN_KEY(DIK_NUMPADEQUALS,"Equals (Keypad)");
	SCAN_KEY(DIK_CIRCUMFLEX,"Circumflex");
	SCAN_KEY(DIK_AT,"At");
	SCAN_KEY(DIK_COLON,"Colon");
	SCAN_KEY(DIK_UNDERLINE,"Underline");
	SCAN_KEY(DIK_KANJI,"Kanji");
	SCAN_KEY(DIK_STOP,"Stop");
	SCAN_KEY(DIK_AX,"Ax");
	SCAN_KEY(DIK_UNLABELED,"Unlabeled");
	SCAN_KEY(DIK_NUMPADENTER,"Enter (Keypad)");
	SCAN_KEY(DIK_RCONTROL,"Right Control");
	SCAN_KEY(DIK_NUMPADCOMMA,"Comma (Keypad)");
	SCAN_KEY(DIK_NUMPADSLASH,"Slash (Keypad)");
	SCAN_KEY(DIK_SYSRQ,"Print Screen");
	SCAN_KEY(DIK_RALT,"Right Alt");
	SCAN_KEY(DIK_HOME,"Home");
	SCAN_KEY(DIK_UPARROW,"Up Arrow");
	SCAN_KEY(DIK_PGUP,"Page Up");
	SCAN_KEY(DIK_LEFTARROW,"Left Arrow");
	SCAN_KEY(DIK_RIGHTARROW,"Right Arrow");
	SCAN_KEY(DIK_END,"End");
	SCAN_KEY(DIK_DOWN,"Down Arrow");
	SCAN_KEY(DIK_PGDN,"Page Down");
	SCAN_KEY(DIK_INSERT,"Insert");
	SCAN_KEY(DIK_DELETE,"Delete");
	SCAN_KEY(DIK_LWIN,"Left Windows");
	SCAN_KEY(DIK_RWIN,"Right Windows");
	SCAN_KEY(DIK_APPS,"Application");
	return(FALSE);
}

BOOL CDirectInput::DetectJoysticks(void)
{
	GUID GUIDtemp;
	
	if(FAILED(lpDirectInput->EnumDevices(DIDEVTYPE_JOYSTICK,
										 EnumJoysticks,
										 &GUIDtemp,
										 DIEDFL_ATTACHEDONLY)))
	{
		DisplayErrorMessage("Failed to create joystick data list.",
							"Error - CDirectInput::ScanJoysticks()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectInput::SelectJoystick(DWORD joynum)
{
	LPDIRECTINPUTDEVICE lpdiTemp=NULL;
	GUID guidTemp;
	
	if(joynum>JoystickCount)
	{
		DisplayErrorMessage("Invalid joystick selected.",
							"Error - CDirectInput::SelectJoystick()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		JoystickActive=FALSE;
		EnumJoysticks(NULL,NULL);
		return(FALSE);
	}
	CopyMemory(&guidTemp,
			   JoystickList+((sizeof(GUID)+41)*joynum),
			   sizeof(GUID));
	if(FAILED(lpDirectInput->CreateDevice(guidTemp,
										  &lpdiTemp,
										  NULL)))
	{
		DisplayErrorMessage("Failed to create DirectInput device for the selected joystick.",
							"Error - CDirectInput::SelectJoystick()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		JoystickActive=FALSE;
	}
	else
	{
		if(FAILED(lpdiTemp->QueryInterface(IID_IDirectInputDevice2,
										   (LPVOID*)&lpJoystick)))
		{
			DisplayErrorMessage("Failed to receive DirectInput interface for the selected joystick.",
								"Error - CDirectInput::SelectJoystick()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			JoystickActive=FALSE;
		}
		else
		{
			if(FAILED(lpJoystick->SetCooperativeLevel(APP_HWND,
													  DISCL_BACKGROUND|
													  DISCL_NONEXCLUSIVE)))
			{
				DisplayErrorMessage("Failed to set cooperative level for the selected joystick.",
									"Error - CDirectInput::SelectJoystick()",
									MB_OK|MB_ICONINFORMATION,
									lpDirectDraw);
				JoystickActive=FALSE;
			}
			else
			{
				if(FAILED(lpJoystick->SetDataFormat(&c_dfDIJoystick)))
				{
					DisplayErrorMessage("Failed to set data format for the selected joystick.",
										"Error - CDirectInput::SelectJoystick()",
										MB_OK|MB_ICONINFORMATION,
										lpDirectDraw);
					JoystickActive=FALSE;
				}
				else
				{
					JoystickRange.lMin=-5;
					JoystickRange.lMax=5;
					JoystickRange.diph.dwSize=sizeof(DIPROPRANGE);
					JoystickRange.diph.dwHeaderSize=sizeof(DIPROPHEADER);
					JoystickRange.diph.dwObj=DIJOFS_X;
					JoystickRange.diph.dwHow=DIPH_BYOFFSET;
					if(FAILED(lpJoystick->SetProperty(DIPROP_RANGE,
													  &JoystickRange.diph)))
					{
						DisplayErrorMessage("Failed to set x-axis range for the selected joystick.",
											"Error - CDirectInput::SelectJoystick()",
											MB_OK|MB_ICONINFORMATION,
											lpDirectDraw);
						JoystickActive=FALSE;
					}
					else
					{
						JoystickRange.lMin=-5;
						JoystickRange.lMax=5;
						JoystickRange.diph.dwSize=sizeof(DIPROPRANGE);
						JoystickRange.diph.dwHeaderSize=sizeof(DIPROPHEADER);
						JoystickRange.diph.dwObj=DIJOFS_Y;
						JoystickRange.diph.dwHow=DIPH_BYOFFSET;
						if(FAILED(lpJoystick->SetProperty(DIPROP_RANGE,
														  &JoystickRange.diph)))
						{
							DisplayErrorMessage("Failed to set y-axis range for the selected joystick.",
												"Error - CDirectInput::SelectJoystick()",
												MB_OK|MB_ICONINFORMATION,
												lpDirectDraw);
							JoystickActive=FALSE;
						}
						else
						{
							JoystickDeadZone.diph.dwSize=sizeof(DIPROPDWORD);
							JoystickDeadZone.diph.dwHeaderSize=sizeof(JoystickDeadZone.diph);
							JoystickDeadZone.diph.dwObj=DIJOFS_X;
							JoystickDeadZone.diph.dwHow=DIPH_BYOFFSET;
							JoystickDeadZone.dwData=1000;
							if(FAILED(lpJoystick->SetProperty(DIPROP_DEADZONE,
															  &JoystickDeadZone.diph)))
							{
								DisplayErrorMessage("Failed to set x-axis dead zone for the selected joystick.",
													"Error - CDirectInput::SelectJoystick()",
													MB_OK|MB_ICONINFORMATION,
													lpDirectDraw);
								JoystickActive=FALSE;
							}
							else
							{
								JoystickDeadZone.diph.dwSize=sizeof(DIPROPDWORD);
								JoystickDeadZone.diph.dwHeaderSize=sizeof(JoystickDeadZone.diph);
								JoystickDeadZone.diph.dwObj=DIJOFS_X;
								JoystickDeadZone.diph.dwHow=DIPH_BYOFFSET;
								JoystickDeadZone.dwData=1000;
								if(FAILED(lpJoystick->SetProperty(DIPROP_DEADZONE,
																  &JoystickDeadZone.diph)))
								{
									DisplayErrorMessage("Failed to set y-axis dead zone for the selected joystick.",
														"Error - CDirectInput::SelectJoystick()",
														MB_OK|MB_ICONINFORMATION,
														lpDirectDraw);
									JoystickActive=FALSE;
								}
								else
								{
									if(FAILED(lpJoystick->Acquire()))
									{
										DisplayErrorMessage("Failed to acquire the selected joystick.",
															"Error - CDirectInput::SelectJoystick()",
															MB_OK|MB_ICONINFORMATION,
															lpDirectDraw);
										JoystickActive=FALSE;
									}
								}
							}
						}
					}
				}
			}
		}
		lpdiTemp->Release();
		lpdiTemp=NULL;
	}
	JoystickActive=TRUE;
	EnumJoysticks(NULL,NULL);
	SetJoystickButtons(NULL);
	return(TRUE);
}

LPSTR CDirectInput::ExtractJoystickName(DWORD joynum)
{
	if(joynum>JoystickCount)
	{
		DisplayErrorMessage("Invalid joystick selected.",
							"Error - CDirectInput::SelectJoystick()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(NULL);
	}
	else
		return(JoystickList+((sizeof(GUID)+41)*joynum)+sizeof(GUID));
}

BOOL _fastcall CDirectInput::SetJoystickButtons(CONTROL_DATA_PTR controldataptr)
{
	if(controldataptr)
	{
		JoystickButtons.btn1=controldataptr->btn1;
		JoystickButtons.btn2=controldataptr->btn2;
		JoystickButtons.btn3=controldataptr->btn3;
		JoystickButtons.btn4=controldataptr->btn4;
	}
	else
	{
		JoystickButtons.btn1=0;
		JoystickButtons.btn2=1;
		JoystickButtons.btn3=2;
		JoystickButtons.btn4=3;
	}
	return(TRUE);
}

BOOL _fastcall CDirectInput::GetJoystickButtons(CONTROL_DATA_PTR controldataptr)
{
	if(controldataptr)
	{
		controldataptr->btn1=JoystickButtons.btn1;
		controldataptr->btn2=JoystickButtons.btn2;
		controldataptr->btn3=JoystickButtons.btn3;
		controldataptr->btn4=JoystickButtons.btn4;
	}
	else
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirectInput::ReadJoystick(void)
{
	if(!JoystickActive)
		return(FALSE);
	if(FAILED(lpJoystick->Poll()))
	{
		DisplayErrorMessage("Failed to poll joystick port.\nJoystick will be de-activated.",
							"Error - CDirectInput::ReadJoystick()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		JoystickActive=FALSE;
		return(FALSE);
	}
	if(FAILED(lpJoystick->GetDeviceState(sizeof(DIJOYSTATE),
										 (LPVOID)&JoystickState)))
	{
		DisplayErrorMessage("Failed to read joystick data.\nJoystick will be de-activated.",
							"Error - CDirectInput::ReadJoystick()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		JoystickActive=FALSE;
		return(FALSE);
	}
	return(TRUE);
}

BOOL inline _fastcall CDirectInput::IsKeyUp(BYTE keycode)
{
	if(KeyState[keycode])
		return(FALSE);
	else
		return(TRUE);
}

BOOL inline _fastcall CDirectInput::IsKeyDown(BYTE keycode)
{
	if(KeyState[keycode])
		return(TRUE);
	else
		return(FALSE);
}

BOOL CDirectInput::Release(void)
{
	if(lpJoystick)
	{
		lpJoystick->Unacquire();
		lpJoystick->Release();
		lpJoystick=NULL;
	}
	if(KeyState)
	{
		GlobalUnlock(KeyStateMem);
		KeyState=NULL;
	}
	if(KeyStateMem)
	{
		if(GlobalFree(KeyStateMem))
			DisplayErrorMessage("Failed to free keyboard state data.\nSystem may become unstable.",
								"Error - CDirectInput::~CDirectInput()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
		else
			KeyStateMem=NULL;
	}
	if(lpKeyboard)
	{
		lpKeyboard->Unacquire();
		lpKeyboard->Release();
		lpKeyboard=NULL;
	}
	if(lpDirectInput)
	{
		lpDirectInput->Release();
		lpDirectInput=NULL;
	}
	return(TRUE);
}

// DIRECTSOUND ENCODING FUNCTIONS //

#ifdef INIT_SOUND
#ifdef ENCODE_DATA
#include"mmsys2.h"
BOOL EncodeSound(char *filename,CDirectDraw* dd,CDirectSound* ds)
{
	DWORD BytesWritten;
	BYTE output;
	output=DTF_IDSOUND;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - EncodeSound()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	WritePos=SetFilePointer(OutputFile,0,NULL,FILE_CURRENT);
	if(!WriteFile(RecordFile,
				  (char*)&WritePos,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 2",
				   "Error - EncodeSound()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	output=DTF_IDSOUND;
	if(!WriteFile(OutputFile,
				  (char*)&output,
				  sizeof(char),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error A",
				   "Error - EncodeSound()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	DWORD size;
	HMMIO hWav;
	MMCKINFO parent,child;
	WAVEFORMATEX wfx;
	int soundid=-1;
	UCHAR *sndbuffer,*audioptr1=NULL,*audioptr2=NULL;
	DWORD audiolength1=0,audiolength2=0;
	parent.ckid=mmioFOURCC(0,0,0,0);
	parent.cksize=0;
	parent.fccType=mmioFOURCC(0,0,0,0);
	parent.dwDataOffset=0;
	parent.dwFlags=0;
	child=parent;
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	if((hWav=mmioOpen(GlobalStr,NULL,MMIO_READ|MMIO_ALLOCBUF))==NULL)
	{
		DisplayErrorMessage("Encode error 3",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	parent.fccType=mmioFOURCC('W','A','V','E');
	if(mmioDescend(hWav,&parent,NULL,MMIO_FINDRIFF))
	{
		mmioClose(hWav,0);
		DisplayErrorMessage("Encode error 4",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	child.ckid=mmioFOURCC('f','m','t',' ');
	if(mmioDescend(hWav,&child,&parent,0))
	{
		mmioClose(hWav,0);
		DisplayErrorMessage("Encode error 5",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	if(mmioRead(hWav,(char*)&wfx,sizeof(wfx))!=sizeof(wfx))
	{
		mmioClose(hWav,0);
		DisplayErrorMessage("Encode error 6",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	if(wfx.wFormatTag!=WAVE_FORMAT_PCM)
	{
		mmioClose(hWav,0);
		DisplayErrorMessage("Encode error 7",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	if(mmioAscend(hWav,&child,0))
	{
		mmioClose(hWav,0);
		DisplayErrorMessage("Encode error 8",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	child.ckid=mmioFOURCC('d','a','t','a');
	if(mmioDescend(hWav,&child,&parent,MMIO_FINDCHUNK))
	{
		mmioClose(hWav,0);
		DisplayErrorMessage("Encode error 9",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	sndbuffer=(UCHAR*)malloc(child.cksize);
	if(!sndbuffer)
	{
		mmioClose(hWav,0);
		free(sndbuffer);
		DisplayErrorMessage("Encode error 10",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	if(mmioRead(hWav,(char*)sndbuffer,child.cksize)==-1)
	{
		mmioClose(hWav,0);
		free(sndbuffer);
		DisplayErrorMessage("Encode error 11",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		return(FALSE);
	}
	if(mmioClose(hWav,0))
	{
		DisplayErrorMessage("Encode error 12",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		free(sndbuffer);
		return(FALSE);
	}
	size=child.cksize;
	if(!WriteFile(OutputFile,
				 (char*)&size,
				 sizeof(DWORD),
				 &BytesWritten,
				 NULL))
	{
		DisplayErrorMessage("Encode error 12b",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		free(sndbuffer);
		return(FALSE);
	}
	if(!WriteFile(OutputFile,
				 (char*)sndbuffer,
				 size,
				 &BytesWritten,
				 NULL))
	{
		DisplayErrorMessage("Encode error 12A",
							"Error - EncodeSound()",
							MB_OK,
							NULL);
		free(sndbuffer);
		return(FALSE);
	}
//	LPDIRECTSOUNDBUFFER sbtemp=NULL;
//	WAVEFORMATEX pcmwf;
//	DSBUFFERDESC desc;
//	SOUND_DATA snddata;
//	if(ds->IsSoundInitialized())
//	{
//		memset(&pcmwf,0,sizeof(WAVEFORMATEX));
//		pcmwf.wFormatTag=WAVE_FORMAT_PCM;
//		pcmwf.nChannels=1;
//		pcmwf.nSamplesPerSec=11025;
//		pcmwf.nBlockAlign=1;
//		pcmwf.nAvgBytesPerSec=pcmwf.nSamplesPerSec*pcmwf.nBlockAlign;
//		pcmwf.wBitsPerSample=8;
//		pcmwf.cbSize=0;
//		desc.dwSize=sizeof(desc);
//		desc.dwFlags=DSBCAPS_CTRLPAN|
//					 DSBCAPS_STATIC|
//					 DSBCAPS_LOCSOFTWARE;
//		desc.dwBufferBytes=size;
//		desc.lpwfxFormat=&pcmwf;
//		desc.dwReserved=NULL;
//		if(FAILED(ds->GetDirectSoundInterfacePointer()->CreateSoundBuffer(&desc,
//																		  &sbtemp,
//																		  NULL)))
//		{
//			DisplayErrorMessage("Encode error 13",
//								"Error - EncodeSound()",
//								MB_OK,
//								dd);
//			free(sndbuffer);
//			return(TRUE);
//		}
//		snddata.sound=sbtemp;
//		if(FAILED(sbtemp->Lock(0,size,(void**)&audioptr1,&audiolength1,(void**)&audioptr2,&audiolength2,DSBLOCK_FROMWRITECURSOR|DSBLOCK_ENTIREBUFFER)))
//		{
//			DisplayErrorMessage("Encode error 14",
//								"Error - EncodeSound()",
//								MB_OK,
//								dd);
//			free(sndbuffer);
//			return(TRUE);
//		}
//		if(audiolength1)
//			memcpy(audioptr1,sndbuffer,audiolength1);
//		if(audiolength2)
//			memcpy(audioptr2,sndbuffer+audiolength1,audiolength2);
//		if(FAILED(sbtemp->Unlock(audioptr1,audiolength1,audioptr2,audiolength2)))
//		{
//			DisplayErrorMessage("Encode error 15",
//								"Error - EncodeSound()",
//								MB_OK,
//								dd);
//			free(sndbuffer);
//			return(TRUE);
//		}
//		snddata.SoundLoaded=TRUE;
//		ds->Play(&snddata,128);
//	}
	free(sndbuffer);
	DisplayErrorMessage("Sound successfully encoded.",
			   "Information",
			   MB_OK|MB_ICONINFORMATION,
							dd);
//	if(ds->IsSoundInitialized())
//	{
//		ds->Stop(&snddata);
//		ds->ReleaseSound(&snddata);
//	}
	return(TRUE);
}
#endif

// DIRECTSOUND FUNCTIONS //

BOOL CDirectSound::Initialize(CDirectDraw* lpdd)
{
	lpDirectDraw=lpdd;
	lpDirectSound=NULL;
	SoundInit=FALSE;
	SoundActive=FALSE;
	if(FAILED(DirectSoundCreate(NULL,
								&lpDirectSound,
								NULL)))
	{
		DisplayErrorMessage("Failed to create DirectSound object.\nSound will not function.",
							"Error - CDirectSound::Initialize()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		SoundInit=FALSE;
		SoundActive=FALSE;
		return(FALSE);
	}
	if(FAILED(lpDirectSound->SetCooperativeLevel(APP_HWND,
												 DSSCL_NORMAL)))
	{
		DisplayErrorMessage("Failed to set DirectSound cooperation level.\nSound will not function.",
							"Error - CDirectSound::Initialize()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		SoundInit=FALSE;
		SoundActive=FALSE;
		return(FALSE);
	}
	SoundInit=TRUE;
	SoundActive=TRUE;
	return(TRUE);
}

BOOL CDirectSound::LoadSoundFromDataFile(SOUND_DATA_PTR sounddataptr)
{
	DWORD SoundSize=0;
	HGLOBAL SoundDataMem=NULL;
	LPSTR SoundData=NULL;
	BYTE DataType=NULL;
	DWORD BytesRead=0;
	LPSTR audioptr1=NULL;
	LPSTR audioptr2=NULL;
	DWORD audiolength1=0;
	DWORD audiolength2=0;
	
	sounddataptr->sound=NULL;
	sounddataptr->SoundLoaded=FALSE;
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read sound ID from disk.",
							"Error - CDirectSound::LoadSoundFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	if(DataType!=DTF_IDSOUND)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadSoundFromDataFile()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&SoundSize,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read sound length from disk.",
							"Error - CDirectSound::LoadSoundFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	SoundDataMem=GlobalAlloc(GHND,
							 SoundSize);
	if(!SoundDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for sound effect.",
							"Error - CDirectSound::LoadSoundFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	SoundData=(LPSTR)GlobalLock(SoundDataMem);
	if(!SoundData)
	{
		DisplayErrorMessage("Failed to lock memory for sound effect.",
							"Error - CDirectSound::LoadSoundFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		GlobalFree(SoundDataMem);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)SoundData,
				 SoundSize,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read sound data from disk.",
							"Error - CDirectSound::LoadSoundFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		GlobalUnlock(SoundDataMem);
		GlobalFree(SoundDataMem);
		return(FALSE);
	}
	if(SoundInit)
	{
		wfx.wFormatTag=WAVE_FORMAT_PCM;
		wfx.nChannels=1;
		wfx.nSamplesPerSec=11025;
		wfx.nBlockAlign=1;
		wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
		wfx.wBitsPerSample=8;
		wfx.cbSize=0;
		dsbufferdesc.dwSize=sizeof(DSBUFFERDESC);
		dsbufferdesc.dwFlags=DSBCAPS_CTRLPAN|
							 DSBCAPS_STATIC|
							 DSBCAPS_LOCHARDWARE;
		dsbufferdesc.dwBufferBytes=SoundSize;
		dsbufferdesc.lpwfxFormat=&wfx;
		dsbufferdesc.dwReserved=NULL;
		if(FAILED(lpDirectSound->CreateSoundBuffer(&dsbufferdesc,
												   &sounddataptr->sound,
												   NULL)))
		{
			wfx.wFormatTag=WAVE_FORMAT_PCM;
			wfx.nChannels=1;
			wfx.nSamplesPerSec=11025;
			wfx.nBlockAlign=1;
			wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
			wfx.wBitsPerSample=8;
			wfx.cbSize=0;
			dsbufferdesc.dwSize=sizeof(DSBUFFERDESC);
			dsbufferdesc.dwFlags=DSBCAPS_CTRLPAN|
								 DSBCAPS_STATIC|
								 DSBCAPS_LOCSOFTWARE;
			dsbufferdesc.dwBufferBytes=SoundSize;
			dsbufferdesc.lpwfxFormat=&wfx;
			dsbufferdesc.dwReserved=NULL;
			if(FAILED(lpDirectSound->CreateSoundBuffer(&dsbufferdesc,
													   &sounddataptr->sound,
													   NULL)))
			{
				DisplayErrorMessage("Failed to create sound effect buffer.",
									"Error - CDirectSound::LoadSoundFromDataFile()",
									MB_OK|MB_ICONINFORMATION,
									lpDirectDraw);
				GlobalUnlock(SoundDataMem);
				GlobalFree(SoundDataMem);
				return(FALSE);
			}
		}
		if(FAILED(sounddataptr->sound->Lock(0,
											SoundSize,
											(LPVOID*)&audioptr1,
											&audiolength1,
											(LPVOID*)&audioptr2,
											&audiolength2,
											DSBLOCK_FROMWRITECURSOR)))
		{
			DisplayErrorMessage("Failed to lock surface for sound effect data.",
								"Error - CDirectSound::LoadSoundFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			GlobalUnlock(SoundDataMem);
			GlobalFree(SoundDataMem);
			return(FALSE);
		}
		CopyMemory(audioptr1,
				   SoundData,
				   audiolength1);
		CopyMemory(audioptr2,
				   SoundData+audiolength1,
				   audiolength2);
		if(FAILED(sounddataptr->sound->Unlock(audioptr1,
											  audiolength1,
											  audioptr2,
											  audiolength2)))
		{
			DisplayErrorMessage("Failed to unlock surface for sound effect data.",
								"Error - CDirectSound::LoadSoundFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			GlobalUnlock(SoundDataMem);
			GlobalFree(SoundDataMem);
			return(FALSE);
		}
		sounddataptr->SoundLoaded=TRUE;
	}
	GlobalUnlock(SoundDataMem);
	if(GlobalFree(SoundDataMem))
	{
		DisplayErrorMessage("Failed to free memory for sound effect.\nSystem may become unstable.",
							"Error - CDirectSound::LoadSoundFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirectSound::Play(SOUND_DATA_PTR sounddataptr,BYTE pan)
{
	long lPan;
	
	if(SoundActive)
		if(sounddataptr->SoundLoaded)
		{
			lPan=(long)pan;
			lPan-=128;
			lPan=(lPan<<6)+(lPan<<3)+(lPan<<2)+(lPan<<1);
			sounddataptr->sound->SetPan(lPan);
			sounddataptr->sound->Play(NULL,NULL,NULL);
		}
	return(TRUE);
}

BOOL _fastcall CDirectSound::Stop(SOUND_DATA_PTR sounddataptr)
{
	if(SoundActive)
		if(sounddataptr->SoundLoaded)
			sounddataptr->sound->Stop();
	return(TRUE);
}

BOOL CDirectSound::ReleaseSound(SOUND_DATA_PTR sounddataptr)
{
	if(SoundInit)
	{
		if(sounddataptr->sound)
		{
			sounddataptr->sound->Release();
			sounddataptr->sound=NULL;
		}
	}
	sounddataptr->SoundLoaded=FALSE;
	return(TRUE);
}

BOOL CDirectSound::ActivateSoundFX(void)
{
	if(SoundInit)
		SoundActive=TRUE;
	return(TRUE);
}

BOOL CDirectSound::DeactivateSoundFX(void)
{
	SoundActive=FALSE;
	return(TRUE);
}

BOOL inline _fastcall CDirectSound::IsSoundInitialized(void)
{
	if(SoundInit)
		return(TRUE);
	return(FALSE);
}

LPDIRECTSOUND inline _fastcall CDirectSound::GetDirectSoundInterfacePointer(void)
{
	return(lpDirectSound);
}

BOOL CDirectSound::Release(void)
{
	if(lpDirectSound)
	{
		lpDirectSound->Release();
		lpDirectSound=NULL;
		SoundActive=FALSE;
	}
	return(TRUE);
}

// DIRECTMUSIC ENCODING FUNCTIONS //

#ifdef ENCODE_DATA
BOOL EncodeMusic(char *filename,CDirectDraw* dd,CDirectMusic* dm)
{
	DWORD BytesWritten;
	BYTE output;
	output=DTF_IDMUSIC;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	WritePos=SetFilePointer(OutputFile,0,NULL,FILE_CURRENT);
	if(!WriteFile(RecordFile,
				  (char*)&WritePos,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 2",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	output=DTF_IDMUSIC;
	if(!WriteFile(OutputFile,
				  (char*)&output,
				  sizeof(char),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error A",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	HANDLE hFile;
	hFile=CreateFile(GlobalStr,
					 GENERIC_READ,
					 NULL,
					 NULL,
					 OPEN_EXISTING,
					 FILE_FLAG_SEQUENTIAL_SCAN,
					 NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Encode error 1",
							"Error - EncodeMusic()",
							MB_OK,
							dd);
		return(FALSE);
	}
	DWORD FileSize=GetFileSize(hFile,NULL);
	if(!WriteFile(OutputFile,
				  (char*)&FileSize,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error 2",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		CloseHandle(hFile);
		return(FALSE);
	}
	HGLOBAL MusicDataMem;
	LPSTR MusicData;
	MusicDataMem=GlobalAlloc(GHND,FileSize);
	if(!MusicDataMem)
	{
		DisplayErrorMessage("Encode error 3",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		CloseHandle(hFile);
		return(FALSE);
	}
	MusicData=(LPSTR)GlobalLock(MusicDataMem);
	if(!MusicData)
	{
		DisplayErrorMessage("Encode error 3a",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		GlobalFree(MusicDataMem);
		CloseHandle(hFile);
		return(FALSE);
	}
	if(!ReadFile(hFile,
				 MusicData,
				 FileSize,
				 &BytesWritten,
				 NULL))
	{
		DisplayErrorMessage("Encode error 4",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		GlobalUnlock(MusicDataMem);
		GlobalFree(MusicDataMem);
		CloseHandle(hFile);
		return(FALSE);
	}
	CloseHandle(hFile);
	if(!WriteFile(OutputFile,
				  MusicData,
				  FileSize,
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error 7",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		GlobalUnlock(MusicDataMem);
		GlobalFree(MusicDataMem);
		CloseHandle(hFile);
		return(FALSE);
	}
	GlobalUnlock(MusicDataMem);
	if(GlobalFree(MusicDataMem))
	{
		DisplayErrorMessage("Encode error 0xFF",
				   "Error - EncodeMusic()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
//	MUSIC_DATA musicdata;
//	if(dm->IsMusicInitialized())
//	{
//		DMUS_OBJECTDESC dmusobjdesc;
//		MULTI_TO_WIDE(WideGlobalStr,DefaultFilePath);
//		if(FAILED(dm->lpDirectMusicLoader->SetSearchDirectory(GUID_DirectMusicAllTypes,
//															  WideGlobalStr,
//															  FALSE)))
//		{
//			DisplayErrorMessage("Encode error 8",
//							    "Error - EncodeMusic()",
//								MB_OK|MB_ICONSTOP,
//								dd);
//			return(TRUE);
//		}
//		MULTI_TO_WIDE(WideGlobalStr,filename);
//		INIT_STRUCT(dmusobjdesc);
//		dmusobjdesc.guidClass=CLSID_DirectMusicSegment;
//		wcscpy(dmusobjdesc.wszFileName,WideGlobalStr);
//		dmusobjdesc.dwValidData=DMUS_OBJ_CLASS|DMUS_OBJ_FILENAME;
//		if(FAILED(dm->lpDirectMusicLoader->GetObject(&dmusobjdesc,
//													 IID_IDirectMusicSegment,
//													 (LPVOID*)&musicdata.segment)))
//		{
//			DisplayErrorMessage("Encode error 9",
//							    "Error - EncodeMusic()",
//								MB_OK|MB_ICONSTOP,
//								dd);
//			return(TRUE);
//		}
//		strcpy(GlobalStr,DefaultFilePath);
//		strcat(GlobalStr,MIDI_TEMP_FILE);
//		DeleteFile(GlobalStr);
//		if(FAILED(musicdata.segment->SetParam(GUID_StandardMIDIFile,
//											  -1,
//											  0,
//											  0,
//											  (LPVOID)dm->lpDirectMusic)))
//		{
//			DisplayErrorMessage("Encode error 10",
//							    "Error - EncodeMusic()",
//								MB_OK|MB_ICONSTOP,
//								dd);
//			return(TRUE);
//		}
//		if(FAILED(musicdata.segment->SetParam(GUID_Download,
//											  -1,
//											  0,
//											  0,
//											  (LPVOID)dm->lpDirectMusic)))
//		{
//			DisplayErrorMessage("Encode error 11",
//							    "Error - EncodeMusic()",
//								MB_OK|MB_ICONSTOP,
//								dd);
//			return(TRUE);
//		}
//		musicdata.SongLoaded=TRUE;
//		musicdata.state=NULL;
//		dm->lpDirectMusic->PlaySegment(musicdata.segment,
//									   0,
//									   0,
//									   &musicdata.state);
//	}
	DisplayErrorMessage("Music successfully encoded.",
						"Information",
						MB_OK|MB_ICONINFORMATION,
						NULL);
//	if(dm->IsMusicInitialized())
//	{
//		dm->lpDirectMusic->Stop(musicdata.segment,NULL,0,0);
//		musicdata.segment->SetParam(GUID_Unload,-1,0,0,(LPVOID)dm->lpDirectMusic);
//		musicdata.segment->Release();
//		musicdata.segment=NULL;
//	}
	return(TRUE);
}
#endif

// DIRECTMUSIC FUNCTIONS //

BOOL CDirectMusic::Initialize(CDirectDraw* lpdd,CDirectSound* lpds)
{
	lpDirectDraw=lpdd;
	lpDirectSound=lpds;
	MusicInit=FALSE;
	MusicActive=FALSE;
	lpDirectMusic=NULL;
	lpDirectMusicLoader=NULL;
	MIDICount=0;
	if(lpDirectSound->IsSoundInitialized())
	{
		if(FAILED(CoCreateInstance(CLSID_DirectMusicPerformance,
								   NULL,
								   CLSCTX_INPROC,
								   IID_IDirectMusicPerformance,
								   (LPVOID*)&lpDirectMusic)))
		{
			DisplayErrorMessage("Failed to create DirectMusic instance.\nMusic will not function",
								"Error - CDirectMusic::Initialize()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			return(FALSE);
		}
		if(FAILED(lpDirectMusic->Init(NULL,
									  lpDirectSound->GetDirectSoundInterfacePointer(),
									  APP_HWND)))
		{
			DisplayErrorMessage("Failed to initialize DirectMusic.\nMusic will not function",
								"Error - CDirectMusic::Initialize()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			return(FALSE);
		}
		if(FAILED(lpDirectMusic->AddPort(NULL)))
		{
			DisplayErrorMessage("Failed to add DirectMusic data port.\nMusic will not function",
								"Error - CDirectMusic::Initialize()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			return(FALSE);
		}
		if(FAILED(CoCreateInstance(CLSID_DirectMusicLoader,
								   NULL,
								   CLSCTX_INPROC,
								   IID_IDirectMusicLoader,
								   (LPVOID*)&lpDirectMusicLoader)))
		{
			DisplayErrorMessage("Failed to create DirectMusic file read interface.\nMusic will not function",
								"Error - CDirectMusic::Initialize()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			return(FALSE);
		}
		MusicActive=TRUE;
		MusicInit=TRUE;
	}
	return(TRUE);
}

BOOL CDirectMusic::LoadMusicFromDataFile(MUSIC_DATA_PTR musicdataptr)
{
	DWORD MusicSize=0;
	HGLOBAL MusicDataMem=NULL;
	LPSTR MusicData=NULL;
	BYTE DataType=NULL;
	DWORD BytesRead=0;
	DWORD BytesWritten=0;
	HANDLE hFile=NULL;
	
	musicdataptr->segment=NULL;
	musicdataptr->state=NULL;
	musicdataptr->SongLoaded=FALSE;
	musicdataptr->SongPlaying=FALSE;
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read music ID from disk.",
							"Error - CDirectMusic::LoadMusicFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	if(DataType!=DTF_IDMUSIC)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadMusicFromDataFile()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&MusicSize,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read music length from disk.",
							"Error - CDirectMusic::LoadMusicFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	MusicDataMem=GlobalAlloc(GHND,
							 MusicSize);
	if(!MusicDataMem)
	{
		DisplayErrorMessage("Failed to allocate memory for music.",
							"Error - CDirectMusic::LoadMusicFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	MusicData=(LPSTR)GlobalLock(MusicDataMem);
	if(!MusicData)
	{
		DisplayErrorMessage("Failed to lock memory for music.",
							"Error - CDirectMusic::LoadMusicFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		GlobalFree(MusicDataMem);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)MusicData,
				 MusicSize,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read music data from disk.",
							"Error - CDirectMusic::LoadMusicFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		GlobalUnlock(MusicDataMem);
		GlobalFree(MusicDataMem);
		return(FALSE);
	}
	if(MusicInit)
	{
		strcpy(GlobalStr,DefaultFilePath);
		strcat(GlobalStr,MIDI_TEMP_FILE);
		if(MIDICount<10)
			GlobalStr[strlen(DefaultFilePath)+7]=(char)('0'+MIDICount);
		else
			GlobalStr[strlen(DefaultFilePath)+7]=(char)('a'+(MIDICount-10));
		hFile=CreateFile(GlobalStr,
						 GENERIC_WRITE,
						 NULL,
						 NULL,
						 CREATE_ALWAYS,
						 FILE_FLAG_WRITE_THROUGH,
						 NULL);
		if(hFile==INVALID_HANDLE_VALUE)
		{
			DisplayErrorMessage("Failed to create temporary MIDI storage file.",
								"Error - CDirectMusic::LoadMusicFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			GlobalUnlock(MusicDataMem);
			GlobalFree(MusicDataMem);
			return(FALSE);
		}
		if(!WriteFile(hFile,
					  (LPVOID)MusicData,
					  MusicSize,
					  &BytesWritten,
					  NULL))
		{
			DisplayErrorMessage("Failed to write temporary MIDI data.",
								"Error - CDirectMusic::LoadMusicFromDataFile()",
								MB_OK|MB_ICONINFORMATION,
								lpDirectDraw);
			CloseHandle(hFile);
			GlobalUnlock(MusicDataMem);
			GlobalFree(MusicDataMem);
			return(FALSE);
		}
		CloseHandle(hFile);
		MULTI_TO_WIDE(WideGlobalStr,DefaultFilePath);
		if(FAILED(lpDirectMusicLoader->SetSearchDirectory(GUID_DirectMusicAllTypes,
														  WideGlobalStr,
														  FALSE)))
		{
			DisplayErrorMessage("Failed to search directory for temporary MIDI data.",
								"Error - CDirectMusic::LoadMusicFromDataFile()",
								MB_OK|MB_ICONSTOP,
								NULL);
			GlobalUnlock(MusicDataMem);
			GlobalFree(MusicDataMem);
			return(FALSE);
		}
		strcpy(GlobalStr,MIDI_TEMP_FILE);
		if(MIDICount<10)
			GlobalStr[7]=(char)('0'+MIDICount);
		else
			GlobalStr[7]=(char)('a'+(MIDICount-10));
		MULTI_TO_WIDE(WideGlobalStr,GlobalStr);
		INIT_STRUCT(dmusobjdesc);
		dmusobjdesc.guidClass=CLSID_DirectMusicSegment;
		wcscpy(dmusobjdesc.wszFileName,WideGlobalStr);
		dmusobjdesc.dwValidData=DMUS_OBJ_CLASS|DMUS_OBJ_FILENAME;
		if(FAILED(lpDirectMusicLoader->GetObject(&dmusobjdesc,
												 IID_IDirectMusicSegment,
												 (LPVOID*)&musicdataptr->segment)))
		{
			DisplayErrorMessage("Failed to load MIDI data.",
								"Error - CDirectMusic::LoadMusicFromDataFile()",
								MB_OK|MB_ICONSTOP,
								lpDirectDraw);
			GlobalUnlock(MusicDataMem);
			GlobalFree(MusicDataMem);
			return(FALSE);
		}
		if(FAILED(musicdataptr->segment->SetParam(GUID_StandardMIDIFile,
												  -1,
												  0,
												  0,
												  (LPVOID)lpDirectMusic)))
		{
			DisplayErrorMessage("Failed to set playback parameters for MIDI data.",
								"Error - CDirectMusic::LoadMusicFromDataFile()",
								MB_OK|MB_ICONSTOP,
								lpDirectDraw);
			GlobalUnlock(MusicDataMem);
			GlobalFree(MusicDataMem);
			return(FALSE);
		}
		if(FAILED(musicdataptr->segment->SetParam(GUID_Download,
												  -1,
												  0,
												  0,
												  (LPVOID)lpDirectMusic)))
		{
			DisplayErrorMessage("Failed to set instrument parameters for MIDI data.",
								"Error - CDirectMusic::LoadMusicFromDataFile()",
								MB_OK|MB_ICONSTOP,
								lpDirectDraw);
			GlobalUnlock(MusicDataMem);
			GlobalFree(MusicDataMem);
			return(FALSE);
		}
		musicdataptr->SongLoaded=TRUE;
		musicdataptr->state=NULL;
		strcpy(GlobalStr,DefaultFilePath);
		strcat(GlobalStr,MIDI_TEMP_FILE);
		if(MIDICount<10)
			GlobalStr[strlen(DefaultFilePath)+7]=(char)('0'+MIDICount);
		else
			GlobalStr[strlen(DefaultFilePath)+7]=(char)('a'+(MIDICount-10));
		DeleteFile(GlobalStr);
		MIDICount++;
	}
	GlobalUnlock(MusicDataMem);
	if(GlobalFree(MusicDataMem))
	{
		DisplayErrorMessage("Failed to free memory for music.\nSystem may become unstable.",
							"Error - CDirectMusic::LoadMusicFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL CDirectMusic::ReleaseMusic(MUSIC_DATA_PTR musicdataptr)
{
	if(MusicInit&&musicdataptr->SongLoaded)
	{
		Stop(musicdataptr);
		musicdataptr->segment->SetParam(GUID_Unload,
										-1,
										0,
										0,
										(LPVOID)lpDirectMusic);
		musicdataptr->segment->Release();
		musicdataptr->segment=NULL;
		MIDICount--;
	}
	musicdataptr->SongLoaded=FALSE;
	return(TRUE);
}

BOOL _fastcall CDirectMusic::Play(MUSIC_DATA_PTR musicdataptr)
{
	if(MusicInit&&MusicActive&&musicdataptr->SongLoaded)
	{
		lpDirectMusic->PlaySegment(musicdataptr->segment,
								   0,
								   0,
								   &musicdataptr->state);
		musicdataptr->SongPlaying=TRUE;
	}
	return(TRUE);
}

BOOL _fastcall CDirectMusic::Stop(MUSIC_DATA_PTR musicdataptr)
{
	if(MusicInit&&musicdataptr->SongLoaded&&musicdataptr->SongPlaying)
	{
		lpDirectMusic->Stop(musicdataptr->segment,
							NULL,
							0,
							0);
		musicdataptr->SongPlaying=FALSE;
	}
	return(TRUE);
}

BOOL CDirectMusic::ActivateMusic(void)
{
	if(MusicInit)
		MusicActive=TRUE;
	return(TRUE);
}

BOOL CDirectMusic::DeactivateMusic(void)
{
	MusicActive=FALSE;
	return(TRUE);
}

BOOL inline _fastcall CDirectMusic::IsMusicInitialized(void)
{
	return(MusicInit);
}

BOOL _fastcall CDirectMusic::Update(MUSIC_DATA_PTR musicdataptr)
{
	if(musicdataptr->SongPlaying)
		if(lpDirectMusic->IsPlaying(musicdataptr->segment,NULL)!=S_OK)
			Play(musicdataptr);
	return(TRUE);
}

BOOL CDirectMusic::Release(void)
{
	if(lpDirectMusicLoader)
	{
		lpDirectMusicLoader->Release();
		lpDirectMusicLoader=NULL;
	}
	if(lpDirectMusic)
	{
		lpDirectMusic->Stop(NULL,NULL,0,0);
		lpDirectMusic->CloseDown();
		lpDirectMusic->Release();
		lpDirectMusic=NULL;
	}
	return(TRUE);
}
#endif

// RAW DATA ENCODING FUNCTIONS //

#ifdef ENCODE_DATA
BOOL EncodeRawData(LPSTR filename,CDirectDraw* dd)
{
	DWORD BytesWritten;
	BYTE output;
	output=DTF_IDRAWDATA;
	if(!WriteFile(RecordFile,
				  (char*)&output,
				  sizeof(BYTE),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 1",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	WritePos=SetFilePointer(OutputFile,0,NULL,FILE_CURRENT);
	if(!WriteFile(RecordFile,
				  (char*)&WritePos,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Record error 2",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	output=DTF_IDRAWDATA;
	if(!WriteFile(OutputFile,
				  (char*)&output,
				  sizeof(char),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error A",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	HANDLE hFile;
	hFile=CreateFile(GlobalStr,
					 GENERIC_READ,
					 NULL,
					 NULL,
					 OPEN_EXISTING,
					 FILE_FLAG_SEQUENTIAL_SCAN,
					 NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Encode error 1",
				   "Error - EncodeRawData()",
							MB_OK,
							dd);
		return(FALSE);
	}
	DWORD FileSize=GetFileSize(hFile,NULL);
	if(!WriteFile(OutputFile,
				  (char*)&FileSize,
				  sizeof(DWORD),
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error 2",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		CloseHandle(hFile);
		return(FALSE);
	}
	HGLOBAL RawDataMem;
	LPSTR RawData;
	RawDataMem=GlobalAlloc(GHND,FileSize);
	if(!RawDataMem)
	{
		DisplayErrorMessage("Encode error 3",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		CloseHandle(hFile);
		return(FALSE);
	}
	RawData=(LPSTR)GlobalLock(RawDataMem);
	if(!RawData)
	{
		DisplayErrorMessage("Encode error 3a",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		GlobalFree(RawDataMem);
		CloseHandle(hFile);
		return(FALSE);
	}
	if(!ReadFile(hFile,
				 RawData,
				 FileSize,
				 &BytesWritten,
				 NULL))
	{
		DisplayErrorMessage("Encode error 4",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		GlobalUnlock(RawDataMem);
		GlobalFree(RawDataMem);
		CloseHandle(hFile);
		return(FALSE);
	}
	CloseHandle(hFile);
	if(!WriteFile(OutputFile,
				  RawData,
				  FileSize,
				  &BytesWritten,
				  NULL))
	{
		DisplayErrorMessage("Encode error 7",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		GlobalUnlock(RawDataMem);
		GlobalFree(RawDataMem);
		CloseHandle(hFile);
		return(FALSE);
	}
	GlobalUnlock(RawDataMem);
	if(GlobalFree(RawDataMem))
	{
		DisplayErrorMessage("Encode error 0xFF",
				   "Error - EncodeRawData()",
				   MB_OK|MB_ICONSTOP,
							dd);
		return(FALSE);
	}
	DisplayErrorMessage("Raw data successfully encoded.",
						"Information",
						MB_OK|MB_ICONINFORMATION,
						NULL);
	return(TRUE);
}
#endif

// RAW DATA FUNCTIONS //

BOOL LoadRawDataFromDataFile(RAW_DATA_PTR rawdataptr,CDirectDraw* lpDirectDraw)
{
	DWORD DataSize=0;
	BYTE DataType=NULL;
	DWORD BytesRead=0;
	HANDLE hFile=NULL;
	
	rawdataptr->hGlobal=NULL;
	rawdataptr->data=NULL;
	if(!ReadFile(DataFile,
				 (LPVOID)&DataType,
				 sizeof(BYTE),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read data ID from disk.",
							"Error - LoadRawDataFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	if(DataType!=DTF_IDRAWDATA)
	{
		DisplayErrorMessage("Data files have been corrupted.\nPlease re-install this program.",
							"Error - CDirectDraw::LoadRawDataFromDataFile()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 (LPVOID)&DataSize,
				 sizeof(DWORD),
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read data length from disk.",
							"Error - LoadRawDataFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	rawdataptr->hGlobal=GlobalAlloc(GHND,
									DataSize);
	if(!rawdataptr->hGlobal)
	{
		DisplayErrorMessage("Failed to allocate memory for data.",
							"Error - LoadRawDataFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	rawdataptr->data=GlobalLock(rawdataptr->hGlobal);
	if(!rawdataptr->data)
	{
		DisplayErrorMessage("Failed to lock memory for data.",
							"Error - LoadRawDataFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		GlobalFree(rawdataptr->hGlobal);
		return(FALSE);
	}
	if(!ReadFile(DataFile,
				 rawdataptr->data,
				 DataSize,
				 &BytesRead,
				 NULL))
	{
		DisplayErrorMessage("Failed to read data from disk.",
							"Error - LoadRawDataFromDataFile()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		GlobalUnlock(rawdataptr->hGlobal);
		GlobalFree(rawdataptr->hGlobal);
		return(FALSE);
	}
	return(TRUE);
}

BOOL ReleaseRawData(RAW_DATA_PTR rawdataptr,CDirectDraw* dd)
{
	if(rawdataptr->hGlobal)
	{
		GlobalUnlock(rawdataptr->hGlobal);
		if(GlobalFree(rawdataptr->hGlobal))
		{	
			DisplayErrorMessage("Failed to release data memory.\nSystem may become unstable.",
								"Error - ReleaseRawData()",
								MB_OK|MB_ICONINFORMATION,
								dd);
			return(FALSE);
		}
	}
	return(TRUE);
}

// DIRECT3D FUNCTIONS //

#ifdef INIT_DIRECT3D
HRESULT WINAPI Enum3DDevices(LPGUID lpguid,LPSTR sdevicedesc,LPSTR sdevicename,LPD3DDEVICEDESC hardwaredesc,LPD3DDEVICEDESC softwaredesc,LPVOID userarg)
{
	static BOOL Initialized=FALSE;
	HGLOBAL hGlobalTemp;
	BOOL HardwareFlag;
	LPD3DDEVICEDESC devicedesc;
	DWORD bitdepth;
	
	if(lpguid)
	{
		if(hardwaredesc->dcmColorModel)
		{
			HardwareFlag=TRUE;
			devicedesc=hardwaredesc;
		}
		else
		{
			HardwareFlag=FALSE;
			devicedesc=softwaredesc;
		}
		if(ActualBitsPerPixel==8)
			bitdepth=DDBD_8;
		if(ActualBitsPerPixel==16)
			bitdepth=DDBD_16;
		if(ActualBitsPerPixel==24)
			bitdepth=DDBD_24;
		if(ActualBitsPerPixel==32)
			bitdepth=DDBD_32;
		if(!(devicedesc->dwDeviceRenderBitDepth&bitdepth))
			return(D3DENUMRET_OK);
		if(HardwareNum!=0xFFFF&&!HardwareFlag)
			if(devicedesc->dcmColorModel==D3DCOLOR_RGB)
				return(D3DENUMRET_OK);
		if(!Initialized)
		{
			Direct3DListMem=GlobalAlloc(GHND,
										sizeof(GUID)+42);
			if(!Direct3DListMem)
			{
				DisplayErrorMessage("Failed to allocate memory for Direct3D data.\nDirect3D list may be incomplete.",
									"Error - CDirect3D::Enum3DDevices()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(D3DENUMRET_OK);
			}
			Direct3DList=(LPSTR)GlobalLock(Direct3DListMem);
			if(!Direct3DList)
			{
				DisplayErrorMessage("Failed to lock memory for Direct3D data.\nDirect3D list may be incomplete.",
									"Error - CDirect3D::Enum3DDevices()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(D3DENUMRET_OK);
			}
			CopyMemory(Direct3DList,
					   lpguid,
					   sizeof(GUID));
			CopyMemory(Direct3DList+sizeof(GUID),
					   sdevicename,
					   40);
			Direct3DList[sizeof(GUID)+40]=0;
			Direct3DList[sizeof(GUID)+41]=HardwareFlag;
			Initialized=TRUE;
			HardwareNum=Direct3DCount;
			Direct3DCount=1;
		}
		else
		{
			GlobalUnlock(Direct3DListMem);
			hGlobalTemp=GlobalReAlloc(Direct3DListMem,
									  (sizeof(GUID)+42)*(Direct3DCount+1),
									  NULL);
			if(!hGlobalTemp)
			{
				DisplayErrorMessage("Failed re-to allocate memory for Direct3D data.\nDirect3D list may be incomplete.",
									"Error - CDirect3D::Enum3DDevices()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(D3DENUMRET_OK);
			}
			Direct3DListMem=hGlobalTemp;
			Direct3DList=(LPSTR)GlobalLock(Direct3DListMem);
			if(!Direct3DList)
			{
				DisplayErrorMessage("Failed to re-lock memory for Direct3D data.\nDirect3D list may be incomplete.",
									"Error - CDirect3D::Enum3DDevices()",
									MB_OK|MB_ICONINFORMATION,
									NULL);
				return(D3DENUMRET_OK);
			}
			CopyMemory(Direct3DList+((sizeof(GUID)+42)*(Direct3DCount)),
					   lpguid,
					   sizeof(GUID));
			CopyMemory(Direct3DList+((sizeof(GUID)+42)*(Direct3DCount))+sizeof(GUID),
					   sdevicename,
					   40);
			Direct3DList[((sizeof(GUID)+42)*(Direct3DCount))+sizeof(GUID)+40]=0;
			Direct3DList[((sizeof(GUID)+42)*(Direct3DCount))+sizeof(GUID)+41]=HardwareFlag;
			HardwareNum=Direct3DCount;
			Direct3DCount++;
		}
	}
	else
	{
		if(Initialized)
		{
			if(Direct3DList)
				GlobalUnlock(Direct3DListMem);
			if(Direct3DListMem)
				if(GlobalFree(Direct3DListMem))
				{
					DisplayErrorMessage("Failed to free memory for Direct3D list.\nSystem may become unstable.",
									"Error - CDirect3D::Enum3DDevices()",
										MB_OK|MB_ICONINFORMATION,
										NULL);
				return(D3DENUMRET_OK);
				}
		}
	}
	return(D3DENUMRET_OK);
}

BOOL CDirect3D::Initialize(DIRECTDRAWINITSTRUCT_PTR lpddis,CDirectDraw* lpdd)
{
	DWORD count;
	
	lpDirectDraw=lpdd;
	lpDirect3D=NULL;
	lpDirect3DDevice=NULL;
	HardwareAccelActive=FALSE;
	lpDirect3DViewport=NULL;
	if(FAILED(lpDirectDraw->GetDirectDrawInterfacePointer()->QueryInterface(IID_IDirect3D3,
																			(LPVOID*)&lpDirect3D)))
	{
		DisplayErrorMessage("Failed to create Direct3D interface.",
							"Error - CDirect3D::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	HardwareNum=0xFFFF;
	if(FAILED(lpDirect3D->EnumDevices(Enum3DDevices,
									  NULL)))
	{
		DisplayErrorMessage("Failed to enumerate valid Direct3D devices.",
							"Error - CDirect3D::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(!Direct3DCount)
	{
		DisplayErrorMessage("Failed to find valid Direct3D devices.",
							"Error - CDirect3D::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(HardwareNum!=0xFFFF)
	{
		if(!SelectDirect3DDevice(HardwareNum))
			return(FALSE);
	}
	else
	{
		if(!SelectDirect3DDevice(0))
			return(FALSE);
	}
	if(FAILED(lpDirect3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE,
											   D3DSHADE_GOURAUD)))
		if(FAILED(lpDirect3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE,
												   D3DSHADE_FLAT)))
		{
			DisplayErrorMessage("Failed to set shading mode.",
								"Error - CDirect3D::Initialize()",
								MB_OK|MB_ICONSTOP,
								lpDirectDraw);
			return(FALSE);
		}
	if(FAILED(lpDirect3D->CreateViewport(&lpDirect3DViewport,
										 NULL)))
	{
		DisplayErrorMessage("Failed to create Direct3D viewport.",
							"Error - CDirect3D::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(FAILED(lpDirect3DDevice->AddViewport(lpDirect3DViewport)))
	{
		DisplayErrorMessage("Failed to attach Direct3D viewport.",
							"Error - CDirect3D::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	INIT_STRUCT(d3dviewport);
	d3dviewport.dwX=0;
	d3dviewport.dwY=0;
	d3dviewport.dwWidth=lpddis->dwScreenWidth;
	d3dviewport.dwHeight=lpddis->dwScreenHeight;
	d3dviewport.dvClipX=-1.0f;
	d3dviewport.dvClipY=1.0f;
	d3dviewport.dvClipWidth=2.0f;
	d3dviewport.dvClipHeight=2.0f;
	d3dviewport.dvMinZ=0.0f;
	d3dviewport.dvMaxZ=1.0f;
	if(FAILED(lpDirect3DViewport->SetViewport2(&d3dviewport)))
	{
		DisplayErrorMessage("Failed to set Direct3D viewport properties.",
							"Error - CDirect3D::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	for(count=0;count<256;count++)
	{
		float rads=(float)count;
		rads/=128;
		rads*=(float)3.14159;
		sintab[count]=(float)sin((double)rads);
		costab[count]=(float)cos((double)rads);
	}
	IdentityMatrix._11=1;
	IdentityMatrix._12=0;
	IdentityMatrix._13=0;
	IdentityMatrix._14=0;
	IdentityMatrix._21=0;
	IdentityMatrix._22=1;
	IdentityMatrix._23=0;
	IdentityMatrix._24=0;
	IdentityMatrix._31=0;
	IdentityMatrix._32=0;
	IdentityMatrix._33=1;
	IdentityMatrix._34=0;
	IdentityMatrix._41=0;
	IdentityMatrix._42=0;
	IdentityMatrix._43=0;
	IdentityMatrix._44=1;
	ScreenRect.x1=0;
	ScreenRect.y1=0;
	ScreenRect.x2=lpddis->dwScreenWidth;
	ScreenRect.y2=lpddis->dwScreenHeight;
	ScreenRect.lX1=0;
	ScreenRect.lY1=0;
	ScreenRect.lX2=lpddis->dwScreenWidth;
	ScreenRect.lY2=lpddis->dwScreenHeight;
	ActualBitsPerPixel=(BYTE)lpddis->dwScreenBitsPerPixel;
	return(TRUE);
}

LPDIRECT3D3 inline _fastcall CDirect3D::GetDirect3DInterfacePointer(void)
{
	return(lpDirect3D);
}

BOOL CDirect3D::Release(void)
{
	if(lpDirect3DViewport)
	{
		lpDirect3DDevice->DeleteViewport(lpDirect3DViewport);
		lpDirect3DViewport->Release();
		lpDirect3DViewport=NULL;
	}
	if(lpDirect3DDevice)
	{
		lpDirect3DDevice->Release();
		lpDirect3DDevice=NULL;
	}
	if(lpDirect3D)
	{
		lpDirect3D->Release();
		lpDirect3D=NULL;
	}
	return(TRUE);
}

BOOL inline _fastcall CDirect3D::IsHardwareAccelerationActive(void)
{
	return(HardwareAccelActive);
}

BOOL CDirect3D::SelectDirect3DDevice(DWORD devicenum)
{
	GUID guidtemp;
	
	CopyMemory(&guidtemp,
			   Direct3DList+((sizeof(GUID)+42)*devicenum),
			   sizeof(GUID));
	if(FAILED(lpDirect3D->CreateDevice(guidtemp,
									   lpDirectDraw->GetSecondaryDirectDrawSurfaceInterfacePointer(),
									   &lpDirect3DDevice,
									   NULL)))
	{
		DisplayErrorMessage("Failed to create a valid Direct3D device.",
							"Error - CDirect3D::Initialize()",
							MB_OK|MB_ICONSTOP,
							lpDirectDraw);
		return(FALSE);
	}
	if(Direct3DList[((sizeof(GUID)+42)*devicenum)+41])
		HardwareAccelActive=TRUE;
	else
		HardwareAccelActive=FALSE;
	Enum3DDevices(NULL,NULL,NULL,NULL,NULL,NULL);
	return(TRUE);
}

LPSTR CDirect3D::GetDirect3DDeviceName(DWORD devicenum)
{
	return(Direct3DList+((sizeof(GUID)+42)*devicenum)+sizeof(GUID));
}

BOOL _inline CDirect3D::BeginScene(void)
{
	if(FAILED(lpDirect3DDevice->BeginScene()))
	{
		DisplayErrorMessage("Failed to begin render.",
							"Error - CDirect3D::BeginScene()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _inline CDirect3D::EndScene(void)
{
	if(FAILED(lpDirect3DDevice->EndScene()))
	{
		DisplayErrorMessage("Failed to end render.",
							"Error - CDirect3D::EndScene()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

LPDIRECT3DDEVICE3 inline _fastcall CDirect3D::GetDirect3DDeviceInterfacePointer(void)
{
	return(lpDirect3DDevice);
}

BOOL inline _fastcall CDirect3D::CreateMatrix(LPD3DMATRIX lpmtx,float xp,float yp,float zp,float xs,float ys,float zs,BYTE yw,BYTE pt,BYTE rl)
{
	float A=costab[yw];
	float B=sintab[yw];
	float C,D;
	if(xs==1)
	{
		C=A;
		D=B;
	}
	else
	{
		C=xs*A;
		D=xs*B;
	}
	float E,F;
	if(zs==1)
	{
		E=B;
		F=A;
	}
	else
	{
		E=zs*B;
		F=zs*A;
	}
	float G=(xp*A)+(zp*B);
	float H=(xp*B)-(zp*A);
	float J=costab[pt];
	float K=sintab[pt];
	float L=D*K;
	float M=D*J;
	float N,P;
	if(ys==1)
	{
		N=J;
		P=K;
	}
	else
	{
		N=ys*J;
		P=ys*K;
	}
	float Q=K*F;
	float R=F*J;
	float S=(yp*J)+(H*K);
	float T=(yp*K)-(H*J);
	float U=costab[rl];
	float V=sintab[rl];
	lpmtx->_11=(C*U)-(L*V);
	lpmtx->_12=(C*V)+(L*U);
	lpmtx->_13=-M;
	lpmtx->_14=0;
	lpmtx->_21=-(N*V);
	lpmtx->_22=N*U;
	lpmtx->_23=P;
	lpmtx->_24=0;
	lpmtx->_31=(E*U)+(Q*V);
	lpmtx->_32=(E*V)-(Q*U);
	lpmtx->_33=R;
	lpmtx->_34=0;
	lpmtx->_41=(G*U)-(S*V);
	lpmtx->_42=(G*V)+(S*U);
	lpmtx->_43=T;
	lpmtx->_44=1;	
	return(TRUE);
}

BOOL _fastcall CDirect3D::SetViewMatrix(LPD3DMATRIX lpmtx)
{
	if(FAILED(lpDirect3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,
											 lpmtx)))
	{
		DisplayErrorMessage("Failed to set viewpoint.",
							"Error - CDirect3D::SetViewMatrix()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirect3D::SetProjectionMatrix(LPD3DMATRIX lpmtx)
{
	if(FAILED(lpDirect3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
											 lpmtx)))
	{
		DisplayErrorMessage("Failed to set projection angle.",
							"Error - CDirect3D::SetProjectionMatrix()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirect3D::SetWorldMatrix(LPD3DMATRIX lpmtx)
{
	if(FAILED(lpDirect3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,
											 lpmtx)))
	{
		DisplayErrorMessage("Failed to place object.",
							"Error - CDirect3D::SetWorldMatrix()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirect3D::PositionCamera(float xp,float yp,float zp,float xs,float ys,float zs,BYTE yaw,BYTE pitch,BYTE roll)
{
	if(!CreateMatrix(&matrix,xp,yp,zp,xs,ys,zs,yaw,pitch,roll))
		return(FALSE);
	if(!SetProjectionMatrix(&matrix))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirect3D::PositionObject(float xp,float yp,float zp,float xs,float ys,float zs,BYTE yaw,BYTE pitch,BYTE roll)
{
	if(!CreateMatrix(&matrix,xp,yp,zp,xs,ys,zs,yaw,pitch,roll))
		return(FALSE);
	if(!SetWorldMatrix(&matrix))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirect3D::ClearViewport(void)
{
	if(FAILED(lpDirect3DViewport->Clear2(1,
										 &ScreenRect,
										 D3DCLEAR_TARGET,
										 D3DRGBA(0,0,0,0),
										 NULL,
										 NULL)))
	{
		DisplayErrorMessage("Failed to clear viewport.",
							"Error - CDirect3D::ClearViewport()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirect3D::DrawPointList(LPVOID ptr,DWORD count)
{
	if(FAILED(lpDirect3DDevice->DrawPrimitive(D3DPT_POINTLIST,
											  D3DFVF_LVERTEX,
											  ptr,
											  count,
											  D3DDP_WAIT)))
	{
		DisplayErrorMessage("Failed to render point list.",
							"Error - CDirect3D::DrawPointList()",
							MB_OK|MB_ICONINFORMATION,
							lpDirectDraw);
		return(FALSE);
	}
	return(TRUE);
}

BOOL _fastcall CDirect3D::ProjectCamera(float d)
{
	matrix._11=1;
	matrix._12=0;
	matrix._13=0;
	matrix._14=0;
	matrix._21=0;
	matrix._22=1;
	matrix._23=0;
	matrix._24=0;
	matrix._31=0;
	matrix._32=0;
	matrix._33=1;
	matrix._34=1/d;
	matrix._41=0;
	matrix._42=0;
	matrix._43=-d;
	matrix._44=1;
	if(!SetProjectionMatrix(&matrix))
		return(FALSE);
	return(TRUE);
}

BOOL _fastcall CDirect3D::TransformPoint(LPD3DMATRIX lpmtx,float x,float y,float z,float *x1,float *y1,float *z1)
{
	*x1=(x*lpmtx->_11)+(y*lpmtx->_21)+(z*lpmtx->_31)+lpmtx->_41;
	*y1=(x*lpmtx->_12)+(y*lpmtx->_22)+(z*lpmtx->_32)+lpmtx->_42;
	*z1=(x*lpmtx->_13)+(y*lpmtx->_23)+(z*lpmtx->_33)+lpmtx->_43;
	return(TRUE);
}

#endif

// END //
