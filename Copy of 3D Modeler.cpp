// REQ'D DEFINES //

#define ID_APPICON NULL
#define ID_APPCURSOR NULL
#define ID_APPMENU NULL
#define ID_WINDOWTITLE "3-D Modeler"
#define ID_WINDOWCLASS "3DMODELER_CLS"

#define APP_HINST hAppInst
#define APP_HWND hAppWnd

#define CALL_INIT_PROC Initialize
#define CALL_MAIN_PROC AppMain
#define CALL_REST_PROC Restore

//#define ENCODE_DATA
#define MOUSE_TRACKING
//#define INIT_DIRECT3D
//#define INIT_SOUND

typedef long fixed;
#define FPROT 8
#define FPMPLR 256
#define FPSET(a) (((fixed)a)<<FPROT)
#define FPMUL(a,b) ((((fixed)a)*((fixed)b))>>FPROT)
#define FPDIV(a,b) ((((fixed)a)<<FPROT)/((fixed)b))
#define FPREAD(a) (((fixed)a)>>FPROT)
#define FPDSET(a) ((fixed)(a*FPMPLR))
#define FPDREAD(a) (((double)(a))/FPMPLR)
#define FPSQUARED(a) ((fixed)(FPMUL((fixed)a,(fixed)a)))

#define CMD_SETCOLOR (BYTE)0x01
#define CMD_RENDER (BYTE)0x80
#define CMD_MOVEOBJECT (BYTE)0x81
#define CMD_DRAWLINE (BYTE)0x82
#define CMD_YPRWORLD (BYTE)0x83
#define CMD_DRAWPOLY (BYTE)0x84
#define CMD_MOVEWORLD (BYTE)0x85
#define CMD_YPROBJECT (BYTE)0x86
#define CMD_ROTYAW (BYTE)0x87
#define CMD_ROTPITCH (BYTE)0x88
#define CMD_ROTROLL (BYTE)0x89
#define CMD_SETYAW (BYTE)0x8A
#define CMD_SETPITCH (BYTE)0x8B
#define CMD_SETROLL (BYTE)0x8C
#define CMD_PERSPECTIVE (BYTE)0x8D
#define CMD_BEGINKEY (BYTE)0x8E
#define CMD_ENDKEY (BYTE)0x8F
#define CMD_POLYRGB (BYTE)0x90
#define CMD_ENDOFFILE (BYTE)0xFF

// CONSTANTS //

#define SCRWD 640
#define SCRHT 480
#define SCRBP 16
#define NUMBACKBUFFERS 2

// INCLUDES //

#include"directx.h"
#include<math.h>

// MACROS //

#define INIT_READ renderdata[readpos++]=cmd;
#define SEEK_READ if(!SeekNextCommand()) return(FALSE); if(!ReadNumber(&input))	return(FALSE); renderdata[readpos++]=(BYTE)input;
#define READ_NEXT if(!ReadNumber(&input)) return(FALSE); renderdata[readpos++]=(BYTE)input;
#define READ_NEXT_SHORT if(!ReadNumber(&input)) return(FALSE); renderdata[readpos++]=(BYTE)(input>>8); renderdata[readpos++]=(BYTE)(input&0xFF);

// PROTOTYPES //

void EncodeDataFiles(void);

// GLOBALS //

CDirectDraw *DirectDraw;
DIRECTDRAWINITSTRUCT ddis;
#ifdef INIT_DIRECT3D
CDirect3D *Direct3D;
#endif
CDirectInput *DirectInput;
BYTE ActualBPP;
BYTE ActualNBB;
LPSTR gbuffer;
DWORD gpitch;
FONT_DATA Font;
BYTE vYaw,vPitch,vRoll;
fixed sintab[256],costab[256];
HGLOBAL zbuffermem=NULL;
LPBYTE zbuffer=NULL;
long wx=0,wy=0,wz=0;

// CLASSES //

typedef struct TAG_POINT3D
{
	fixed x,y,z;
	fixed tx,ty,tz;
} POINT3D,*POINT3D_PTR;

typedef struct TAG_POINTSPEC
{
	long obj,vtx;
} POINTSPEC,*POINTSPEC_PTR;

typedef struct TAG_OBJECT_DATA
{
	long numvtx;
	POINT3D_PTR lpvtx;
	HGLOBAL hglobal;
	BYTE yw,pt,rl;
	BYTE tyw,tpt,trl;
} OBJECT_DATA,*OBJECT_DATA_PTR;

class C3DModel
{
public:
	BOOL LoadModelFile(LPSTR filename);
	BOOL UnloadModelFile(void);
	BOOL Render(long x,long y,long z,BYTE yaw,BYTE pitch,BYTE roll);
protected:
	long numobj;
	HANDLE hfile;
	HGLOBAL hobjmem;
	OBJECT_DATA_PTR lpobj;
	HGLOBAL hrendermem;
	LPBYTE renderdata;

	BOOL ReadNumber(long *n);
	BOOL ReadByte(LPBYTE lpb);
	BOOL SeekNextCommand(void);
	BOOL InitializeModel(long no);
	BOOL LoadObject(long objnum);
	BOOL UnloadObject(long objnum);
	BOOL ReadCoordinates(POINT3D_PTR lppt);
	BOOL ReadPointSpec(POINTSPEC_PTR);
	BOOL GetNextCommand(LPBYTE);
};

C3DModel* Jasmine;

// FUNCTIONS //

BOOL Initialize(void)
{
	long count;
	
	while(ShowCursor(FALSE)>=0)
	{}
	srand(GetTickCount());
	//SetDefaultFilePath("c:\\windows\\msremote.sfs\\program files\\3d modeler\\");
	//SetDefaultFilePath("c:\\windows\\desktop\\randy's folder\\3d modeler\\");
	SetDefaultFilePath("c:\\randy's folder\\3d modeler\\");
	if(!(DirectDraw=new CDirectDraw()))
	{
		DisplayErrorMessage("Failed to create CDirectDraw class instance.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	ddis.dwScreenWidth=SCRWD;
	ddis.dwScreenHeight=SCRHT;
	ddis.dwScreenBitsPerPixel=SCRBP;
	ddis.dwBackBufferCount=NUMBACKBUFFERS;
	if(!DirectDraw->Initialize(&ddis))
		return(FALSE);
	ActualBPP=(BYTE)ddis.dwScreenBitsPerPixel;
	ActualNBB=(BYTE)ddis.dwBackBufferCount;
#ifdef INIT_DIRECT3D
	if(!(Direct3D=new CDirect3D))
	{
		DisplayErrorMessage("Failed to create CDirect3D class instance.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							NULL);
		return(FALSE);
	}
	if(!Direct3D->Initialize(&ddis,DirectDraw))
		return(FALSE);
#endif
	if(!(DirectInput=new CDirectInput()))
	{
		DisplayErrorMessage("Failed to create CDirectInput class instance.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							DirectDraw);
		return(FALSE);
	}
	EncodeDataFiles();
	if(!VerifyDataFiles("3D Modeler.rec",DirectDraw))
		return(FALSE);
	OpenDataFile("3D Modeler.dat",DirectDraw,FALSE);
	DirectDraw->LoadFontFromDataFile(&Font);
	CloseDataFile(DirectDraw);
	DirectDraw->SetDirectDrawPaletteEntry(255,255,255,255);
	DirectDraw->SetFontColor(&Font,255);
	if(!DirectInput->Initialize(DirectDraw))
		return(FALSE);
	DirectInput->DetectJoysticks();
	if(JoystickCount)
		DirectInput->SelectJoystick(0);
	for(count=0;count<256;count++)
	{
		double r=(double)count;
		r/=128;
		r*=(double)3.14159;
		double c=cos(r)*FPMPLR;
		double s=sin(r)*FPMPLR;
		costab[count]=(fixed)c;
		sintab[count]=(fixed)s;
	}
	zbuffermem=GlobalAlloc(GHND,SCRWD*SCRHT);
	if(!zbuffermem)
	{
		DisplayErrorMessage("Failed to allocate memory for z-buffer.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							DirectDraw);
		return(FALSE);
	}
	zbuffer=(LPBYTE)GlobalLock(zbuffermem);
	if(!zbuffer)
	{
		DisplayErrorMessage("Failed to lock memory for z-buffer.",
							"Error - Initialize()",
							MB_OK|MB_ICONSTOP,
							DirectDraw);
		return(FALSE);
	}

	Jasmine=new C3DModel();
	Jasmine->LoadModelFile("jasmine.3dm");
	
	return(TRUE);
}

void AppMain(void)
{
	static DWORD fpsTickCount=GetTickCount();
	static DWORD frame=0;
	static DWORD fps=0;
	DWORD TickCount;
	DWORD ProgramComplete=FALSE;
	
	TickCount=GetTickCount();

// BEGIN FRAME RENDER //////////////////////////////////////////////////	

	CONTROL_DATA UserInput;
	CONTROL_DATA UserInputNew;
	DWORD keyval;

	DirectInput->GetUserInput(&UserInput,&UserInputNew);
	if(UserInputNew.esc)
		ProgramComplete=TRUE;

	DirectDraw->ClearSecondaryDirectDrawSurface();

	vYaw=(BYTE)m_xp;
	if(!m_bt)
		vPitch=(BYTE)m_yp;
	else
		vRoll=(BYTE)m_yp;

	if(UserInput.up)
		wy--;
	if(UserInput.down)
		wy++;
	if(UserInput.left)
		wx--;
	if(UserInput.right)
		wx++;
	if(UserInput.btn1)
		wz--;
	if(UserInput.btn4)
		wz++;
	DirectInput->ScanKeyboard((LPSTR*)&GlobalStr,&keyval);
	if(keyval)
	{
		sprintf(GlobalStr,"Key: %d",keyval);
		DirectDraw->BufferString(&Font,GlobalStr,0,464,NULL,FALSE);
	}
	Jasmine->Render(wx,wy,wz,vYaw,vPitch,vRoll);

	sprintf(GlobalStr,"%d",fps);
	DirectDraw->BufferString(&Font,GlobalStr,0,0,NULL,FALSE);

// END FRAME RENDER ////////////////////////////////////////////////////

	while(GetTickCount()<TickCount+33)
	{}
	DirectDraw->PerformPageFlip();
	frame++;
	if(GetTickCount()>=(fpsTickCount+1000))
	{
		fpsTickCount=GetTickCount();
		fps=frame;
		frame=0;
	}
	if(ProgramComplete)
	{
		PostMessage(hAppWnd,WM_QUIT,NULL,NULL);
		return;
	}
}

void Restore(void)
{
	Jasmine->UnloadModelFile();
	delete Jasmine;
	
	if(zbuffer)
		GlobalUnlock(zbuffermem);
	if(zbuffermem)
		if(GlobalFree(zbuffermem))
			DisplayErrorMessage("Failed to free memory.\nSystem may become unstable.",
								"Error - Restore()",
								MB_OK|MB_ICONINFORMATION,
								DirectDraw);
	DirectDraw->ReleaseFont(&Font);
	DirectInput->Release();
	delete DirectInput;
	DirectDraw->ClearPrimaryDirectDrawSurface();
#ifdef INIT_DIRECT3D
	Direct3D->Release();
	delete Direct3D;
#endif
	DirectDraw->Release();
	delete DirectDraw;
	while(ShowCursor(TRUE)<0)
	{}
}

void EncodeDataFiles(void)
{
#ifdef ENCODE_DATA
	OpenRecorder("3D Modeler.rec");
		OpenEncoder("3D Modeler.dat");
			EncodeFont("cronos.ftd",DirectDraw);
		CloseEncoder();
	CloseRecorder();
#endif
}

void TransformPoint(POINT3D_PTR lpp,BYTE yaw,BYTE pitch,BYTE roll)
{
	fixed x1=FPMUL(lpp->tz,sintab[yaw])+FPMUL(lpp->tx,costab[yaw]);
	fixed y1=lpp->ty;
	fixed z1=FPMUL(lpp->tz,costab[yaw])-FPMUL(lpp->tx,sintab[yaw]);
	fixed x2=x1;
	fixed y2=FPMUL(y1,costab[pitch])-FPMUL(z1,sintab[pitch]);
	fixed z2=FPMUL(y1,sintab[pitch])+FPMUL(z1,costab[pitch]);
	fixed x3=FPMUL(y2,sintab[roll])+FPMUL(x2,costab[roll]);
	fixed y3=FPMUL(y2,costab[roll])-FPMUL(x2,sintab[roll]);
	fixed z3=z2;
	lpp->tx=x3;
	lpp->ty=y3;
	lpp->tz=z3;
}

void SetPoint(POINT3D_PTR lpp,BYTE color)
{
	long x=FPREAD(lpp->tx)+(SCRWD/2);
	long y=FPREAD(lpp->ty)+(SCRHT/2);
	BYTE r,g,b,c1,c2;
	long pos;
	
	if(x<0||x>SCRWD-1||y<0||y>SCRHT-1)
		return;
	DirectDraw->GetDirectDrawPaletteEntry(color,&r,&g,&b);
	r=r>>3;
	g=g>>2;
	b=b>>3;
	c1=b+((g&7)<<5);
	c2=(g>>3)+(r<<3);
	pos=(x*2)+(y*gpitch);
	gbuffer[pos]=c1;
	gbuffer[pos+1]=c2;
}

void DrawVector(POINT3D_PTR lpp1,POINT3D_PTR lpp2,BYTE color)
{
	long x1=FPREAD(lpp1->tx)+(SCRWD/2);
	long y1=FPREAD(lpp1->ty)+(SCRHT/2);
	long x2=FPREAD(lpp2->tx)+(SCRWD/2);
	long y2=FPREAD(lpp2->ty)+(SCRHT/2);
	BYTE r,g,b,c1,c2;
	long pos;
	long dx,dy,dx2,dy2,xi,yi,error,index;

	if(x1<0||x1>SCRWD-1||y1<0||y1>SCRHT-1)
		return;
	if(x2<0||x2>SCRWD-1||y2<0||y2>SCRHT-1)
		return;
	DirectDraw->GetDirectDrawPaletteEntry(color,&r,&g,&b);
	r=r>>3;
	g=g>>2;
	b=b>>3;
	c1=b+((g&7)<<5);
	c2=(g>>3)+(r<<3);
	pos=(x1*2)+(y1*gpitch);
	dx=x2-x1;
	dy=y2-y1;
	if(dx>=0)
		xi=2;
	else
	{
		xi=-2;
		dx=-dx;
	}
	if(dy>=0)
		yi=gpitch;
	else
	{
		yi=gpitch;
		yi=-yi;
		dy=-dy;
	}
	dx2=dx<<1;
	dy2=dy<<1;
	if(dx>dy)
	{
		error=dy2-dx;
		for(index=0;index<=dx;index++)
		{
			gbuffer[pos]=c1;
			gbuffer[pos+1]=c2;
			if(error>=0)
			{
				error-=dx2;
				pos+=yi;
			}
			error+=dy2;
			pos+=xi;
		}
	}
	else
	{
		error=dx2-dy;
		for(index=0;index<=dy;index++)
		{
			gbuffer[pos]=c1;
			gbuffer[pos+1]=c2;
			if(error>=0)
			{
				error-=dy2;
				pos+=xi;
			}
			error+=dx2;
			pos+=yi;
		}
	}
}

void AddPoints(POINT3D_PTR lpp1,POINT3D_PTR lpp2)
{
	lpp1->tx+=lpp2->tx;
	lpp1->ty+=lpp2->ty;
	lpp1->tz+=lpp2->tz;
}

void InitTransform(POINT3D_PTR lpp)
{
	lpp->tx=lpp->x;
	lpp->ty=lpp->y;
	lpp->tz=lpp->z;
}

void ClearZBuffer(void)
{
	ZeroMemory(zbuffer,SCRWD*SCRHT);
}

void DrawPolygon(POINT3D_PTR lpp1,POINT3D_PTR lpp2,POINT3D_PTR lpp3,BYTE color)
{
	long x1=FPREAD(lpp1->tx)+(SCRWD/2);
	long y1=FPREAD(lpp1->ty)+(SCRHT/2);
	long z1=FPREAD(lpp1->tz)+128;
	long x2=FPREAD(lpp2->tx)+(SCRWD/2);
	long y2=FPREAD(lpp2->ty)+(SCRHT/2);
	long z2=FPREAD(lpp2->tz)+128;
	long x3=FPREAD(lpp3->tx)+(SCRWD/2);
	long y3=FPREAD(lpp3->ty)+(SCRHT/2);
	long z3=FPREAD(lpp3->tz)+128;
	long x4,y4,z4;
	BYTE r,g,b,c1,c2;
	fixed xs,dxs,xe,dxe;
	fixed zs,dzs,ze,dze;
	fixed z,dz;
	long x,y;

	if(x1<0||x1>SCRWD-1||y1<0||y1>SCRHT-1)
		return;
	if(x2<0||x2>SCRWD-1||y2<0||y2>SCRHT-1)
		return;
	if(x3<0||x3>SCRWD-1||y3<0||y3>SCRHT-1)
		return;
	if(y3<y2)
	{
		Swap(&x2,&x3);
		Swap(&y2,&y3);
		Swap(&z2,&z3);
	}
	if(y2<y1)
	{
		Swap(&x1,&x2);
		Swap(&y1,&y2);
		Swap(&z1,&z2);
	}
	if(y3<y2)
	{
		Swap(&x2,&x3);
		Swap(&y2,&y3);
		Swap(&z2,&z3);
	}
	if(z1<0)
		z1=0;
	if(z1>255)
		z1=255;
	if(z2<0)
		z2=0;
	if(z2>255)
		z2=255;
	if(z3<0)
		z3=0;
	if(z3>255)
		z3=255;
	DirectDraw->GetDirectDrawPaletteEntry(color,&r,&g,&b);
	r=r>>3;
	g=g>>2;
	b=b>>3;
	c1=b+((g&7)<<5);
	c2=(g>>3)+(r<<3);
	if(y1==y3)
		return;
	if(y1!=y2)
	{
		x4=x1+FPREAD(FPMUL(FPSET(x3-x1),FPDIV(FPSET(y2-y1),FPSET(y3-y1)))+(FPMPLR/2));
		y4=y2;
		z4=z1+FPREAD(FPMUL(FPSET(z3-z1),FPDIV(FPSET(y2-y1),FPSET(y3-y1)))+(FPMPLR/2));
		xs=FPSET(x1);
		dxs=FPDIV(FPSET(x2-x1),FPSET(y2-y1));
		xe=FPSET(x1);
		dxe=FPDIV(FPSET(x4-x1),FPSET(y4-y1));
		zs=FPSET(z1);
		dzs=FPDIV(FPSET(z2-z1),FPSET(y2-y1));
		ze=FPSET(z1);
		dze=FPDIV(FPSET(z4-z1),FPSET(y4-y1));
		if(x2>x4)
		{
			Swap(&xs,&xe);
			Swap(&dxs,&dxe);
			Swap(&zs,&ze);
			Swap(&dzs,&dze);
		}
		for(y=y1;y<y2;y++)
		{
			z=zs;
			if(xs==xe)
				dz=0;
			else
				dz=FPDIV(FPSET(ze-zs),FPSET(xe-xs));
			for(x=FPREAD(xs);x<=FPREAD(xe);x++)
			{
				if(zbuffer[x+(y*SCRWD)]<=(BYTE)FPREAD(z))
				{
					gbuffer[(x*2)+(y*gpitch)]=c1;
					gbuffer[(x*2)+(y*gpitch)+1]=c2;
					zbuffer[x+(y*SCRWD)]=(BYTE)FPREAD(z);
				}
				z+=dz;
			}
			xs+=dxs;
			xe+=dxe;
			zs+=dzs;
			ze+=dze;
		}
	}
	else
	{
		x4=x1;
		y4=y1;
		z4=z1;
	}
	if(y2!=y3)
	{
		xs=FPSET(x2);
		dxs=FPDIV(FPSET(x3-x2),FPSET(y3-y2));
		xe=FPSET(x4);
		dxe=FPDIV(FPSET(x3-x4),FPSET(y3-y4));
		zs=FPSET(z2);
		dzs=FPDIV(FPSET(z3-z2),FPSET(y3-y2));
		ze=FPSET(z4);
		dze=FPDIV(FPSET(z3-z4),FPSET(y3-y4));
		if(xs>xe)
		{
			Swap(&xs,&xe);
			Swap(&dxs,&dxe);
			Swap(&zs,&ze);
			Swap(&dzs,&dze);
		}
		for(y=y2;y<=y3;y++)
		{
			z=zs;
			if(xs==xe)
				dz=0;
			else
				dz=FPDIV(FPSET(ze-zs),FPSET(xe-xs));
			for(x=FPREAD(xs);x<=FPREAD(xe);x++)
			{
				if(zbuffer[x+(y*SCRWD)]<=(BYTE)FPREAD(z))
				{
					gbuffer[(x*2)+(y*gpitch)]=c1;
					gbuffer[(x*2)+(y*gpitch)+1]=c2;
					zbuffer[x+(y*SCRWD)]=(BYTE)FPREAD(z);
				}
				z+=dz;
			}
			xs+=dxs;
			xe+=dxe;
			zs+=dzs;
			ze+=dze;
		}
	}
}

BOOL C3DModel::LoadObject(long objnum)
{
	long count;

	if(!SeekNextCommand())
		return(FALSE);
	if(!ReadNumber(&lpobj[objnum].numvtx))
		return(FALSE);
	lpobj[objnum].hglobal=GlobalAlloc(GHND,
									  sizeof(POINT3D)*lpobj[objnum].numvtx);
	if(!lpobj[objnum].hglobal)
	{
		sprintf(GlobalStr,"Failed to allocate memory for 3-D object data. (%d bytes)",sizeof(POINT3D)*lpobj[objnum].numvtx);
		DisplayErrorMessage(GlobalStr,
							"Error - C3DModeler::LoadObject",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	lpobj[objnum].lpvtx=(POINT3D_PTR)GlobalLock(lpobj[objnum].hglobal);
	if(!lpobj[objnum].lpvtx)
	{
		DisplayErrorMessage("Failed to lock memory for 3-D object data.",
							"Error - C3DModeler::LoadObject",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	for(count=0;count<lpobj[objnum].numvtx;count++)
	{
		if(!ReadCoordinates(&lpobj[objnum].lpvtx[count]))
			return(FALSE);
	}
	lpobj[objnum].yw=0;
	lpobj[objnum].pt=0;
	lpobj[objnum].rl=0;
	return(TRUE);
}

BOOL C3DModel::ReadPointSpec(POINTSPEC_PTR lpps)
{
	if(!SeekNextCommand())
		return(FALSE);
	if(!ReadNumber(&lpps->obj))
		return(FALSE);
	if(!ReadNumber(&lpps->vtx))
		return(FALSE);
	return(TRUE);
}

BOOL C3DModel::ReadCoordinates(POINT3D_PTR lppt)
{
	if(!SeekNextCommand())
		return(FALSE);
	if(!ReadNumber(&lppt->x))
		return(FALSE);
	lppt->x=FPSET(lppt->x);
	if(!ReadNumber(&lppt->y))
		return(FALSE);
	lppt->y=FPSET(lppt->y);
	if(!ReadNumber(&lppt->z))
		return(FALSE);
	lppt->z=FPSET(lppt->z);
	InitTransform(lppt);
	return(TRUE);
}

BOOL C3DModel::UnloadObject(long objnum)
{
	if(lpobj[objnum].lpvtx)
		GlobalUnlock(lpobj[objnum].hglobal);
	if(lpobj[objnum].hglobal)
		if(GlobalFree(lpobj[objnum].hglobal))
	{
		DisplayErrorMessage("Failed to free memory.\nSystem may become unstable.",
							"Error - C3DModeler::UnloadObject",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	return(TRUE);
}

BOOL C3DModel::ReadNumber(long *n)
{
	BYTE input;
	BOOL done=FALSE;
	BOOL neg=FALSE;
	long output=0;

	while(!done)
	{
		if(!ReadByte(&input))
			return(FALSE);
		if(input=='>'||
		   input==','||
		   input==':')
			done=TRUE;
		else if(input=='-')
			neg=TRUE;
		else
			output=(output*10)+(input-'0');
	}
	if(neg)
		output=-output;
	*n=output;
	return(TRUE);
}

BOOL C3DModel::SeekNextCommand(void)
{
	BYTE input=0;
	
	while(input!='<')
	{
		if(!ReadByte(&input))
			return(FALSE);
	}
	return(TRUE);
}

BOOL C3DModel::ReadByte(LPBYTE lpb)
{
	DWORD bytesread=0;

	if(ReadFile(hfile,
				lpb,
				sizeof(BYTE),
				&bytesread,
				NULL)==HFILE_ERROR)
	{
		DisplayErrorMessage("Failed to read 3-D model file.",
							"Error - C3DModel::ReadByte()",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}	
	return(TRUE);
}

BOOL C3DModel::UnloadModelFile(void)
{
	long count;
	
	for(count=0;count<numobj;count++)
		UnloadObject(count);
	if(lpobj)
		GlobalUnlock(hobjmem);
	if(hobjmem)
		if(GlobalFree(hobjmem))
			DisplayErrorMessage("Failed to release memory.\nSystem may become unstable.",
								"Error - C3DModel::UnloadModelFile",
								MB_OK|MB_ICONINFORMATION,
								NULL);
	if(renderdata)
		GlobalUnlock(hrendermem);
	if(hrendermem)
		if(GlobalFree(hrendermem))
			DisplayErrorMessage("Failed to release memory.\nSystem may become unstable.",
								"Error - C3DModel::UnloadModelFile",
								MB_OK|MB_ICONINFORMATION,
								NULL);
	return(TRUE);
}

BOOL C3DModel::InitializeModel(long no)
{
	numobj=no;
	hobjmem=GlobalAlloc(GHND,
						sizeof(OBJECT_DATA)*numobj);
	if(!hobjmem)
	{
		sprintf(GlobalStr,"Failed to allocate memory for 3-D model. (%d bytes)",sizeof(OBJECT_DATA)*numobj);
		DisplayErrorMessage(GlobalStr,
							"Error - C3DModel::InitializeModel()",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	lpobj=(OBJECT_DATA_PTR)GlobalLock(hobjmem);
	if(!lpobj)
	{
		DisplayErrorMessage("Failed to allocate memory for 3-D model.",
							"Error - C3DModel::InitializeModel()",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	return(TRUE);
}

BOOL C3DModel::GetNextCommand(LPBYTE lpb)
{
	char cmd[20];
	long pos=0;
	char input=0;
	BOOL done=FALSE;
	
	if(!SeekNextCommand())
		return(FALSE);
	while(!done)
	{
		if(!ReadByte((LPBYTE)&input))
			return(FALSE);
		if(input=='>')
			done=TRUE;
		else
			cmd[pos++]=input;
	}
	cmd[pos]=0;
	if(!strcmp(cmd,"clr"))
		*lpb=CMD_SETCOLOR;
	if(!strcmp(cmd,"render"))
		*lpb=CMD_RENDER;
	if(!strcmp(cmd,"move"))
		*lpb=CMD_MOVEOBJECT;
	if(!strcmp(cmd,"line"))
		*lpb=CMD_DRAWLINE;
	if(!strcmp(cmd,"yprw"))
		*lpb=CMD_YPRWORLD;
	if(!strcmp(cmd,"movw"))
		*lpb=CMD_MOVEWORLD;
	if(!strcmp(cmd,"ypr"))
		*lpb=CMD_YPROBJECT;
	if(!strcmp(cmd,"roty"))
		*lpb=CMD_ROTYAW;
	if(!strcmp(cmd,"rotp"))
		*lpb=CMD_ROTPITCH;
	if(!strcmp(cmd,"rotr"))
		*lpb=CMD_ROTROLL;
	if(!strcmp(cmd,"sety"))
		*lpb=CMD_SETYAW;
	if(!strcmp(cmd,"setp"))
		*lpb=CMD_SETPITCH;
	if(!strcmp(cmd,"setr"))
		*lpb=CMD_SETROLL;
	if(!strcmp(cmd,"poly"))
		*lpb=CMD_DRAWPOLY;
	if(!strcmp(cmd,"pers"))
		*lpb=CMD_PERSPECTIVE;
	if(!strcmp(cmd,"key"))
		*lpb=CMD_BEGINKEY;
	if(!strcmp(cmd,"endkey"))
		*lpb=CMD_ENDKEY;
	if(!strcmp(cmd,"prgb"))
		*lpb=CMD_POLYRGB;
	if(!strcmp(cmd,"end"))
		*lpb=CMD_ENDOFFILE;
	return(TRUE);
}

BOOL C3DModel::LoadModelFile(LPSTR filename)
{
	hfile=NULL;
	long count;
	BOOL done=FALSE;
	BYTE cmd;
	DWORD startpos;
	DWORD readpos;
	long input;
	long c,r,g,b;

	strcpy(GlobalStr,DefaultFilePath);
	strcat(GlobalStr,filename);
	hfile=CreateFile(GlobalStr,
					 GENERIC_READ,
					 NULL,
					 NULL,
					 OPEN_EXISTING,
					 NULL,
					 NULL);
	if(hfile==INVALID_HANDLE_VALUE)
	{
		DisplayErrorMessage("Failed to open 3-D model file.",
							"Error - C3DModel::LoadModelFile()",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	if(!SeekNextCommand())
		return(FALSE);
	if(!ReadNumber(&numobj))
		return(FALSE);
	if(!InitializeModel(numobj))
		return(FALSE);
	for(count=0;count<numobj;count++)
		if(!LoadObject(count))
			return(FALSE);
	while(!done)
	{
		if(!GetNextCommand(&cmd))
			return(FALSE);
		switch(cmd)
		{
			case(CMD_SETCOLOR):
				if(!SeekNextCommand())
					return(FALSE);
				if(!ReadNumber(&c))
					return(FALSE);
				if(!ReadNumber(&r))
					return(FALSE);
				if(!ReadNumber(&g))
					return(FALSE);
				if(!ReadNumber(&b))
					return(FALSE);
				if(!DirectDraw->SetDirectDrawPaletteEntry((BYTE)c,(BYTE)r,(BYTE)g,(BYTE)b))
					return(FALSE);
				break;
			case(CMD_RENDER):
				done=TRUE;
				break;
		}
	}
	done=FALSE;
	startpos=SetFilePointer(hfile,0,NULL,FILE_CURRENT);
	readpos=0;
	while(!done)
	{
		if(!GetNextCommand(&cmd))
			return(FALSE);
		switch(cmd)
		{
			case(CMD_MOVEOBJECT):
				readpos+=5;	
				break;
			case(CMD_DRAWLINE):
				readpos+=8;
				break;
			case(CMD_YPRWORLD):
				readpos+=2;
				break;
			case(CMD_MOVEWORLD):
				readpos+=2;
				break;
			case(CMD_YPROBJECT):
				readpos+=3;
				break;
			case(CMD_ROTYAW):
				readpos+=3;
				break;
			case(CMD_ROTPITCH):
				readpos+=3;
				break;
			case(CMD_ROTROLL):
				readpos+=3;
				break;
			case(CMD_SETYAW):
				readpos+=3;
				break;
			case(CMD_SETPITCH):
				readpos+=3;
				break;
			case(CMD_SETROLL):
				readpos+=3;
				break;
			case(CMD_DRAWPOLY):
				readpos+=11;
				break;
			case(CMD_PERSPECTIVE):
				readpos+=1;
				break;
			case(CMD_BEGINKEY):
				readpos+=2;
				break;
			case(CMD_ENDKEY):
				readpos+=1;
				break;
			case(CMD_POLYRGB):
				readpos+=13;
				break;
			case(CMD_ENDOFFILE):
				readpos+=1;
				done=1;
				break;
		}
	}
	hrendermem=GlobalAlloc(GHND,readpos);
	if(!hrendermem)
	{
		DisplayErrorMessage("Failed to allocate memory for render buffer.",
							"Error - C3DModel::LoadModelFile()",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	renderdata=(LPBYTE)GlobalLock(hrendermem);
	if(!renderdata)
	{
		DisplayErrorMessage("Failed to lock memory for render buffer.",
							"Error - C3DModel::LoadModelFile()",
							MB_OK|MB_ICONINFORMATION,
							NULL);
		return(FALSE);
	}
	SetFilePointer(hfile,startpos,NULL,FILE_BEGIN);
	readpos=0;
	done=FALSE;
	while(!done)
	{
		if(!GetNextCommand(&cmd))
			return(FALSE);
		switch(cmd)
		{
			case(CMD_MOVEOBJECT):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				READ_NEXT_SHORT;
				break;
			case(CMD_DRAWLINE):
				INIT_READ;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				break;
			case(CMD_YPRWORLD):
				INIT_READ;
				SEEK_READ;
				break;
			case(CMD_MOVEWORLD):
				INIT_READ;
				SEEK_READ;
				break;
			case(CMD_YPROBJECT):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				break;
			case(CMD_ROTYAW):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				break;
			case(CMD_ROTPITCH):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				break;
			case(CMD_ROTROLL):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				break;
			case(CMD_SETYAW):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				break;
			case(CMD_SETPITCH):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				break;
			case(CMD_SETROLL):
				INIT_READ;
				SEEK_READ;
				SEEK_READ;
				break;
			case(CMD_DRAWPOLY):
				INIT_READ;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				break;
			case(CMD_PERSPECTIVE):
				INIT_READ;
				break;
			case(CMD_BEGINKEY):
				INIT_READ;
				SEEK_READ;
				break;
			case(CMD_ENDKEY):
				INIT_READ;
				break;
			case(CMD_POLYRGB):
				INIT_READ;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				READ_NEXT_SHORT;
				SEEK_READ;
				READ_NEXT;
				READ_NEXT;
				break;
			case(CMD_ENDOFFILE):
				INIT_READ;
				done=1;
				break;
		}
	}
	CloseHandle(hfile);
	return(TRUE);
}

BOOL C3DModel::Render(long x,long y,long z,BYTE yaw,BYTE pitch,BYTE roll)
{
	long count,count2;
	BYTE done=FALSE;
	DWORD pos=0;
	BYTE cmd=0;
	long obj1,obj2,obj3,vtx1,vtx2,vtx3;
	BYTE clr;
	BYTE anginc;
	BOOL draw=TRUE;
	BYTE keyval;
	BYTE r,g,b;
			
	ClearZBuffer();
	DirectDraw->LockSecondaryDirectDrawSurface(&gbuffer,&gpitch);
	while(!done)
	{
		cmd=renderdata[pos++];
		switch(cmd)
		{
			case(CMD_MOVEOBJECT):
				obj1=(long)renderdata[pos++];
				obj2=(long)renderdata[pos++];
				vtx2=(long)renderdata[pos++];
				vtx2=(vtx2<<8)+(long)renderdata[pos++];
				if(draw)
					for(count=0;count<lpobj[obj1].numvtx;count++)
						AddPoints(&lpobj[obj1].lpvtx[count],
								  &lpobj[obj2].lpvtx[vtx2]);
				break;
			case(CMD_DRAWLINE):
				obj1=(long)renderdata[pos++];
				vtx1=(long)renderdata[pos++];
				vtx1=(vtx1<<8)+(long)renderdata[pos++];
				obj2=(long)renderdata[pos++];
				vtx2=(long)renderdata[pos++];
				vtx2=(vtx2<<8)+(long)renderdata[pos++];
				clr=renderdata[pos++];
				if(draw)
					DrawVector(&lpobj[obj1].lpvtx[vtx1],
							   &lpobj[obj2].lpvtx[vtx2],
							   clr);
				break;
			case(CMD_YPRWORLD):
				obj1=(long)renderdata[pos++];
				if(draw)
				{
					lpobj[obj1].tyw=lpobj[obj1].yw;
					lpobj[obj1].tpt=lpobj[obj1].pt;
					lpobj[obj1].trl=lpobj[obj1].rl;
					for(count=0;count<lpobj[obj1].numvtx;count++)
					{
						InitTransform(&lpobj[obj1].lpvtx[count]);
						TransformPoint(&lpobj[obj1].lpvtx[count],
									   lpobj[obj1].yw,
									   lpobj[obj1].pt,
									   lpobj[obj1].rl);
						TransformPoint(&lpobj[obj1].lpvtx[count],
									   yaw,
									   pitch,
									   roll);
					}
				}
				break;
			case(CMD_MOVEWORLD):
				obj1=(long)renderdata[pos++];
				if(draw)
					for(count=0;count<lpobj[obj1].numvtx;count++)
					{
						lpobj[obj1].lpvtx[count].tx+=FPSET(x);
						lpobj[obj1].lpvtx[count].ty+=FPSET(y);
						lpobj[obj1].lpvtx[count].tz+=FPSET(z);
					}
				break;
			case(CMD_YPROBJECT):
				obj1=(long)renderdata[pos++];
				obj2=(long)renderdata[pos++];
				if(draw)
				{
					lpobj[obj1].tyw=lpobj[obj1].yw+lpobj[obj2].yw;
					lpobj[obj1].tpt=lpobj[obj1].pt+lpobj[obj2].pt;
					lpobj[obj1].trl=lpobj[obj1].rl+lpobj[obj2].rl;
					for(count=0;count<lpobj[obj1].numvtx;count++)
					{
						InitTransform(&lpobj[obj1].lpvtx[count]);
						TransformPoint(&lpobj[obj1].lpvtx[count],
									   lpobj[obj1].tyw,
									   lpobj[obj1].tpt,
									   lpobj[obj1].trl);
						TransformPoint(&lpobj[obj1].lpvtx[count],
									   yaw,
									   pitch,
									   roll);
					}
				}
				break;
			case(CMD_ROTYAW):
				obj1=(long)renderdata[pos++];
				anginc=renderdata[pos++];
				if(draw)
					lpobj[obj1].yw+=anginc;
				break;
			case(CMD_ROTPITCH):
				obj1=(long)renderdata[pos++];
				anginc=renderdata[pos++];
				if(draw)
					lpobj[obj1].pt+=anginc;
				break;
			case(CMD_ROTROLL):
				obj1=(long)renderdata[pos++];
				anginc=renderdata[pos++];
				if(draw)
					lpobj[obj1].rl+=anginc;
				break;
			case(CMD_SETYAW):
				obj1=(long)renderdata[pos++];
				anginc=renderdata[pos++];
				if(draw)
					lpobj[obj1].yw=anginc;
				break;
			case(CMD_SETPITCH):
				obj1=(long)renderdata[pos++];
				anginc=renderdata[pos++];
				if(draw)
					lpobj[obj1].pt=anginc;
				break;
			case(CMD_SETROLL):
				obj1=(long)renderdata[pos++];
				anginc=renderdata[pos++];
				if(draw)
					lpobj[obj1].rl=anginc;
				break;
			case(CMD_DRAWPOLY):
				obj1=(long)renderdata[pos++];
				vtx1=(long)renderdata[pos++];
				vtx1=(vtx1<<8)+(long)renderdata[pos++];
				obj2=(long)renderdata[pos++];
				vtx2=(long)renderdata[pos++];
				vtx2=(vtx2<<8)+(long)renderdata[pos++];
				obj3=(long)renderdata[pos++];
				vtx3=(long)renderdata[pos++];
				vtx3=(vtx3<<8)+(long)renderdata[pos++];
				clr=renderdata[pos++];
				if(draw)
					DrawPolygon(&lpobj[obj1].lpvtx[vtx1],
								&lpobj[obj2].lpvtx[vtx2],
								&lpobj[obj3].lpvtx[vtx3],
								clr);
				break;
			case(CMD_PERSPECTIVE):
				if(draw)
					for(count=0;count<numobj;count++)
						for(count2=0;count2<lpobj[count].numvtx;count2++)
						{
							if(lpobj[count].lpvtx[count2].tz==0)
								lpobj[count].lpvtx[count2].tz=1;
							lpobj[count].lpvtx[count2].tx=FPDIV(FPMUL(FPMPLR*SCRWD,lpobj[count].lpvtx[count2].tx),lpobj[count].lpvtx[count2].tz);
							lpobj[count].lpvtx[count2].ty=FPDIV(FPMUL(FPMPLR*SCRWD,lpobj[count].lpvtx[count2].ty),lpobj[count].lpvtx[count2].tz);
						}
				break;
			case(CMD_BEGINKEY):
				keyval=renderdata[pos++];
				if(DirectInput->IsKeyDown(keyval))
					draw=TRUE;
				else
					draw=FALSE;
				break;
			case(CMD_ENDKEY):
				draw=TRUE;
				break;
			case(CMD_POLYRGB):
				obj1=(long)renderdata[pos++];
				vtx1=(long)renderdata[pos++];
				vtx1=(vtx1<<8)+(long)renderdata[pos++];
				obj2=(long)renderdata[pos++];
				vtx2=(long)renderdata[pos++];
				vtx2=(vtx2<<8)+(long)renderdata[pos++];
				obj3=(long)renderdata[pos++];
				vtx3=(long)renderdata[pos++];
				vtx3=(vtx3<<8)+(long)renderdata[pos++];
				r=renderdata[pos++];
				g=renderdata[pos++];
				b=renderdata[pos++];
				if(draw)
				{
					DirectDraw->SetDirectDrawPaletteEntry(254,r,g,b);
					DrawPolygon(&lpobj[obj1].lpvtx[vtx1],
								&lpobj[obj2].lpvtx[vtx2],
								&lpobj[obj3].lpvtx[vtx3],
								254);
				}
				break;
			case(CMD_ENDOFFILE):
				done=1;
				break;
		}
	}
	DirectDraw->UnlockSecondaryDirectDrawSurface();
	return(TRUE);
}

// END //

