#include "main.h"
#include "ui_buttom.h"
#include "util.h"
#include "palcfg.h"

UIBUTTOM gUI_Buttom[MAX_BUTTOM];
UIGAMEPAD gUI_GamePad;
static char internal_buffer[PAL_MAX_GLOBAL_BUFFERS + 1][PAL_GLOBAL_BUFFER_SIZE];
#define INTERNAL_BUFFER_SIZE_ARGS internal_buffer[PAL_MAX_GLOBAL_BUFFERS], PAL_GLOBAL_BUFFER_SIZE

extern SDL_Surface       *gpScreenReal;
extern SDL_Renderer      *gpRenderer;
extern SDL_Rect           gViewRect;
extern DWORD g_dwScreenWidth;
extern DWORD g_dwScreenHeight;
extern BOOL mouseLkey, mouseRkey, mouseMkey;

static SDL_Renderer *nowRenderer = NULL;

extern float gpScreenW_Magnification;
extern float gpScreenH_Magnification;
extern float gpScreenW_MagnificationOriginal;
extern float gpScreenH_MagnificationOriginal;

void AddButtom(int index,LPCSTR file1, LPCSTR file2, LPCSTR file3)
{
	const char *tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, gConfig.pszGamePath, "pic");
	if (tmp == NULL)
		return;
	char *imgpath = malloc(strlen(tmp) + 1);
	strcpy(imgpath, tmp);

	if (file1)
	{
		tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, file1);
		if (tmp)
		{
			SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
			if (loadedSurface != NULL)
			{
				gUI_Buttom[index].img = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
				if (gUI_Buttom[index].img == NULL)
					SDL_FreeSurface(loadedSurface);
				else
				{
					gUI_Buttom[index].range.w = gUI_Buttom[index].img->w;
					gUI_Buttom[index].range.h = gUI_Buttom[index].img->h;
					gUI_Buttom[index].enable = TRUE;
				}
			}
		}
	}
	if (file2)
	{
		tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, file2);
		if (tmp)
		{
			SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
			if (loadedSurface != NULL)
			{
				gUI_Buttom[index].img_selected = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
				if (gUI_Buttom[index].img_selected == NULL)
					SDL_FreeSurface(loadedSurface);
			}
		}
	}
	if (file3)
	{
		tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, file3);
		if (tmp)
		{
			SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
			if (loadedSurface != NULL)
			{
				gUI_Buttom[index].img_disable = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
				if (gUI_Buttom[index].img_disable == NULL)
					SDL_FreeSurface(loadedSurface);
			}
		}
	}
	free(imgpath);
}

void DelButtom(int index)
{
	if (index == buttomBackup)
	{
		memset(&gUI_Buttom[index], 0, sizeof(UIBUTTOM));
		return;
	}
	if (gUI_Buttom[index].img)
	{
		SDL_FreeSurface(gUI_Buttom[index].img);
		gUI_Buttom[index].img = NULL;
	}
	if (gUI_Buttom[index].img_selected)
	{
		SDL_FreeSurface(gUI_Buttom[index].img_selected);
		gUI_Buttom[index].img_selected = NULL;
	}
	if (gUI_Buttom[index].img_disable)
	{
		SDL_FreeSurface(gUI_Buttom[index].img_disable);
		gUI_Buttom[index].img_disable = NULL;
	}
	if (gUI_Buttom[index].tex)
	{
		SDL_DestroyTexture(gUI_Buttom[index].tex);
		gUI_Buttom[index].tex = NULL;
	}
	gUI_Buttom[index].visable = FALSE;
}

void SetButtom(int index, int x, int y, int w, int h)
{
	int _x = (int)((float)x * gpScreenW_Magnification) + gViewRect.x;
	int _y = (int)((float)y * gpScreenH_Magnification) + gViewRect.y;
	int _w = (int)((float)w * gpScreenW_Magnification);
	int _h = (int)((float)h * gpScreenH_Magnification);

	if (_x + _w > (int)g_dwScreenWidth)
	{
		_x = g_dwScreenWidth - _w;
	}
	if (_y + _h > (int)g_dwScreenHeight)
	{
		_y = (int)g_dwScreenHeight - _h;
	}

	if (_x < 0)
	{
		_x = 0;
	}
	if (_y < 0)
	{
		_y = 0;
	}
    
	if (index >= 0)
	{
		gUI_Buttom[index].range.x = _x;
		gUI_Buttom[index].range.y = _y;
		gUI_Buttom[index].range.w = _w;
		gUI_Buttom[index].range.h = _h;
		gUI_Buttom[index].Alpha = 255;
	}
	else
	{
		gUI_GamePad.range.x = _x;
		gUI_GamePad.range.y = _y;
		gUI_GamePad.range.w = _w;
		gUI_GamePad.range.h = _h;
		gUI_GamePad.Alpha = 255;
	}
    
}
void CheckRender()
{
	if (nowRenderer != gpRenderer)
	{
		for (int i = 0; i < MAX_BUTTOM; i++)
		{
			gUI_Buttom[i].tex_status = 0;
			if (gUI_Buttom[i].tex)
			{
				SDL_DestroyTexture(gUI_Buttom[i].tex);
				gUI_Buttom[i].tex = NULL;
			}
		}
		nowRenderer = gpRenderer;
	}
}
void MakeTexture(int index)
{
	int status = 0;
	if (gUI_Buttom[index].visable == FALSE) return;
	if (gUI_Buttom[index].img == NULL) return;

	if (gUI_Buttom[index].tex == NULL)
	{
		gUI_Buttom[index].selected = FALSE;
		gUI_Buttom[index].tex_status = 0;
	}
	if (gUI_Buttom[index].img)
		status |= 1;
	if (gUI_Buttom[index].enable == FALSE  && gUI_Buttom[index].img_disable)
		status |= 2;
	if (gUI_Buttom[index].selected == TRUE && gUI_Buttom[index].img_selected)
		status |= 4;
	
	if (gUI_Buttom[index].tex_status != status)
	{
		if (gUI_Buttom[index].tex)
			SDL_DestroyTexture(gUI_Buttom[index].tex);
		if ((status & 2) != 0)
		{
			gUI_Buttom[index].tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_Buttom[index].img_disable);
		}
		else if ((status & 4) != 0)
		{
			gUI_Buttom[index].tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_Buttom[index].img_selected);
		}
		else
		{
			gUI_Buttom[index].tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_Buttom[index].img);
		}
		gUI_Buttom[index].tex_status = status;
	}
}

void BackupBACK()
{
	if (gUI_Buttom[buttomBACK].tex)
		SDL_DestroyTexture(gUI_Buttom[buttomBACK].tex);
	gUI_Buttom[buttomBACK].tex = NULL;
	gUI_Buttom[buttomBACK].tex_status = -1;
	memcpy(&gUI_Buttom[buttomBackup], &gUI_Buttom[buttomBACK], sizeof(UIBUTTOM));
}
void Switch_BACKtoClose()
{
	BOOL visable = gUI_Buttom[buttomBACK].visable;
	if (gUI_Buttom[buttomClose].tex)
		SDL_DestroyTexture(gUI_Buttom[buttomClose].tex);
	gUI_Buttom[buttomClose].tex = NULL;
	gUI_Buttom[buttomClose].tex_status = -1;
	if (gUI_Buttom[buttomBACK].tex)
		SDL_DestroyTexture(gUI_Buttom[buttomBACK].tex);
	gUI_Buttom[buttomBACK].tex = NULL;
	gUI_Buttom[buttomBACK].tex_status = -1;
	memcpy(&gUI_Buttom[buttomBACK], &gUI_Buttom[buttomClose], sizeof(UIBUTTOM));
	gUI_Buttom[buttomBACK].visable = visable;
}
void RestoreBACK()
{
	BOOL visable = gUI_Buttom[buttomBACK].visable;

	if (gUI_Buttom[buttomBACK].tex)
		SDL_DestroyTexture(gUI_Buttom[buttomBACK].tex);
	gUI_Buttom[buttomBACK].tex = NULL;
	gUI_Buttom[buttomBACK].tex_status = -1;

	memcpy(&gUI_Buttom[buttomBACK], &gUI_Buttom[buttomBackup], sizeof(UIBUTTOM));

	gUI_Buttom[buttomBACK].visable = visable;
}

void SetGamePadTexture()
{
	if (gUI_Buttom[buttomGP_SW].visable)
	{
		if (gpGlobals->dwUI_Game != 2)
		{
			gUI_Buttom[buttomGP_SW].visable = FALSE;
			gUI_Buttom[buttomGP_A].visable = FALSE;
		}
		else
		{
			if (gUI_GamePad.visable)
			{
				gUI_Buttom[buttomGP_A].visable = TRUE;
				PutGamePadTexture();
				if (gUI_GamePad.tex)
					SDL_RenderCopy(gpRenderer, gUI_GamePad.tex, NULL, &gUI_GamePad.range);
			}
			else
				gUI_Buttom[buttomGP_A].visable = FALSE;
		}
	}
}
void AddGamePad()
{
	BOOL is4_3 = FALSE;
	if ((float)gConfig.dwScreenHeight / (float)gConfig.dwScreenWidth == 0.75f)
		is4_3 = TRUE;
	AddButtom(buttomGP_SW, "dpad_sw.bmp", "dpad_sw_click.bmp", NULL);
	if (is4_3)
	{
		SetButtom(buttomGP_SW, 0, 0, 30, 25);
	}
	else
		SetButtom(buttomGP_SW, -34, 0, 30, 25);
	
	AddButtom(buttomGP_A, "dpad_A.bmp", "dpad_A_click.bmp", NULL);
	if (is4_3)
	{
		SetButtom(buttomGP_A, 275, 140, 45, 37);
	}
	else
		SetButtom(buttomGP_A, 320, 80, 48, 39);

	const char *tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, gConfig.pszGamePath, "pic");
	if (tmp == NULL)
		return;
	char *imgpath = malloc(strlen(tmp) + 1);
	strcpy(imgpath, tmp);

	tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, "dpad.bmp");
	if (tmp)
	{
		SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
		if (loadedSurface != NULL)
		{
			gUI_GamePad.img = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
			if (gUI_GamePad.img == NULL)
				SDL_FreeSurface(loadedSurface);
			else
			{
				gUI_GamePad.range.w = gUI_GamePad.img->w;
				gUI_GamePad.range.h = gUI_GamePad.img->h;
				gUI_GamePad.enable = TRUE;
				gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img);
			}
		}
	}
	tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, "dpad_dl.bmp");
	if (tmp)
	{
		SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
		if (loadedSurface != NULL)
		{
			gUI_GamePad.img_dl = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
			if (gUI_GamePad.img_dl == NULL)
				SDL_FreeSurface(loadedSurface);
		}
	}
	tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, "dpad_dr.bmp");
	if (tmp)
	{
		SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
		if (loadedSurface != NULL)
		{
			gUI_GamePad.img_dr = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
			if (gUI_GamePad.img_dr == NULL)
				SDL_FreeSurface(loadedSurface);
		}
	}
	tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, "dpad_ul.bmp");
	if (tmp)
	{
		SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
		if (loadedSurface != NULL)
		{
			gUI_GamePad.img_ul = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
			if (gUI_GamePad.img_ul == NULL)
				SDL_FreeSurface(loadedSurface);
		}
	}
	tmp = UTIL_GetFullPathName(INTERNAL_BUFFER_SIZE_ARGS, imgpath, "dpad_ur.bmp");
	if (tmp)
	{
		SDL_Surface* loadedSurface = SDL_LoadBMP(tmp);
		if (loadedSurface != NULL)
		{
			gUI_GamePad.img_ur = SDL_ConvertSurface(loadedSurface, gpScreenReal->format, 0);
			if (gUI_GamePad.img_ur == NULL)
				SDL_FreeSurface(loadedSurface);
		}
	}
	gUI_GamePad.visable = FALSE;

	if (is4_3)
		SetButtom(-1, -80, 130, 60, 50);
	else
		SetButtom(-1, -80, 63, 90, 75);
}

void PutGamePadTexture()
{
	if (gUI_GamePad.tex)
		SDL_DestroyTexture(gUI_GamePad.tex);
	gUI_GamePad.tex = NULL;
	switch (gUI_GamePad.currentDirection)
	{
	case DPadNone:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img);
		break;
	case DPadLeft:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img_ul);
		break;
	case DPadRight:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img_ur);
		break;
	case DPadUp:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img);
		break;
	case DPadDown:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img);
		break;
	case DPadLeftDown:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img_dl);
		break;
	case DPadLeftUp:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img_ul);
		break;
	case DPadRightDown:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img_dr);
		break;
	case DPadRightUp:
		gUI_GamePad.tex = SDL_CreateTextureFromSurface(gpRenderer, gUI_GamePad.img_ur);
		break;

	}
}
BOOL CheckGamePad(int rmx, int rmy, Uint32 evtype)
{
	if (gpGlobals->dwUI_Game != 2) return FALSE;
	if (gUI_GamePad.enable == FALSE) return FALSE;
	if (gUI_GamePad.visable == FALSE) return FALSE;

	if (gUI_GamePad.currentDirection != DPadNone && evtype == SDL_MOUSEBUTTONUP)
	{
		gUI_GamePad.currentDirection = DPadNone;
		return TRUE;
	}
	if (mouseLkey == FALSE) return FALSE;
	if (rmx < gUI_GamePad.range.x) return FALSE;;
	if (rmx > gUI_GamePad.range.x + gUI_GamePad.range.w) return FALSE;;
	if (rmy < gUI_GamePad.range.y) return FALSE;;
	if (rmy > gUI_GamePad.range.y + gUI_GamePad.range.h) return FALSE;;
	

	static DPadDirection eight_way_map[] = {
		DPadLeft, DPadLeftDown, DPadLeftDown, DPadDown,
		DPadDown, DPadRightDown, DPadRightDown, DPadRight,
		DPadRight, DPadRightUp, DPadRightUp, DPadUp,
		DPadUp, DPadLeftUp, DPadLeftUp, DPadLeft
	};

	int ptX = rmx - gUI_GamePad.range.x;
	int ptY = rmy - gUI_GamePad.range.y;
	int ptCenterX = gUI_GamePad.range.w / 2;
	int ptCenterY = gUI_GamePad.range.h / 2;
	double angle = atan2(ptCenterY - ptY, ptX - ptCenterX);
	int index = (int)((angle + M_PI) / (M_PI / 8)) % 16;

	int dir = eight_way_map[index];


	switch (dir)
	{
	case DPadLeft:
		PAL_KeyDown(kKeyLeft, FALSE);
		break;
	case DPadRight:
		PAL_KeyDown(kKeyRight, FALSE);
		break;
	case DPadUp:
		PAL_KeyDown(kKeyUp, FALSE);
		break;
	case DPadDown:
		PAL_KeyDown(kKeyDown, FALSE);
		break;
	case DPadLeftDown:
		PAL_KeyDown(kKeyDown, FALSE);
		break;
	case DPadLeftUp:
		PAL_KeyDown(kKeyLeft, FALSE);
		break;
	case DPadRightDown:
		PAL_KeyDown(kKeyRight, FALSE);
		break;
	case DPadRightUp:
		PAL_KeyDown(kKeyUp, FALSE);
		break;

	}
	gUI_GamePad.currentDirection = dir;
	return TRUE;
}

void GamePadMode(BOOL active)
{
	if (active)
	{
		gUI_Buttom[buttomGP_SW].visable = TRUE;
		if (gUI_GamePad.visable)
		{
			gUI_Buttom[buttomGP_A].visable = TRUE;
		}
	}
	else
	{
		gUI_Buttom[buttomGP_SW].visable = FALSE;
		gUI_Buttom[buttomGP_A].visable = FALSE;
	}
}