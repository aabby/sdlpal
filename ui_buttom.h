
#ifndef UI_BOTTOM_H
#define UI_BOTTOM_H

#include "common.h"
#include "palcommon.h"

#define MAX_BUTTOM 20

typedef enum tagUIBUTTOM_ITEM
{
	buttomBACK = 0,
	buttomMENU,
	buttomAttack,
	buttomMagic,
	buttomRepet,
	buttomClose,
	buttomBackup,
	buttomGP_SW,
	buttomGP_A,
	buttomLOGO,
}UIBUTTOM_ITEM;

typedef struct tagUIBUTTOM
{
	BOOL visable;
	BOOL enable;
	BOOL selected;
	BOOL KeyClick;

	SDL_Surface * img;
	SDL_Surface * img_selected;
	SDL_Surface * img_disable;

	SDL_Texture * tex;
	int tex_status;

	BYTE Alpha;
	SDL_Rect range;

} UIBUTTOM;

extern UIBUTTOM gUI_Buttom[MAX_BUTTOM];

typedef enum
{
	DPadNone,
	DPadRight,
	DPadRightUp,
	DPadUp,
	DPadLeftUp,
	DPadLeft,
	DPadLeftDown,
	DPadDown,
	DPadRightDown
} DPadDirection;

typedef struct tagUIGamePad
{
	BOOL visable;
	BOOL enable;
	BOOL selected;
	BOOL KeyClick;

	SDL_Surface * img;
	SDL_Surface * img_ur;
	SDL_Surface * img_dr;
	SDL_Surface * img_ul;
	SDL_Surface * img_dl;

	SDL_Texture * tex;

	BYTE Alpha;
	SDL_Rect range;

	int currentDirection;
} UIGAMEPAD;
extern UIGAMEPAD gUI_GamePad;

void AddButtom(int index, LPCSTR file1, LPCSTR file2, LPCSTR file3);

void DelButtom(int index);

void SetButtom(int index, int x, int y, int w, int h);

void CheckRender();

void MakeTexture(int index);

void BackupBACK();
void Switch_BACKtoClose();
void RestoreBACK();

void AddGamePad();

BOOL CheckGamePad(int rmx, int rmy, Uint32 evtype);

void PutGamePadTexture();

void SetGamePadTexture();

void GamePadMode(BOOL active);

#endif
