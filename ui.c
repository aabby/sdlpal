/* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2018, SDLPAL development team.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "main.h"

LPSPRITE      gpSpriteUI = NULL;

volatile MENUMOUSECTRL g_MenuCtrl;
volatile LISTMENU g_ListMenu;

extern BOOL mouseLkey, mouseRkey, mouseMkey;

static LPBOX
PAL_CreateBoxInternal(
	const SDL_Rect *rect
)
{
	LPBOX lpBox = (LPBOX)calloc(1, sizeof(BOX));
	if (lpBox == NULL)
	{
		return NULL;
	}

	lpBox->pos = PAL_XY(rect->x, rect->y);
	lpBox->lpSavedArea = VIDEO_DuplicateSurface(gpScreen, rect);
	lpBox->wHeight = (WORD)rect->w;
	lpBox->wWidth = (WORD)rect->h;

	if (lpBox->lpSavedArea == NULL)
	{
		free(lpBox);
		return NULL;
	}

	return lpBox;
}

INT
PAL_InitUI(
	VOID
)
/*++
  Purpose:

	Initialze the UI subsystem.

  Parameters:

	None.

  Return value:

	0 = success, -1 = fail.

--*/
{
	int        iSize;

	//
	// Load the UI sprite.
	//
	iSize = PAL_MKFGetChunkSize(CHUNKNUM_SPRITEUI, gpGlobals->f.fpDATA);
	if (iSize < 0)
	{
		return -1;
	}

	gpSpriteUI = (LPSPRITE)calloc(1, iSize);
	if (gpSpriteUI == NULL)
	{
		return -1;
	}

	PAL_MKFReadChunk(gpSpriteUI, iSize, CHUNKNUM_SPRITEUI, gpGlobals->f.fpDATA);

	extern SDL_Rect           gViewRect;
	if (gpGlobals->fMOUSE == TRUE)
	{
		BOOL is4_3 = FALSE;
		float rate = (float)gConfig.dwScreenHeight / (float)gConfig.dwScreenWidth;
		if (rate == 0.75f || (gConfig.fIsIOS && gConfig.fIsIPAD))
			is4_3 = TRUE;

		memset(gUI_Buttom, 0, sizeof(UIBUTTOM)*MAX_BUTTOM);
		AddButtom(buttomBACK, "back0.bmp", "back1.bmp", NULL);
		SetButtom(buttomBACK, 324, -20, 30, 25);
		AddButtom(buttomMENU, "menu.bmp", NULL, NULL);
		SetButtom(buttomMENU, 324, -20, 30, 25);
		if (is4_3)
		{
			gUI_Buttom[buttomBACK].Alpha = 0xC0;
			gUI_Buttom[buttomMENU].Alpha = 0xA0;
		}

		AddButtom(buttomAttack, "attack0.bmp", NULL, "attack2.bmp"); //³ò§ð
		if (is4_3)
		{
			SetButtom(buttomAttack, 5, 4, 30, 25);
			gUI_Buttom[buttomAttack].Alpha = 0xC0;
		}
		else
			SetButtom(buttomAttack, -34, 4, 30, 25);
		AddButtom(buttomMagic, "magic0.bmp", NULL, "magic2.bmp"); //±j§ð
		if (is4_3)
		{
			SetButtom(buttomMagic, 45, 4, 30, 25);
			gUI_Buttom[buttomMagic].Alpha = 0xC0;
		}
		else
			SetButtom(buttomMagic, -34, 4 + (4 + 25), 30, 25);
		AddButtom(buttomRepet, "repet0.bmp", NULL, "repet2.bmp"); //­«½Æ
		if (is4_3)
		{
			SetButtom(buttomRepet, 85, 4, 30, 25);
			gUI_Buttom[buttomRepet].Alpha = 0xC0;
		}
		else
			SetButtom(buttomRepet, -34, 4 + ((4 + 25) * 2), 30, 25);
		AddButtom(buttomClose, "close0.bmp", "close1.bmp", NULL);
		SetButtom(buttomClose, 324, -20, 30, 25);

	}
	AddButtom(buttomLOGO, "LOGO.bmp", NULL, NULL); //copyright logo
	SetButtom(buttomLOGO, 0, 170, 320, 30);
#ifndef PAL_STEAM
	if (gpGlobals->wLanguage == 0)
	{
		AddButtom(buttomLOGO2, "gamerating.org.tw.15.bmp", NULL, NULL);
		SetButtom(buttomLOGO2, 289, 172, 26, 21);
	}
#endif
#ifdef PAL_HAS_GAMEPAD
   AddGamePad();
#endif
   return 0;
}

VOID
PAL_FreeUI(
   VOID
)
/*++
  Purpose:

    Shutdown the UI subsystem.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   if (gpSpriteUI != NULL)
   {
      free(gpSpriteUI);
      gpSpriteUI = NULL;
   }

   for (int i = 0; i < MAX_BUTTOM; i++)
   {
	   DelButtom(i);
   }
   memset(gUI_Buttom, 0, sizeof(UIBUTTOM)*MAX_BUTTOM);
}

LPBOX
PAL_CreateBox(
   PAL_POS        pos,
   INT            nRows,
   INT            nColumns,
   INT            iStyle,
   BOOL           fSaveScreen
)
{
    return PAL_CreateBoxWithShadow( pos, nRows, nColumns, iStyle, fSaveScreen, 6 );
}

LPBOX
PAL_CreateBoxWithShadow(
   PAL_POS        pos,
   INT            nRows,
   INT            nColumns,
   INT            iStyle,
   BOOL           fSaveScreen,
   INT            nShadowOffset
)
/*++
  Purpose:

    Create a box on the screen.

  Parameters:

    [IN]  pos - position of the box.

    [IN]  nRows - number of rows of the box.

    [IN]  nColumns - number of columns of the box.

    [IN]  iStyle - style of the box (0 or 1).

    [IN]  fSaveScreen - whether save the used screen area or not.

  Return value:

    Pointer to a BOX structure. NULL if failed.
    If fSaveScreen is false, then always returns NULL.

--*/
{
   int              i, j, x, m, n;
   LPCBITMAPRLE     rglpBorderBitmap[3][3];
   LPBOX            lpBox = NULL;
   SDL_Rect         rect;

   //
   // Get the bitmaps
   //
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         rglpBorderBitmap[i][j] = PAL_SpriteGetFrame(gpSpriteUI, i * 3 + j + iStyle * 9);
      }
   }

   rect.x = PAL_X(pos);
   rect.y = PAL_Y(pos);
   rect.w = 0;
   rect.h = 0;

   //
   // Get the total width and total height of the box
   //
   for (i = 0; i < 3; i++)
   {
      if (i == 1)
      {
         rect.w += PAL_RLEGetWidth(rglpBorderBitmap[0][i]) * nColumns;
         rect.h += PAL_RLEGetHeight(rglpBorderBitmap[i][0]) * nRows;
      }
      else
      {
         rect.w += PAL_RLEGetWidth(rglpBorderBitmap[0][i]);
         rect.h += PAL_RLEGetHeight(rglpBorderBitmap[i][0]);
      }
   }

   // Include shadow
   rect.w += nShadowOffset;
   rect.h += nShadowOffset;

   if (fSaveScreen)
   {
      //
      // Save the used part of the screen
      //
      lpBox = PAL_CreateBoxInternal(&rect);
   }

   //
   // Border takes 2 additional rows and columns...
   //
   nRows += 2;
   nColumns += 2;

   //
   // Draw the box
   //
   for (i = 0; i < nRows; i++)
   {
      x = rect.x;
      m = (i == 0) ? 0 : ((i == nRows - 1) ? 2 : 1);

      for (j = 0; j < nColumns; j++)
      {
         n = (j == 0) ? 0 : ((j == nColumns - 1) ? 2 : 1);
         PAL_RLEBlitToSurfaceWithShadow(rglpBorderBitmap[m][n], gpScreen, PAL_XY(x+nShadowOffset, rect.y+nShadowOffset),TRUE);
         PAL_RLEBlitToSurface(rglpBorderBitmap[m][n], gpScreen, PAL_XY(x, rect.y));
         x += PAL_RLEGetWidth(rglpBorderBitmap[m][n]);
      }

      rect.y += PAL_RLEGetHeight(rglpBorderBitmap[m][0]);
   }

   return lpBox;
}

LPBOX
PAL_CreateSingleLineBox(
   PAL_POS        pos,
   INT            nLen,
   BOOL           fSaveScreen
)
{
    return PAL_CreateSingleLineBoxWithShadow(pos, nLen, fSaveScreen, 6);
}

LPBOX
PAL_CreateSingleLineBoxWithShadow(
   PAL_POS        pos,
   INT            nLen,
   BOOL           fSaveScreen,
   INT            nShadowOffset
)
/*++
  Purpose:

    Create a single-line box on the screen.

  Parameters:

    [IN]  pos - position of the box.

    [IN]  nLen - length of the box.

    [IN]  fSaveScreen - whether save the used screen area or not.

  Return value:

    Pointer to a BOX structure. NULL if failed.
    If fSaveScreen is false, then always returns NULL.

--*/
{
   static const int      iNumLeftSprite   = 44;
   static const int      iNumMidSprite    = 45;
   static const int      iNumRightSprite  = 46;

   LPCBITMAPRLE          lpBitmapLeft;
   LPCBITMAPRLE          lpBitmapMid;
   LPCBITMAPRLE          lpBitmapRight;
   SDL_Rect              rect;
   LPBOX                 lpBox = NULL;
   int                   i;
   int                   xSaved;

   //
   // Get the bitmaps
   //
   lpBitmapLeft = PAL_SpriteGetFrame(gpSpriteUI, iNumLeftSprite);
   lpBitmapMid = PAL_SpriteGetFrame(gpSpriteUI, iNumMidSprite);
   lpBitmapRight = PAL_SpriteGetFrame(gpSpriteUI, iNumRightSprite);

   rect.x = PAL_X(pos);
   rect.y = PAL_Y(pos);

   //
   // Get the total width and total height of the box
   //
   rect.w = PAL_RLEGetWidth(lpBitmapLeft) + PAL_RLEGetWidth(lpBitmapRight);
   rect.w += PAL_RLEGetWidth(lpBitmapMid) * nLen;
   rect.h = PAL_RLEGetHeight(lpBitmapLeft);

   // Include shadow
   rect.w += nShadowOffset;
   rect.h += nShadowOffset;

   if (fSaveScreen)
   {
      //
      // Save the used part of the screen
      //
      lpBox = PAL_CreateBoxInternal(&rect);
   }
   xSaved = rect.x;

   //
   // Draw the shadow
   //
   PAL_RLEBlitToSurfaceWithShadow(lpBitmapLeft, gpScreen, PAL_XY(rect.x+nShadowOffset, rect.y+nShadowOffset), TRUE);

   rect.x += PAL_RLEGetWidth(lpBitmapLeft);

   for (i = 0; i < nLen; i++)
   {
      PAL_RLEBlitToSurfaceWithShadow(lpBitmapMid, gpScreen, PAL_XY(rect.x+nShadowOffset, rect.y+nShadowOffset), TRUE);
      rect.x += PAL_RLEGetWidth(lpBitmapMid);
   }

   PAL_RLEBlitToSurfaceWithShadow(lpBitmapRight, gpScreen, PAL_XY(rect.x+nShadowOffset, rect.y+nShadowOffset), TRUE);

   rect.x = xSaved;
   //
   // Draw the box
   //
   PAL_RLEBlitToSurface(lpBitmapLeft, gpScreen, pos);

   rect.x += PAL_RLEGetWidth(lpBitmapLeft);

   for (i = 0; i < nLen; i++)
   {
      PAL_RLEBlitToSurface(lpBitmapMid, gpScreen, PAL_XY(rect.x, rect.y));
      rect.x += PAL_RLEGetWidth(lpBitmapMid);
   }

   PAL_RLEBlitToSurface(lpBitmapRight, gpScreen, PAL_XY(rect.x, rect.y));

   return lpBox;
}

VOID
PAL_DeleteBox(
   LPBOX          lpBox
)
/*++
  Purpose:

    Delete a box and restore the saved part of the screen.

  Parameters:

    [IN]  lpBox - pointer to the BOX struct.

  Return value:

    None.

--*/
{
   SDL_Rect        rect;

   //
   // Check for NULL pointer.
   //
   if (lpBox == NULL)
   {
      return;
   }

   //
   // Restore the saved screen part
   //
   rect.x = PAL_X(lpBox->pos);
   rect.y = PAL_Y(lpBox->pos);
   rect.w = lpBox->wWidth;
   rect.h = lpBox->wHeight;

   VIDEO_CopySurface(lpBox->lpSavedArea, NULL, gpScreen, &rect);

   //
   // Free the memory used by the box
   //
   VIDEO_FreeSurface(lpBox->lpSavedArea);
   free(lpBox);
}

WORD
PAL_ReadMenu(
   LPITEMCHANGED_CALLBACK    lpfnMenuItemChanged,
   LPCMENUITEM               rgMenuItem,
   INT                       nMenuItem,
   WORD                      wDefaultItem,
   BYTE                      bLabelColor
)
/*++
  Purpose:

    Execute a menu.

  Parameters:

    [IN]  lpfnMenuItemChanged - Callback function which is called when user
                                changed the current menu item.

    [IN]  rgMenuItem - Array of the menu items.

    [IN]  nMenuItem - Number of menu items.

    [IN]  wDefaultItem - default item index.

    [IN]  bLabelColor - color of the labels.

  Return value:

    Return value of the selected menu item. MENUITEM_VALUE_CANCELLED if cancelled.

--*/
{
   int               i;

   WORD              wCurrentItem    = (wDefaultItem < nMenuItem) ? wDefaultItem : 0;

   if (gpGlobals->fMOUSE == TRUE)
		Setup_MenuCtrl(rgMenuItem, nMenuItem, wCurrentItem);
   //
   // Draw all the menu texts.
   //
   for (i = 0; i < nMenuItem; i++)
   {
      BYTE bColor = bLabelColor;

      if (!rgMenuItem[i].fEnabled)
      {
         if (i == wCurrentItem)
         {
            bColor = MENUITEM_COLOR_SELECTED_INACTIVE;
         }
         else
         {
            bColor = MENUITEM_COLOR_INACTIVE;
         }
      }

      PAL_DrawText(PAL_GetWord(rgMenuItem[i].wNumWord), rgMenuItem[i].pos, bColor, TRUE, TRUE, FALSE);

   }

   if (lpfnMenuItemChanged != NULL)
   {
      (*lpfnMenuItemChanged)(rgMenuItem[wDefaultItem].wValue);
   }

   while (TRUE)
   {
      PAL_ClearKeyState();
	  
      //
      // Redraw the selected item if needed.
      //
      if (rgMenuItem[wCurrentItem].fEnabled)
      {
         PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
            rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED, FALSE, TRUE, FALSE);
      }

      PAL_ProcessEvent();

	  if (gUI_Buttom[buttomLOGO].visable == TRUE && gUI_Buttom[buttomLOGO].KeyClick == TRUE)
	  {
		  gUI_Buttom[buttomLOGO].KeyClick = FALSE;
		  gUI_Buttom[buttomLOGO].visable = FALSE;
		  PAL_AdditionalCredits();
		  return MENUITEM_VALUE_RESET;
	  }
	  if (g_InputState.dwKeyPress == 0)
	  {
		  for (i = 0; i < nMenuItem; i++)
		  {
			  if (rgMenuItem[i].fEnabled)
			  {
				  if (g_MenuCtrl.Click && g_MenuCtrl.Select == i)
				  {
					  wCurrentItem = i;
					  for (int j = 0; j < nMenuItem; j++)
					  {
						  BYTE bColor = bLabelColor;
						  if (wCurrentItem == j)
							  bColor = MENUITEM_COLOR_SELECTED_INACTIVE;
						  PAL_DrawText(PAL_GetWord(rgMenuItem[j].wNumWord), rgMenuItem[j].pos, bColor, TRUE, TRUE, FALSE);
						  
					  }
					  if (lpfnMenuItemChanged != NULL)
					  {
						  (*lpfnMenuItemChanged)(rgMenuItem[wCurrentItem].wValue);
					  }
					  if (g_MenuCtrl.Click == 2)
					  {
						  g_MenuCtrl.Click = 0;
						  if (g_MenuCtrl.LastSelect == i)
						  {
							  g_InputState.dwKeyPress |= kKeySearch;
						  }
						  else
							  g_MenuCtrl.LastSelect = i;
					  }
					  break;
				  }
			  }
		  }
	  }

      if (g_InputState.dwKeyPress & (kKeyDown | kKeyRight))
      {
         //
         // User pressed the down or right arrow key
         //
         if (rgMenuItem[wCurrentItem].fEnabled)
         {
            //
            // Dehighlight the unselected item.
            //
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, bLabelColor, FALSE, TRUE, FALSE);
         }
         else
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_INACTIVE, FALSE, TRUE, FALSE);
         }

         wCurrentItem++;

         if (wCurrentItem >= nMenuItem)
         {
            wCurrentItem = 0;
         }

         //
         // Highlight the selected item.
         //
         if (rgMenuItem[wCurrentItem].fEnabled)
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED, FALSE, TRUE, FALSE);
         }
         else
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED_INACTIVE, FALSE, TRUE, FALSE);
         }

         if (lpfnMenuItemChanged != NULL)
         {
            (*lpfnMenuItemChanged)(rgMenuItem[wCurrentItem].wValue);
         }
		 g_InputState.dwKeyPress = 0;
      }
      else if (g_InputState.dwKeyPress & (kKeyUp | kKeyLeft))
      {
         //
         // User pressed the up or left arrow key
         //
         if (rgMenuItem[wCurrentItem].fEnabled)
         {
            //
            // Dehighlight the unselected item.
            //
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, bLabelColor, FALSE, TRUE, FALSE);
         }
         else
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_INACTIVE, FALSE, TRUE, FALSE);
         }

         if (wCurrentItem > 0)
         {
            wCurrentItem--;
         }
         else
         {
            wCurrentItem = nMenuItem - 1;
         }

         //
         // Highlight the selected item.
         //
         if (rgMenuItem[wCurrentItem].fEnabled)
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED, FALSE, TRUE, FALSE);
         }
         else
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED_INACTIVE, FALSE, TRUE, FALSE);
         }

         if (lpfnMenuItemChanged != NULL)
         {
            (*lpfnMenuItemChanged)(rgMenuItem[wCurrentItem].wValue);
         }
		 g_InputState.dwKeyPress = 0;
      }
      else if (g_InputState.dwKeyPress & kKeyMenu)
      {
         //
         // User cancelled
         //
         if (rgMenuItem[wCurrentItem].fEnabled)
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, bLabelColor, FALSE, TRUE, FALSE);
         }
         else
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_INACTIVE, FALSE, TRUE, FALSE);
         }
		 g_InputState.dwKeyPress = 0;
         break;
      }
      else if (g_InputState.dwKeyPress & kKeySearch)
      {
         //
         // User pressed Enter
         //
         if (rgMenuItem[wCurrentItem].fEnabled)
         {
            PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
               rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_CONFIRMED, FALSE, TRUE, FALSE);
			Setup_MenuCtrl(NULL, 0, 0);
			g_InputState.dwKeyPress = 0;
            return rgMenuItem[wCurrentItem].wValue;
         }
      }

      //
      // Use delay function to avoid high CPU usage.
      //
      SDL_Delay(50);
   }
   Setup_MenuCtrl(NULL, 0, 0);
   return MENUITEM_VALUE_CANCELLED;
}

VOID
PAL_DrawNumber(
   UINT            iNum,
   UINT            nLength,
   PAL_POS         pos,
   NUMCOLOR        color,
   NUMALIGN        align
)
/*++
  Purpose:

    Draw the specified number with the bitmaps in the UI sprite.

  Parameters:

    [IN]  iNum - the number to be drawn.

    [IN]  nLength - max. length of the number.

    [IN]  pos - position on the screen.

    [IN]  color - color of the number (yellow or blue).

    [IN]  align - align mode of the number.

  Return value:

    None.

--*/
{
   UINT          nActualLength, i;
   int           x, y;
   LPCBITMAPRLE  rglpBitmap[10];

   //
   // Get the bitmaps. Blue starts from 29, Cyan from 56, Yellow from 19.
   //
   x = (color == kNumColorBlue) ? 29 : ((color == kNumColorCyan) ? 56 : 19);

   for (i = 0; i < 10; i++)
   {
      rglpBitmap[i] = PAL_SpriteGetFrame(gpSpriteUI, (UINT)x + i);
   }

   i = iNum;
   nActualLength = 0;

   //
   // Calculate the actual length of the number.
   //
   while (i > 0)
   {
      i /= 10;
      nActualLength++;
   }

   if (nActualLength > nLength)
   {
      nActualLength = nLength;
   }
   else if (nActualLength == 0)
   {
      nActualLength = 1;
   }

   x = PAL_X(pos) - 6;
   y = PAL_Y(pos);

   switch (align)
   {
   case kNumAlignLeft:
      x += 6 * nActualLength;
      break;

   case kNumAlignMid:
      x += 3 * (nLength + nActualLength);
      break;

   case kNumAlignRight:
      x += 6 * nLength;
      break;
   }

   //
   // Draw the number.
   //
   while (nActualLength-- > 0)
   {
      PAL_RLEBlitToSurface(rglpBitmap[iNum % 10], gpScreen, PAL_XY(x, y));
      x -= 6;
      iNum /= 10;
   }
}

/*++
	Purpose:

		Calculate the text width of the given text.

	Parameters:

		[IN]  itemText - Pointer to the text.

	Return value:

		text width.

--*/
INT
PAL_TextWidth(
   LPCWSTR lpszItemText
)

{
    size_t l = wcslen(lpszItemText), j = 0, w = 0;
    for (j = 0; j < l; j++)
    {
        w += PAL_CharWidth(lpszItemText[j]);
    }
    return (int)w;
}

INT
PAL_MenuTextMaxWidth(
   LPCMENUITEM    rgMenuItem,
   INT            nMenuItem
)
/*++
  Purpose:

    Calculate the maximal text width of all the menu items in number of full width characters.

  Parameters:

    [IN]  rgMenuItem - Pointer to the menu item array.
	[IN]  nMenuItem - Number of menu items.

  Return value:

    Maximal text width.

--*/
{
	int i, r = 0;
	for (i = 0; i < nMenuItem; i++)
	{
		LPCWSTR itemText = PAL_GetWord(rgMenuItem[i].wNumWord);
		int w = (PAL_TextWidth(itemText) + 8) >> 4;
		if (r < w)
		{
			r = w;
		}
	}
	return r;
}

INT
PAL_WordMaxWidth(
   INT            nFirstWord,
   INT            nWordNum
)
/*++
  Purpose:

    Calculate the maximal text width of a specific range of words in number of full width characters.

  Parameters:

    [IN]  nFirstWord - First index of word.
	[IN]  nWordNum - Number of words.

  Return value:

    Maximal text width.

--*/
{
	int i, r = 0;
	for (i = 0; i < nWordNum; i++)
	{
		LPCWSTR itemText = PAL_GetWord(nFirstWord + i);
		int j = 0, l = (int)wcslen(itemText), w = 0;
		for (j = 0; j < l; j++)
		{
			w += PAL_CharWidth(itemText[j]);
		}
		w = (w + 8) >> 4;
		if (r < w)
		{
			r = w;
		}
	}
	return r;
}

INT
PAL_WordWidth(
   INT            nWordIndex
)
/*++
  Purpose:

    Calculate the text width of a specific word.

  Parameters:

	[IN]  nWordNum - Index of the word.

  Return value:

    Text width.

--*/
{
	LPCWSTR itemText = PAL_GetWord(nWordIndex);
	int i, l = (int)wcslen(itemText), w = 0;
	for (i = 0; i < l; i++)
	{
		w += PAL_CharWidth(itemText[i]);
	}
	return (w + 8) >> 4;
}

LPOBJECTDESC
PAL_LoadObjectDesc(
   LPCSTR         lpszFileName
)
/*++
  Purpose:

    Load the object description strings from file.

  Parameters:

    [IN]  lpszFileName - the filename to be loaded.

  Return value:

    Pointer to loaded data, in linked list form. NULL if unable to load.

--*/
{
   FILE                      *fp;
   PAL_LARGE char             buf[512];
   char                      *p;
   LPOBJECTDESC               lpDesc = NULL, pNew = NULL;
   unsigned int               i;

   fp = UTIL_OpenFileForMode(lpszFileName, "r");

   if (fp == NULL)
   {
      return NULL;
   }

   //
   // Load the description data
   //
   while (fgets(buf, 512, fp) != NULL)
   {
      int wlen,strip_count=2;
      p = strchr(buf, '=');
      if (p == NULL)
      {
         continue;
      }

      *p++ = '\0';
      while(strip_count--){
         if(p[strlen(p)-1]=='\r') p[strlen(p)-1]='\0';
         if(p[strlen(p)-1]=='\n') p[strlen(p)-1]='\0';
      }
	  wlen = PAL_MultiByteToWideChar(p, -1, NULL, 0);

      pNew = UTIL_calloc(1, sizeof(OBJECTDESC));

      sscanf(buf, "%x", &i);
      pNew->wObjectID = i;
	  pNew->lpDesc = (LPWSTR)UTIL_malloc(wlen * sizeof(WCHAR));
	  PAL_MultiByteToWideChar(p, -1, pNew->lpDesc, wlen);

      pNew->next = lpDesc;
      lpDesc = pNew;
   }

   fclose(fp);
   return lpDesc;
}

VOID
PAL_FreeObjectDesc(
   LPOBJECTDESC   lpObjectDesc
)
/*++
  Purpose:

    Free the object description data.

  Parameters:

    [IN]  lpObjectDesc - the description data to be freed.

  Return value:

    None.

--*/
{
   LPOBJECTDESC    p;

   while (lpObjectDesc != NULL)
   {
      p = lpObjectDesc->next;
      free(lpObjectDesc->lpDesc);
      free(lpObjectDesc);
      lpObjectDesc = p;
   }
}

LPCWSTR
PAL_GetObjectDesc(
   LPOBJECTDESC   lpObjectDesc,
   WORD           wObjectID
)
/*++
  Purpose:

    Get the object description string from the linked list.

  Parameters:

    [IN]  lpObjectDesc - the description data linked list.

    [IN]  wObjectID - the object ID.

  Return value:

    The description string. NULL if the specified object ID
    is not found.

--*/
{
   while (lpObjectDesc != NULL)
   {
      if (lpObjectDesc->wObjectID == wObjectID)
      {
         return lpObjectDesc->lpDesc;
      }

      lpObjectDesc = lpObjectDesc->next;
   }

   return NULL;
}

void Setup_MenuCtrl(MENUITEM * rgMenuItem, int count, int select)
{
	if (rgMenuItem == NULL || count <= 0)
	{
		g_MenuCtrl.MenuItem = NULL;
	}
	else if (rgMenuItem == g_MenuCtrl.MenuItem)
	{
		return;
	}
	else
	{
		g_MenuCtrl.MenuItem = rgMenuItem;
		g_MenuCtrl.Click = 0;
		g_MenuCtrl.Count = count;
		g_MenuCtrl.LastSelect = select;
		g_MenuCtrl.Select = select;
	}
}

void ListMenuOpen(int *selectitem, int *itemCount, int x, int y, int iItemsPerLine, int iLinesPerPage, int iItemTextWidth)
{
	memset(g_ListMenu.ListScreen->pixels, 1, 320 * 240);
	gpGlobals->dwUI_Game |= 0x200; //Scroll menu
	g_ListMenu.iSelect = selectitem;
	g_ListMenu.iCount = itemCount;
	g_ListMenu.wClick = 0;
	g_ListMenu.iItemsPerLine = iItemsPerLine;
	g_ListMenu.iLinesPerPage = iLinesPerPage;
	g_ListMenu.iItemTextWidth = iItemTextWidth;

	g_ListMenu.srcrng.x = 0;
	g_ListMenu.srcrng.y = 18;
	g_ListMenu.srcrng.w = g_ListMenu.iItemTextWidth * g_ListMenu.iItemsPerLine;
	g_ListMenu.srcrng.h = g_ListMenu.iLinesPerPage * 18;
	g_ListMenu.dstrng.x = x;
	g_ListMenu.dstrng.y = y;
	g_ListMenu.dstrng.w = g_ListMenu.srcrng.w;
	g_ListMenu.dstrng.h = g_ListMenu.srcrng.h;

	ListMenu_GoTpFocus();
}

void ListMenuClose()
{
	gpGlobals->dwUI_Game &= (0xffffffff - 0x200);
	g_ListMenu.fDoUpdate = FALSE;
	g_ListMenu.wClick = 0;
	g_ListMenu.iVectorY = 0;
}

void ListMenuUpdate()
{
	int item_delta, i;
	gpGlobals->dwUI_Game |= 0x200; //Scroll menu
	const int          iLinesMax = g_ListMenu.iLinesPerPage * 18;
	int bottom = (*g_ListMenu.iCount / g_ListMenu.iItemsPerLine);
	if (*g_ListMenu.iCount % g_ListMenu.iItemsPerLine != 0)
		bottom += 1;
	bottom = bottom * 18 - iLinesMax;
	if (bottom < 0) bottom = 0;
	//
	// Process input
	//
	i = *g_ListMenu.iSelect + 1;
	if (g_InputState.dwKeyPress & kKeyUp)
	{
		item_delta = -g_ListMenu.iItemsPerLine;
		g_ListMenu.iVectorY += -18;

		int y = ((i / g_ListMenu.iItemsPerLine) * 18) + g_ListMenu.iShiftY;
		if (y < 0)
		{
			g_ListMenu.iShiftY = 0;
			g_ListMenu.iVectorY = 0;
		}

	}
	else if (g_InputState.dwKeyPress & kKeyDown)
	{
		item_delta = g_ListMenu.iItemsPerLine;
		g_ListMenu.iVectorY += 18;

		if (g_ListMenu.iShiftY < -bottom)
		{
			g_ListMenu.iShiftY = -bottom;
			g_ListMenu.iVectorY = 0;
		}
	}
	else if (g_InputState.dwKeyPress & kKeyLeft)
	{
		item_delta = -1;
		if (*g_ListMenu.iSelect % g_ListMenu.iItemsPerLine == 0)
		{
			g_ListMenu.iVectorY += -18;
			int y = ((i / g_ListMenu.iItemsPerLine) * 18) + g_ListMenu.iShiftY;
			if (y < 0)
			{
				g_ListMenu.iShiftY = 0;
				g_ListMenu.iVectorY = 0;
			}
		}
	}
	else if (g_InputState.dwKeyPress & kKeyRight)
	{
		item_delta = 1;
		if (*g_ListMenu.iSelect % g_ListMenu.iItemsPerLine == 2)
		{
			g_ListMenu.iVectorY += 18;
			if (g_ListMenu.iShiftY < -bottom)
			{
				g_ListMenu.iShiftY = -bottom;
				g_ListMenu.iVectorY = 0;
			}
		}
	}
	else if (g_InputState.dwKeyPress & kKeyPgUp)
	{
		item_delta = -(g_ListMenu.iItemsPerLine * g_ListMenu.iLinesPerPage);
		g_ListMenu.iVectorY += -(18 * g_ListMenu.iLinesPerPage);
		int y = ((i / g_ListMenu.iItemsPerLine) * 18) + g_ListMenu.iShiftY;
		if (y < 0 || *g_ListMenu.iSelect + item_delta < 0)
		{
			g_ListMenu.iShiftY = 0;
			g_ListMenu.iVectorY = 0;
		}
	}
	else if (g_InputState.dwKeyPress & kKeyPgDn)
	{
		item_delta = g_ListMenu.iItemsPerLine * g_ListMenu.iLinesPerPage;
		g_ListMenu.iVectorY += 18 * g_ListMenu.iLinesPerPage;
		if (g_ListMenu.iShiftY < -bottom || *g_ListMenu.iSelect + item_delta >= *g_ListMenu.iCount)
		{
			g_ListMenu.iShiftY = -bottom;
			g_ListMenu.iVectorY = 0;
		}
	}
	else if (g_InputState.dwKeyPress & kKeyHome)
	{
		item_delta = -*g_ListMenu.iSelect;
		g_ListMenu.iShiftY = 0;
		g_ListMenu.iVectorY = 0;
	}
	else if (g_InputState.dwKeyPress & kKeyEnd)
	{
		item_delta = *g_ListMenu.iCount - *g_ListMenu.iSelect - 1;
		g_ListMenu.iShiftY = -((*g_ListMenu.iCount / g_ListMenu.iItemsPerLine + 1) * 18 - iLinesMax);
		g_ListMenu.iVectorY = 0;
	}
	else
	{
		item_delta = 0;
	}

	//
	// Make sure the current menu item index is in bound
	//
	if (item_delta != 0)
	{
		if (*g_ListMenu.iSelect + item_delta < 0)
			*g_ListMenu.iSelect = 0;
		else if (*g_ListMenu.iSelect + item_delta >= *g_ListMenu.iCount)
			*g_ListMenu.iSelect = *g_ListMenu.iCount - 1;
		else
			*g_ListMenu.iSelect += item_delta;
		ListMenu_GoTpFocus();
	}
	if (g_ListMenu.ListScreen == NULL) return;

	g_ListMenu.fDoUpdate = TRUE;


	if (bottom <= 0)
	{
		g_ListMenu.iShiftY = 0;
		g_ListMenu.iVectorY = 0;
		return;
	}
	if (g_ListMenu.iVectorY)
	{
		if (g_ListMenu.iVectorY > 0)
		{
			if (g_ListMenu.iVectorY > 100)
			{
				g_ListMenu.iShiftY -= 25;
				g_ListMenu.iVectorY -= 25;
			}
			else if (g_ListMenu.iVectorY > 60)
			{
				g_ListMenu.iShiftY -= 16;
				g_ListMenu.iVectorY -= 16;
			}
			else if (g_ListMenu.iVectorY > 20)
			{
				g_ListMenu.iShiftY -= 8;
				g_ListMenu.iVectorY -= 8;
			}
			else if (g_ListMenu.iVectorY >= 4)
			{
				g_ListMenu.iShiftY -= 4;
				g_ListMenu.iVectorY -= 4;
			}
			else
			{
				g_ListMenu.iShiftY--;
				g_ListMenu.iVectorY--;
			}
		}
		else
		{
			if (g_ListMenu.iVectorY < -100)
			{
				g_ListMenu.iShiftY += 25;
				g_ListMenu.iVectorY += 25;
			}
			else if (g_ListMenu.iVectorY < -60)
			{
				g_ListMenu.iShiftY += 16;
				g_ListMenu.iVectorY += 16;
			}
			else if (g_ListMenu.iVectorY < -20)
			{
				g_ListMenu.iShiftY += 8;
				g_ListMenu.iVectorY += 8;
			}
			else if (g_ListMenu.iVectorY <= -4)
			{
				g_ListMenu.iShiftY += 4;
				g_ListMenu.iVectorY += 4;
			}
			else
			{
				g_ListMenu.iShiftY++;
				g_ListMenu.iVectorY++;
			}
		}
	}


	if (g_ListMenu.iShiftY > 0)
	{
		if (g_ListMenu.iShiftY > 18 * 2)
		{
			g_ListMenu.iShiftY = 18 * 2;
			g_ListMenu.iVectorY = (18 * 2);
		}
		if (g_ListMenu.iVectorY == 0)
			g_ListMenu.iVectorY += g_ListMenu.iShiftY;
	}
	else if (g_ListMenu.iShiftY < -bottom)
	{
		if (g_ListMenu.iShiftY < -(bottom + 18 * 2))
		{
			g_ListMenu.iShiftY = -(bottom + 18 * 2);
			g_ListMenu.iVectorY = -(18 * 2);
		}
		if (g_ListMenu.iVectorY == 0)
			g_ListMenu.iVectorY += (g_ListMenu.iShiftY + bottom);
	}

}


void ListMenu_MouseEvent(int mx, int my, Uint32 status)
{
	if (mx < g_ListMenu.dstrng.x) return;
	if (mx > g_ListMenu.dstrng.x + g_ListMenu.dstrng.w) return;
	if (my < g_ListMenu.dstrng.y) return;
	if (my > g_ListMenu.dstrng.y + g_ListMenu.dstrng.h) return;

	static BOOL isRoll;
	static int SX, SY;

	const int          iLinesMax = g_ListMenu.iLinesPerPage * 18;


	int x = ((mx - g_ListMenu.dstrng.x) / g_ListMenu.iItemTextWidth) + 1;
	if (x > g_ListMenu.iItemsPerLine) x = g_ListMenu.iItemsPerLine;
	if (x < 1) x = 1;
	int y = (my - g_ListMenu.dstrng.y + (-g_ListMenu.iShiftY)) / 18;
	if (y < 0) y = 0;
	int select = (y * g_ListMenu.iItemsPerLine) + x - 1;
	if (status == SDL_MOUSEBUTTONDOWN)
	{
		SX = mx;
		SY = my;
		isRoll = FALSE;
		g_ListMenu.iVectorY = 0;
	}
	else if (status == SDL_MOUSEMOTION)
	{

		if (SX - mx > 5 || SX - mx < -5 || SY - my >5 || SY - my < -5)
		{
			isRoll = TRUE;
		}
		else
			g_ListMenu.iVectorY += (SY - my);
		if (isRoll)
		{
			int p = SY - my;
			if (p < 0) p = -p;
			if (p == 0)
			{
				g_ListMenu.iVectorY = 0;
			}
			else if (p < 5)
			{
				g_ListMenu.iVectorY += (SY - my);
			}
			else if (p < 10)
			{
				g_ListMenu.iVectorY += (SY - my) * 4;
			}
			else
			{
				g_ListMenu.iVectorY += (SY - my) * 8;
			}
			SY = my;
		}
	}
	else if (status == SDL_MOUSEBUTTONUP)
	{
		if (isRoll == FALSE)
		{
			if (*g_ListMenu.iSelect != select)
			{
				if (select < *g_ListMenu.iCount)
					*g_ListMenu.iSelect = select;
			}
			else
			{
				g_ListMenu.wClick = 1;
			}
		}
		else
		{
			isRoll = FALSE;
		}
	}
}

void ListMenu_GoTpFocus()
{
	const int          iLinesMax = g_ListMenu.iLinesPerPage * 18;
	const int iHalfList = g_ListMenu.iLinesPerPage / 2;
	int bottom = (*g_ListMenu.iCount / g_ListMenu.iItemsPerLine);
	if (*g_ListMenu.iCount % g_ListMenu.iItemsPerLine != 0)
		bottom += 1;
	bottom = bottom * 18 - iLinesMax;
	if (bottom < 0) return;

	int selecty = *g_ListMenu.iSelect / g_ListMenu.iItemsPerLine;
	g_ListMenu.iVectorY = 0;
	
	if (selecty < iHalfList)
		g_ListMenu.iShiftY = 0;
	else
	{
		g_ListMenu.iShiftY = -(selecty - iHalfList) * 18;
		if (g_ListMenu.iShiftY < -bottom)
			g_ListMenu.iShiftY = -bottom;
	}
		
}

VOID
PAL_AdditionalCredits(
	VOID
)
/*++
Purpose:

Show the additional credits.

Parameters:

None.

Return value:

None.

--*/
{


	LPCWSTR rgszStrings[] = {
		L"  PAL2019 (https://github.com/aabby/sdlpal)",
		L"  by SOFTSTAR (C) 1995-2019",
		L"  include SDL, SDLPAL ",
		L"  ",
		L"  GNU General Public License v3",
		L"  ",
		L"  ",  // Porting information line 1
		L"  ",  // Porting information line 2
		L"  ",  // GNU line 1
		L"  ",  // GNU line 2
		L"  ",  // GNU line 3
		L"  ",  // Press Enter to continue
	};

	int        i = 0;

	PAL_DrawOpeningMenuBackground();

	for (i = 0; i < 4; i++)
	{
		//WCHAR buffer[48];
		//wcsncpy(buffer, rgszStrings[i],20);
		PAL_DrawText(rgszStrings[i], PAL_XY(0, 2 + i * 16), DESCTEXT_COLOR, TRUE, FALSE, FALSE);
	}

	PAL_SetPalette(0, FALSE);
	VIDEO_UpdateScreen(NULL);

	PAL_WaitForKey(0);
}
