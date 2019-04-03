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
#include <float.h>
#include "main.h"

// Screen buffer
SDL_Surface              *gpScreen           = NULL;
SDL_Surface              *gpScreen240        = NULL;

// Backup screen buffer
SDL_Surface              *gpScreenBak        = NULL;
SDL_Surface              *gpScreenBak240     = NULL;

// The global palette
static SDL_Palette       *gpPalette          = NULL;

#if SDL_VERSION_ATLEAST(2,0,0)
SDL_Window               *gpWindow           = NULL;
SDL_Renderer      *gpRenderer         = NULL;
SDL_Texture       *gpTexture          = NULL;
SDL_Texture       *gpTexture240       = NULL;
SDL_Texture       *gpTouchOverlay     = NULL;
SDL_Rect           gOverlayRect;
SDL_Rect           gTextureRect;
SDL_Rect           gViewRect;
BOOL gDraw240 = FALSE;

DWORD g_dwScreenWidth = 0;
DWORD g_dwScreenHeight = 0;
float gpScreenW_Magnification = 0.0f;
float gpScreenH_Magnification = 0.0f;
float gpScreenH_Magnification240 = 0.0f;
float gpScreenW_MagnificationOriginal = 0.0f;
float gpScreenH_MagnificationOriginal = 0.0f;
float gpScreenH_MagnificationOriginal240 = 0.0f;

#if defined (__IOS__)
SDL_Surface *wallpaper = NULL;
SDL_Texture *wallpaper_texture = NULL;
#endif

static struct RenderBackend {
    void (*Init)();
    void (*Setup)();
    SDL_Texture *(*CreateTexture)(int width, int height);
    void (*RenderCopy)();
} gRenderBackend;
#else
#undef PAL_HAS_GLSL
#endif

// The real screen surface
SDL_Surface       *gpScreenReal       = NULL;
SDL_Surface       *gpScreenReal240    = NULL;

volatile BOOL g_bRenderPaused = FALSE;

static BOOL bScaleScreen = PAL_SCALE_SCREEN;

// Shake times and level
static WORD               g_wShakeTime       = 0;
static WORD               g_wShakeLevel      = 0;

#if PAL_HAS_GLSL
#include "video_glsl.h"
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
void VIDEO_SetupTouchArea(int window_w, int window_h, int draw_w, int draw_h)
{
	gOverlayRect.x = (window_w - draw_w) / 2;
	gOverlayRect.y = (window_h - draw_h) / 2;
	gOverlayRect.w = draw_w;
	gOverlayRect.h = draw_h;
#if PAL_HAS_TOUCH
	PAL_SetTouchBounds(window_w, window_h, gOverlayRect);
#endif
}

#define SDL_SoftStretch SDL_UpperBlit
static SDL_Texture *VIDEO_CreateTexture(int width, int height)
{
    double ratio = 320.0f / 240.0f;
    if (width % 2 != 0) width--;
    if (height % 2 != 0) height--;
    if (width > height)
    {
        gViewRect.w = width;
        gViewRect.h = height;

        gViewRect.w = (int)((double)height * ratio);
        if (gViewRect.w > width)
        {
            gViewRect.w = width;
            gViewRect.h = (int)((double)width * ratio);
        }
    }
    else
    {
        gViewRect.w = height;
        gViewRect.h = width;
        gViewRect.w = (int)((double)width * ratio);
        if (gViewRect.w > height)
        {
            gViewRect.w = height;
            gViewRect.h = (int)((double)height * ratio);
        }

    }

    gViewRect.x = (width - gViewRect.w) / 2;
    gViewRect.y = (height - gViewRect.h) / 2;
    if (gViewRect.x < 0) gViewRect.x = 0;
    if (gViewRect.y < 0) gViewRect.y = 0;

    
	int texture_width, texture_height;

	texture_width = 320;
	texture_height = 200;
	gTextureRect.x = gTextureRect.y = 0;
	gTextureRect.w = 320; gTextureRect.h = 200;
		
	VIDEO_SetupTouchArea(width,height,width,height);


	//
	// Create texture for screen.
	//
	return SDL_CreateTexture(gpRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, texture_width, texture_height);
}
#endif

void NullFunc() {}

INT
VIDEO_Startup(
   VOID
)
/*++
  Purpose:

    Initialze the video subsystem.

  Parameters:

    None.

  Return value:

    0 = success, -1 = fail to create the screen surface,
    -2 = fail to create screen buffer.

--*/
{
#if SDL_VERSION_ATLEAST(2,0,0)
   int render_w, render_h;

   gRenderBackend.Init = NullFunc;
   gRenderBackend.Setup = NullFunc;
   gRenderBackend.CreateTexture = VIDEO_CreateTexture;
   gRenderBackend.RenderCopy = VIDEO_RenderCopy;

#if PAL_HAS_GLSL
   if( gConfig.fEnableGLSL) {
	   gRenderBackend.Init = VIDEO_GLSL_Init;
	   gRenderBackend.Setup = VIDEO_GLSL_Setup;
	   gRenderBackend.CreateTexture = VIDEO_GLSL_CreateTexture;
	   gRenderBackend.RenderCopy = VIDEO_GLSL_RenderCopy;
   }
#endif
	
   gRenderBackend.Init();

   //
   // Before we can render anything, we need a window and a renderer.
   //
#if defined (__IOS__)
    //gConfig.dwScreenWidth = 320;
    //gConfig.dwScreenHeight = 240;
    gConfig.fFullScreen = YES;
#endif
    
   if (gpWindow == NULL)
   gpWindow = SDL_CreateWindow("Pal", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               gConfig.dwScreenWidth, gConfig.dwScreenHeight, PAL_VIDEO_INIT_FLAGS | (gConfig.fFullScreen ? SDL_WINDOW_BORDERLESS : 0));

   if (gpWindow == NULL)
   {
      return -1;
   }

   gpRenderer = SDL_CreateRenderer(gpWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

   gRenderBackend.Setup();

   if (gpRenderer == NULL)
   {
      return -1;
   }



#if defined (__IOS__)
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
   SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
    
#endif

   //
   // Create the screen buffer and the backup screen buffer.
   //
   gpScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
   gpScreenBak = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
   gpScreenReal = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 32,
                                       0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

#ifdef PAL_MODE240
   gConfig.fMode240 = TRUE;
#else
   gConfig.fMode240 = FALSE;
#endif

   //
   // init LISTMENU
   //
   g_ListMenu.ListScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8, 0, 0, 0, 0);
   VIDEO_Clean240();
   g_ListMenu.fDoUpdate = FALSE;

   if (gConfig.fMode240 == TRUE)
   {
	   gpScreen240 = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8, 0, 0, 0, 0);
	   gpScreenBak240 = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8, 0, 0, 0, 0);
	   gpScreenReal240 = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 32,
		   0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	   VIDEO_Clean240();
   }
   //
   // Create texture for screen.
   //
   SDL_GetRendererOutputSize(gpRenderer, &render_w, &render_h);
   if(!gConfig.fEnableGLSL)
      SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, gConfig.pszScaleQuality);
   gpTexture = gRenderBackend.CreateTexture(render_w, render_h);
   if (gConfig.fMode240 == TRUE)
   {
	   gpTexture240 = SDL_CreateTexture(gpRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
	   SDL_SetTextureBlendMode(gpTexture240, SDL_BLENDMODE_BLEND);
   }
   SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    g_dwScreenWidth = render_w;
    g_dwScreenHeight = render_h;
    gpScreenW_Magnification = (float)gViewRect.w / 320.0f;
    gpScreenH_Magnification = (float)gViewRect.h / 200.0f;
	gpScreenH_Magnification240 = (float)gViewRect.h / 240.0f;
    gpScreenW_MagnificationOriginal = 320.0f / (float)gViewRect.w;
    gpScreenH_MagnificationOriginal = 200.0f / (float)gViewRect.h;
	gpScreenH_MagnificationOriginal240 = 240.0f / (float)gViewRect.h;
    
   //
   // Create palette object
   //
   gpPalette = SDL_AllocPalette(256);

   //
   // Failed?
   //
   if (gpScreen == NULL || gpScreenBak == NULL || gpScreenReal == NULL || gpTexture == NULL || gpPalette == NULL || g_ListMenu.ListScreen == NULL)
   {
      VIDEO_Shutdown();
      return -2;
   }
   if (gConfig.fMode240 == TRUE)
   {
	   if (gpScreen240 == NULL || gpScreenBak240 == NULL || gpScreenReal240 == NULL || gpTexture240 == NULL)
	   {
		   VIDEO_Shutdown();
		   return -2;
	   }
   }
#if defined (__IOS__)
   if ((float)g_dwScreenHeight / (float)g_dwScreenWidth != 0.75f)
   {
	   SDL_Surface *tmp = UTIL_LoadBMP("wallpaper.bmp");
	   wallpaper = SDL_ConvertSurface(tmp, gpScreenReal->format, 0);
	   if (wallpaper)
	   {
		   wallpaper_texture = SDL_CreateTexture(gpRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, wallpaper->w, wallpaper->h);

		   if (wallpaper_texture)
		   {
			   void *texture_pixels;
			   int texture_pitch;
			   SDL_LockTexture(wallpaper_texture, NULL, &texture_pixels, &texture_pitch);

			   uint8_t *wpixels = (uint8_t *)texture_pixels;
			   uint8_t *wsrc = (uint8_t *)wallpaper->pixels;
			   memcpy(wpixels, wsrc, wallpaper->pitch * wallpaper->h);

			   SDL_UnlockTexture(wallpaper_texture);
		   }
		}
	}
#else
   //
   // Create texture for overlay.
   //
   if (gConfig.fUseTouchOverlay)
   {
      extern const void * PAL_LoadOverlayBMP(void);
      extern int PAL_OverlayBMPLength();

      SDL_Surface *overlay = SDL_LoadBMP_RW(SDL_RWFromConstMem(PAL_LoadOverlayBMP(), PAL_OverlayBMPLength()), 1);
      if (overlay != NULL)
      {
         SDL_SetColorKey(overlay, SDL_RLEACCEL, SDL_MapRGB(overlay->format, 255, 0, 255));
         SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, gConfig.pszScaleQuality);
         gpTouchOverlay = SDL_CreateTextureFromSurface(gpRenderer, overlay);
         SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
         SDL_SetTextureAlphaMod(gpTouchOverlay, 120);
         SDL_FreeSurface(overlay);
      }
   }
#endif
    
# if PAL_HAS_GLSL
   // notice: power of 2
#  define PIXELS 1
   // We need a total empty texture in case of not using touch overlay.
   // Or GL runtime will pick the previous texture - the main screen itself
   // and reuse it - that makes color seems overexposed
   else if( gConfig.fEnableGLSL )
   {
	   BYTE pixels[4*PIXELS*PIXELS];
	   memset(pixels, 0, sizeof(pixels));
	   SDL_Surface *temp = SDL_CreateRGBSurfaceFrom(pixels, PIXELS, PIXELS, 32, PIXELS, 0, 0, 0, 0);
	   gpTouchOverlay = SDL_CreateTextureFromSurface(gpRenderer, temp);
	   SDL_FreeSurface(temp);
   }
# endif
#else

   //
   // Create the screen surface.
   //
   gpScreenReal = SDL_SetVideoMode(gConfig.dwScreenWidth, gConfig.dwScreenHeight, 8, PAL_VIDEO_INIT_FLAGS);

   if (gpScreenReal == NULL)
   {
      //
      // Fall back to 640x480 software mode.
      //
      gpScreenReal = SDL_SetVideoMode(640, 480, 8,
         SDL_SWSURFACE | (gConfig.fFullScreen ? SDL_FULLSCREEN : 0));
   }

   //
   // Still fail?
   //
   if (gpScreenReal == NULL)
   {
      return -1;
   }

   gpPalette = gpScreenReal->format->palette;

   //
   // Create the screen buffer and the backup screen buffer.
   //
   gpScreen = SDL_CreateRGBSurface(gpScreenReal->flags & ~SDL_HWSURFACE, 320, 200, 8,
      gpScreenReal->format->Rmask, gpScreenReal->format->Gmask,
      gpScreenReal->format->Bmask, gpScreenReal->format->Amask);

   gpScreenBak = SDL_CreateRGBSurface(gpScreenReal->flags & ~SDL_HWSURFACE, 320, 200, 8,
      gpScreenReal->format->Rmask, gpScreenReal->format->Gmask,
      gpScreenReal->format->Bmask, gpScreenReal->format->Amask);

   //
   // Failed?
   //
   if (gpScreen == NULL || gpScreenBak == NULL)
   {
      VIDEO_Shutdown();
      return -2;
   }

   if (gConfig.fFullScreen)
   {
      SDL_ShowCursor(FALSE);
   }

#endif

   return 0;
}

VOID
VIDEO_Shutdown(
   VOID
)
/*++
  Purpose:

    Shutdown the video subsystem.

  Parameters:

    None.

  Return value:

    None.

--*/
{
#if PAL_HAS_GLSL
    // since gConfig is cleared already we'd to detect on side effects
	if( gRenderBackend.Init == VIDEO_GLSL_Init ) {
		VIDEO_GLSL_Destroy();
	}
#endif

   if (gpScreen != NULL)
   {
      SDL_FreeSurface(gpScreen);
   }
   gpScreen = NULL;

   if (gpScreenBak != NULL)
   {
      SDL_FreeSurface(gpScreenBak);
   }
   gpScreenBak = NULL;

#if SDL_VERSION_ATLEAST(2,0,0)

   if (gpScreen240 != NULL)
   {
	   SDL_FreeSurface(gpScreen240);
   }
   gpScreen240 = NULL;

   if (gpScreenBak240 != NULL)
   {
	   SDL_FreeSurface(gpScreenBak240);
   }
   gpScreenBak240 = NULL;

   if (gpTouchOverlay)
   {
      SDL_DestroyTexture(gpTouchOverlay);
   }
   gpTouchOverlay = NULL;

   if (gpTexture)
   {
	  SDL_DestroyTexture(gpTexture);
   }
   gpTexture = NULL;

   if (gpTexture240)
   {
	   SDL_DestroyTexture(gpTexture240);
   }
   gpTexture240 = NULL;

   if (gpRenderer)
   {
      SDL_DestroyRenderer(gpRenderer);
   }
   gpRenderer = NULL;

   if (gpWindow)
   {
      SDL_DestroyWindow(gpWindow);
   }
   gpWindow = NULL;

   if (gpPalette)
   {
      SDL_FreePalette(gpPalette);
   }

   if (g_ListMenu.ListScreen)
   {
	   SDL_FreeSurface(g_ListMenu.ListScreen);
   }
   g_ListMenu.ListScreen = NULL;

	#if defined (__IOS__)
   if (wallpaper_texture)
	   SDL_DestroyTexture(wallpaper_texture);
   wallpaper_texture = NULL;

   if (wallpaper)
	   SDL_FreeSurface(wallpaper);
   wallpaper = NULL;
	#endif
#endif
   gpPalette = NULL;

   if (gpScreenReal != NULL)
   {
      SDL_FreeSurface(gpScreenReal);
   }
   gpScreenReal = NULL;

   if (gpScreenReal240 != NULL)
   {
	   SDL_FreeSurface(gpScreenReal240);
   }
   gpScreenReal240 = NULL;
}

#if SDL_VERSION_ATLEAST(2,0,0)
VOID
VIDEO_RenderCopy(
   VOID
)
{
	void *texture_pixels;
	int texture_pitch;

	SDL_LockTexture(gpTexture, NULL, &texture_pixels, &texture_pitch);

    uint8_t *pixels = (uint8_t *)texture_pixels;
    uint8_t *src = (uint8_t *)gpScreenReal->pixels;
    memcpy(pixels, src, (320 << 2)*200);
    
	SDL_UnlockTexture(gpTexture);

	if (gpTexture240)
	{
		SDL_LockTexture(gpTexture240, NULL, &texture_pixels, &texture_pitch);

		pixels = (uint8_t *)texture_pixels;
		src = (uint8_t *)gpScreenReal240->pixels;
		memcpy(pixels, src, (320 << 2) * 240);

		SDL_UnlockTexture(gpTexture240);
	}

	SDL_RenderClear(gpRenderer);
#if defined (__IOS__)
    if (wallpaper_texture)
    {
        SDL_Rect destrect = {0,0,g_dwScreenWidth,g_dwScreenHeight};
        SDL_RenderCopy(gpRenderer, wallpaper_texture, NULL, &destrect);
    }
#endif
    SDL_Rect srcrng = {0,0,320,200};
	SDL_RenderCopy(gpRenderer, gpTexture, &srcrng, &gViewRect);

	if (gpTexture240)
	{
		srcrng.h = 240;
		SDL_RenderCopy(gpRenderer, gpTexture240, &srcrng, &gViewRect);
	}

	CheckRender();
	for (int i = 0; i < MAX_BUTTOM; i++)
	{
		if (gUI_Buttom[i].visable)
		{
			MakeTexture(i);
			if (gUI_Buttom[i].tex)
			{
				SDL_SetTextureAlphaMod(gUI_Buttom[i].tex, gUI_Buttom[i].Alpha);
				SDL_RenderCopy(gpRenderer, gUI_Buttom[i].tex, NULL, &gUI_Buttom[i].range);
			}
		}
	}
	SetGamePadTexture();
#if defined (__IOS__)
#else
	if (gConfig.fUseTouchOverlay)
	{
		SDL_RenderCopy(gpRenderer, gpTouchOverlay, NULL, &gOverlayRect);
	}
#endif
	SDL_RenderPresent(gpRenderer);
}
#endif

VOID
VIDEO_UpdateScreen(
   const SDL_Rect  *lpRect
)
/*++
  Purpose:

    Update the screen area specified by lpRect.

  Parameters:

    [IN]  lpRect - Screen area to update.

  Return value:

    None.

--*/
{
   SDL_Rect        srcrect, dstrect;
   short           offset = 240 - 200;
   short           screenRealHeight = gpScreenReal->h;
   short           screenRealY = 0;

#if SDL_VERSION_ATLEAST(2,0,0)
   if (g_bRenderPaused)
   {
	   return;
   }
#endif

   //
   // Lock surface if needed
   //
   if (SDL_MUSTLOCK(gpScreenReal))
   {
      if (SDL_LockSurface(gpScreenReal) < 0)
         return;
   }

   if (!bScaleScreen)
   {
      screenRealHeight -= offset;
      screenRealY = offset / 2;
   }

   if (lpRect != NULL)
   {
      dstrect.x = (SHORT)((INT)(lpRect->x) * gpScreenReal->w / gpScreen->w);
      dstrect.y = (SHORT)((INT)(screenRealY + lpRect->y) * screenRealHeight / gpScreen->h);
      dstrect.w = (WORD)((DWORD)(lpRect->w) * gpScreenReal->w / gpScreen->w);
      dstrect.h = (WORD)((DWORD)(lpRect->h) * screenRealHeight / gpScreen->h);

	  if (gDraw240 && gpScreen240)
	  {
		  //memset(gpScreenReal240->pixels, 0, 320 * 240 * 4);
		  if (dstrect.x == 0 && dstrect.y == 0 && dstrect.w == 320 && dstrect.h == 200)
		  {
			  SDL_SoftStretch(gpScreen240, NULL, gpScreenReal240, NULL);
		  }
		  else
			SDL_SoftStretch(gpScreen240, (SDL_Rect *)lpRect, gpScreenReal240, &dstrect);
		  gDraw240 = FALSE;
	  }
		 
	  SDL_SoftStretch(gpScreen, (SDL_Rect *)lpRect, gpScreenReal, &dstrect);

	  if (g_ListMenu.fDoUpdate == TRUE)
	  {
		  SDL_SoftStretch(g_ListMenu.ListScreen, &g_ListMenu.srcrng, gpScreenReal, &g_ListMenu.dstrng);
		  g_ListMenu.fDoUpdate = FALSE;
		  memset(g_ListMenu.ListScreen->pixels, 1, 320 * 240);
	  }
   }
   else if (g_wShakeTime != 0)
   {
      //
      // Shake the screen
      //
      srcrect.x = 0;
      srcrect.y = 0;
      srcrect.w = 320;
      srcrect.h = 200 - g_wShakeLevel;

      dstrect.x = 0;
      dstrect.y = screenRealY;
      dstrect.w = 320 * gpScreenReal->w / gpScreen->w;
      dstrect.h = (200 - g_wShakeLevel) * screenRealHeight / gpScreen->h;

      if (g_wShakeTime & 1)
      {
         srcrect.y = g_wShakeLevel;
      }
      else
      {
         dstrect.y = (screenRealY + g_wShakeLevel) * screenRealHeight / gpScreen->h;
      }

      SDL_SoftStretch(gpScreen, &srcrect, gpScreenReal, &dstrect);

      if (g_wShakeTime & 1)
      {
         dstrect.y = (screenRealY + screenRealHeight - g_wShakeLevel) * screenRealHeight / gpScreen->h;
      }
      else
      {
         dstrect.y = screenRealY;
      }

      dstrect.h = g_wShakeLevel * screenRealHeight / gpScreen->h;

      SDL_FillRect(gpScreenReal, &dstrect, 0);

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION <= 2
      dstrect.x = dstrect.y = 0;
      dstrect.w = gpScreenReal->w;
      dstrect.h = gpScreenReal->h;
#endif
      g_wShakeTime--;
   }
   else
   {
      dstrect.x = 0;
      dstrect.y = screenRealY;
      dstrect.w = gpScreenReal->w;
      dstrect.h = screenRealHeight;

      SDL_SoftStretch(gpScreen, NULL, gpScreenReal, &dstrect);

	  if (g_ListMenu.fDoUpdate == TRUE)
	  {
		  SDL_SoftStretch(g_ListMenu.ListScreen, &g_ListMenu.srcrng, gpScreenReal, &g_ListMenu.dstrng);
		  g_ListMenu.fDoUpdate = FALSE;
		  memset(g_ListMenu.ListScreen->pixels, 1, 320 * 240);
      }
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION <= 2
      dstrect.x = dstrect.y = 0;
      dstrect.w = gpScreenReal->w;
      dstrect.h = gpScreenReal->h;
#endif
   }

#if SDL_VERSION_ATLEAST(2,0,0)
   gRenderBackend.RenderCopy();
#else
   SDL_UpdateRect(gpScreenReal, dstrect.x, dstrect.y, dstrect.w, dstrect.h);
#endif

   if (SDL_MUSTLOCK(gpScreenReal))
   {
	   SDL_UnlockSurface(gpScreenReal);
   }
}

VOID
VIDEO_SetPalette(
   SDL_Color        rgPalette[256]
)
/*++
  Purpose:

    Set the palette of the screen.

  Parameters:

    [IN]  rgPalette - array of 256 colors.

  Return value:

    None.

--*/
{
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_Rect rect;
   if (gpScreen240)
   {
	   for (int i=0;i<255;i++)
		   rgPalette[i].a = 255;
	   rgPalette[1].a = 0;
	   rgPalette[0xF0].a = 0xC0;
   }
   
   SDL_SetPaletteColors(gpPalette, rgPalette, 0, 256);

   SDL_SetSurfacePalette(gpScreen, gpPalette);
   SDL_SetSurfacePalette(gpScreenBak, gpPalette);
   if (gpScreen240)
   {
	   SDL_SetSurfacePalette(gpScreen240, gpPalette);
	   SDL_SetSurfacePalette(gpScreenBak240, gpPalette);
   }
   if (g_ListMenu.ListScreen)
   {
	   SDL_SetSurfacePalette(g_ListMenu.ListScreen, gpPalette);
	   SDL_SetColorKey(g_ListMenu.ListScreen, TRUE, 1);
   }
   //
   // HACKHACK: need to invalidate gpScreen->map otherwise the palette
   // would not be effective during blit
   //
   SDL_SetSurfaceColorMod(gpScreen, 0, 0, 0);
   SDL_SetSurfaceColorMod(gpScreen, 0xFF, 0xFF, 0xFF);
   SDL_SetSurfaceColorMod(gpScreenBak, 0, 0, 0);
   SDL_SetSurfaceColorMod(gpScreenBak, 0xFF, 0xFF, 0xFF);
   if (gpScreen240)
   {
	   SDL_SetSurfaceColorMod(gpScreen240, 0, 0, 0);
	   SDL_SetSurfaceColorMod(gpScreen240, 0xFF, 0xFF, 0xFF);
	   SDL_SetSurfaceColorMod(gpScreenBak240, 0, 0, 0);
	   SDL_SetSurfaceColorMod(gpScreenBak240, 0xFF, 0xFF, 0xFF);
   }
   if (g_ListMenu.ListScreen)
   {
	   SDL_SetSurfaceColorMod(g_ListMenu.ListScreen, 0, 0, 0);
	   SDL_SetSurfaceColorMod(g_ListMenu.ListScreen, 0xFF, 0xFF, 0xFF);
   }
   rect.x = 0;
   rect.y = 0;
   rect.w = 320;
   rect.h = 200;

   BOOL Draw240 = gDraw240;
   if (gpScreen240) gDraw240 = TRUE;
   VIDEO_UpdateScreen(&rect);
   gDraw240 = Draw240;
#else
   SDL_SetPalette(gpScreen, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
   SDL_SetPalette(gpScreenBak, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
   SDL_SetPalette(gpScreenReal, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
# if defined(PAL_FORCE_UPDATE_ON_PALETTE_SET)
   {
      static UINT32 time = 0;
      if (SDL_GetTicks() - time > 50)
      {
	      SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
	      time = SDL_GetTicks();
      }
   }
# endif
#endif
}

VOID
VIDEO_Resize(
   INT             w,
   INT             h
)
/*++
  Purpose:

    This function is called when user resized the window.

  Parameters:

    [IN]  w - width of the window after resizing.

    [IN]  h - height of the window after resizing.

  Return value:

    None.

--*/
{
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_Rect rect;

   int render_w, render_h;
   SDL_GetRendererOutputSize(gpRenderer, &render_w, &render_h);
   w = render_w;
   h = render_h;
   if (gpTexture)
   {
      SDL_DestroyTexture(gpTexture);
   }

   gpTexture = gRenderBackend.CreateTexture(w, h);

   if (gpTexture == NULL)
   {
      TerminateOnError("Re-creating texture failed on window resize!\n");
   }

   rect.x = 0;
   rect.y = 0;
   rect.w = 320;
   rect.h = 200;

   g_dwScreenWidth = w;
   g_dwScreenHeight = h;
   gpScreenW_Magnification = (float)gViewRect.w / 320.0f;
   gpScreenH_Magnification = (float)gViewRect.h / 200.0f;
   gpScreenH_Magnification240 = (float)gViewRect.h / 240.0f;
   gpScreenW_MagnificationOriginal = 320.0f / (float)gViewRect.w;
   gpScreenH_MagnificationOriginal = 200.0f / (float)gViewRect.h;
   gpScreenH_MagnificationOriginal240 = 240.0f / (float)gViewRect.h;

   VIDEO_UpdateScreen(&rect);
#else
   DWORD                    flags;
   PAL_LARGE SDL_Color      palette[256];
   int                      i, bpp;

   //
   // Get the original palette.
   //
   if (gpScreenReal->format->palette != NULL)
   {
      for (i = 0; i < gpScreenReal->format->palette->ncolors; i++)
      {
         palette[i] = gpScreenReal->format->palette->colors[i];
      }
   }
   else i = 0;

   //
   // Create the screen surface.
   //
   flags = gpScreenReal->flags;
   bpp = gpScreenReal->format->BitsPerPixel;

   SDL_FreeSurface(gpScreenReal);
   gpScreenReal = SDL_SetVideoMode(w, h, bpp, flags);

   if (gpScreenReal == NULL)
   {
      //
      // Fall back to software windowed mode in default size.
      //
      gpScreenReal = SDL_SetVideoMode(PAL_DEFAULT_WINDOW_WIDTH, PAL_DEFAULT_WINDOW_HEIGHT, bpp, SDL_SWSURFACE);
   }

   SDL_SetPalette(gpScreenReal, SDL_PHYSPAL | SDL_LOGPAL, palette, 0, i);
   VIDEO_UpdateScreen(NULL);

   gpPalette = gpScreenReal->format->palette;
#endif
}

SDL_Color *
VIDEO_GetPalette(
   VOID
)
/*++
  Purpose:

    Get the current palette of the screen.

  Parameters:

    None.

  Return value:

    Pointer to the current palette.

--*/
{
   return gpPalette->colors;
}

VOID
VIDEO_ToggleScaleScreen(
   VOID
)
/*++
  Purpose:

    Toggle scalescreen mode, only used in some platforms.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   bScaleScreen = !bScaleScreen;
   VIDEO_Resize(PAL_DEFAULT_WINDOW_WIDTH, PAL_DEFAULT_WINDOW_HEIGHT);
   VIDEO_UpdateScreen(NULL);
}

VOID
VIDEO_ToggleFullscreen(
   VOID
)
/*++
  Purpose:

    Toggle fullscreen mode.

  Parameters:

    None.

  Return value:

    None.

--*/
{
#if SDL_VERSION_ATLEAST(2,0,0)
	if (gConfig.fFullScreen)
	{
		SDL_SetWindowFullscreen(gpWindow, 0);
		gConfig.fFullScreen = FALSE;
	}
	else
	{
		SDL_SetWindowFullscreen(gpWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
		gConfig.fFullScreen = TRUE;
	}
#else
   DWORD                    flags;
   PAL_LARGE SDL_Color      palette[256];
   int                      i, bpp;

   //
   // Get the original palette.
   //
   if (gpScreenReal->format->palette != NULL)
   {
      for (i = 0; i < gpScreenReal->format->palette->ncolors; i++)
      {
         palette[i] = gpScreenReal->format->palette->colors[i];
      }
   }

   //
   // Get the flags and bpp of the original screen surface
   //
   flags = gpScreenReal->flags;
   bpp = gpScreenReal->format->BitsPerPixel;

   if (flags & SDL_FULLSCREEN)
   {
      //
      // Already in fullscreen mode. Remove the fullscreen flag.
      //
      flags &= ~SDL_FULLSCREEN;
      flags |= SDL_RESIZABLE;
      SDL_ShowCursor(TRUE);
   }
   else
   {
      //
      // Not in fullscreen mode. Set the fullscreen flag.
      //
      flags |= SDL_FULLSCREEN;
      SDL_ShowCursor(FALSE);
   }

   //
   // Free the original screen surface
   //
   SDL_FreeSurface(gpScreenReal);

   //
   // ... and create a new one
   //
   if (gConfig.dwScreenWidth == 640 && gConfig.dwScreenHeight == 400 && (flags & SDL_FULLSCREEN))
   {
      gpScreenReal = SDL_SetVideoMode(640, 480, bpp, flags);
   }
   else if (gConfig.dwScreenWidth == 640 && gConfig.dwScreenHeight == 480 && !(flags & SDL_FULLSCREEN))
   {
      gpScreenReal = SDL_SetVideoMode(640, 400, bpp, flags);
   }
   else
   {
      gpScreenReal = SDL_SetVideoMode(gConfig.dwScreenWidth, gConfig.dwScreenHeight, bpp, flags);
   }

   VIDEO_SetPalette(palette);

   //
   // Update the screen
   //
   VIDEO_UpdateScreen(NULL);
#endif
}

VOID
VIDEO_ChangeDepth(
   INT             bpp
)
/*++
  Purpose:

    Change screen color depth.

  Parameters:

    [IN]  bpp - bits per pixel (0 = default).

  Return value:

    None.

--*/
{
#if !SDL_VERSION_ATLEAST(2,0,0)
   DWORD                    flags;
   int                      w, h;

   //
   // Get the flags and resolution of the original screen surface
   //
   flags = gpScreenReal->flags;
   w = gpScreenReal->w;
   h = gpScreenReal->h;

   //
   // Clear the screen surface.
   //
   SDL_FillRect(gpScreenReal, NULL, 0);

   //
   // Create the screen surface.
   //
   SDL_FreeSurface(gpScreenReal);
   gpScreenReal = SDL_SetVideoMode(w, h, (bpp == 0)?8:bpp, flags);

   if (gpScreenReal == NULL)
   {
      //
      // Fall back to software windowed mode in default size.
      //
      gpScreenReal = SDL_SetVideoMode(PAL_DEFAULT_WINDOW_WIDTH, PAL_DEFAULT_WINDOW_HEIGHT, (bpp == 0)?8:bpp, SDL_SWSURFACE);
   }

   gpPalette = gpScreenReal->format->palette;
#endif
}

VOID
VIDEO_SaveScreenshot(
   VOID
)
/*++
  Purpose:

    Save the screenshot of current screen to a BMP file.

  Parameters:

    None.

  Return value:

    None.

--*/
{
	char filename[32];
#ifdef _WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(filename, "%04d%02d%02d%02d%02d%02d%03d.bmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#elif !defined( GEKKO )
	struct timeval tv;
	struct tm *ptm;
	gettimeofday(&tv, NULL);
	ptm = localtime(&tv.tv_sec);
	sprintf(filename, "%04d%02d%02d%02d%02d%02d%03d.bmp", ptm->tm_year + 1900, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (int)(tv.tv_usec / 1000));
#endif

	//
	// Save the screenshot.
	//
#if SDL_VERSION_ATLEAST(2,0,0)
	SDL_SaveBMP(gpScreen, PAL_CombinePath(0, gConfig.pszSavePath, filename));
#else
	SDL_SaveBMP(gpScreenReal, PAL_CombinePath(0, gConfig.pszSavePath, filename));
#endif
}

VOID
VIDEO_ShakeScreen(
   WORD           wShakeTime,
   WORD           wShakeLevel
)
/*++
  Purpose:

    Set the screen shake time and level.

  Parameters:

    [IN]  wShakeTime - how many times should we shake the screen.

    [IN]  wShakeLevel - level of shaking.

  Return value:

    None.

--*/
{
   g_wShakeTime = wShakeTime;
   g_wShakeLevel = wShakeLevel;
}

VOID
VIDEO_SwitchScreen(
   WORD           wSpeed
)
/*++
  Purpose:

    Switch the screen from the backup screen buffer to the current screen buffer.
    NOTE: This will destroy the backup buffer.

  Parameters:

    [IN]  wSpeed - speed of fading (the larger value, the slower).

  Return value:

    None.

--*/
{
   int               i, j;
   const int         rgIndex[6] = {0, 3, 1, 5, 2, 4};
   SDL_Rect          dstrect;

   short             offset = 240 - 200;
   short             screenRealHeight = gpScreenReal->h;
   short             screenRealY = 0;

   if (!bScaleScreen)
   {
      screenRealHeight -= offset;
      screenRealY = offset / 2;
   }

   wSpeed++;
   wSpeed *= 10;

   for (i = 0; i < 6; i++)
   {
      for (j = rgIndex[i]; j < gpScreen->pitch * gpScreen->h; j += 6)
      {
         ((LPBYTE)(gpScreenBak->pixels))[j] = ((LPBYTE)(gpScreen->pixels))[j];
      }
	  if (gpScreen240)
	  {
		  for (j = rgIndex[i]; j < gpScreen240->pitch * gpScreen240->h; j += 6)
		  {
			  ((LPBYTE)(gpScreenBak240->pixels))[j] = ((LPBYTE)(gpScreen240->pixels))[j];
		  }
	  }
      //
      // Draw the backup buffer to the screen
      //
      dstrect.x = 0;
      dstrect.y = screenRealY;
      dstrect.w = gpScreenReal->w;
      dstrect.h = screenRealHeight;

	  if (SDL_MUSTLOCK(gpScreenReal))
	  {
		  if (SDL_LockSurface(gpScreenReal) < 0)
			  return;
	  }

      SDL_SoftStretch(gpScreenBak, NULL, gpScreenReal, &dstrect);

	  if (gpScreen240)
	  {
		  if (SDL_MUSTLOCK(gpScreenReal240))
		  {
			  if (SDL_LockSurface(gpScreenReal240) < 0)
				  return;
		  }

		  SDL_SoftStretch(gpScreenBak240, NULL, gpScreenReal240, NULL);
	  }
#if SDL_VERSION_ATLEAST(2, 0, 0)
      gRenderBackend.RenderCopy();
#else
      SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
#endif

	  if (SDL_MUSTLOCK(gpScreenReal))
	  {
		  SDL_UnlockSurface(gpScreenReal);
	  }
	  if (gpScreen240)
	  {
		  if (SDL_MUSTLOCK(gpScreenReal240))
		  {
			  SDL_UnlockSurface(gpScreenReal240);
		  }
	  }
      UTIL_Delay(wSpeed);
   }
}

VOID
VIDEO_FadeScreen(
   WORD           wSpeed
)
/*++
  Purpose:

    Fade from the backup screen buffer to the current screen buffer.
    NOTE: This will destroy the backup buffer.

  Parameters:

    [IN]  wSpeed - speed of fading (the larger value, the slower).

  Return value:

    None.

--*/
{
   int               i, j, k;
   DWORD             time;
   BYTE              a, b;
   const int         rgIndex[6] = {0, 3, 1, 5, 2, 4};
   SDL_Rect          dstrect;
   short             offset = 240 - 200;
   short             screenRealHeight = gpScreenReal->h;
   short             screenRealY = 0;

   //
   // Lock surface if needed
   //
   if (SDL_MUSTLOCK(gpScreenReal))
   {
      if (SDL_LockSurface(gpScreenReal) < 0)
         return;
   }
   if (gpScreenReal240 && SDL_MUSTLOCK(gpScreenReal240))
   {
	   if (SDL_LockSurface(gpScreenReal240) < 0)
		   return;
   }

   if (!bScaleScreen)
   {
      screenRealHeight -= offset;
      screenRealY = offset / 2;
   }

   time = SDL_GetTicks();

   wSpeed++;
   wSpeed *= 10;

   for (i = 0; i < 12; i++)
   {
      for (j = 0; j < 6; j++)
      {
         PAL_ProcessEvent();
         while (!SDL_TICKS_PASSED(SDL_GetTicks(), time))
         {
            PAL_ProcessEvent();
            SDL_Delay(5);
         }
         time = SDL_GetTicks() + wSpeed;

         //
         // Blend the pixels in the 2 buffers, and put the result into the
         // backup buffer
         //
         for (k = rgIndex[j]; k < gpScreen->pitch * gpScreen->h; k += 6)
         {
            a = ((LPBYTE)(gpScreen->pixels))[k];
            b = ((LPBYTE)(gpScreenBak->pixels))[k];

            if (i > 0)
            {
               if ((a & 0x0F) > (b & 0x0F))
               {
                  b++;
               }
               else if ((a & 0x0F) < (b & 0x0F))
               {
                  b--;
               }
            }

            ((LPBYTE)(gpScreenBak->pixels))[k] = ((a & 0xF0) | (b & 0x0F));
         }
		 if (gpScreen240)
		 {
			 for (k = rgIndex[j]; k < gpScreen240->pitch * gpScreen240->h; k += 6)
			 {
				 a = ((LPBYTE)(gpScreen240->pixels))[k];
				 b = ((LPBYTE)(gpScreenBak240->pixels))[k];

				 if (i > 0)
				 {
					 if ((a & 0x0F) > (b & 0x0F))
					 {
						 b++;
					 }
					 else if ((a & 0x0F) < (b & 0x0F))
					 {
						 b--;
					 }
				 }

				 ((LPBYTE)(gpScreenBak240->pixels))[k] = ((a & 0xF0) | (b & 0x0F));
			 }
		 }
         //
         // Draw the backup buffer to the screen
         //
         if (g_wShakeTime != 0)
         {
            //
            // Shake the screen
            //
            SDL_Rect srcrect, dstrect;

            srcrect.x = 0;
            srcrect.y = 0;
            srcrect.w = 320;
            srcrect.h = 200 - g_wShakeLevel;

            dstrect.x = 0;
            dstrect.y = screenRealY;
            dstrect.w = 320 * gpScreenReal->w / gpScreen->w;
            dstrect.h = (200 - g_wShakeLevel) * screenRealHeight / gpScreen->h;

            if (g_wShakeTime & 1)
            {
               srcrect.y = g_wShakeLevel;
            }
            else
            {
               dstrect.y = (screenRealY + g_wShakeLevel) * screenRealHeight / gpScreen->h;
            }

            SDL_SoftStretch(gpScreenBak, &srcrect, gpScreenReal, &dstrect);

            if (g_wShakeTime & 1)
            {
               dstrect.y = (screenRealY + screenRealHeight - g_wShakeLevel) * screenRealHeight / gpScreen->h;
            }
            else
            {
               dstrect.y = screenRealY;
            }

            dstrect.h = g_wShakeLevel * screenRealHeight / gpScreen->h;

            SDL_FillRect(gpScreenReal, &dstrect, 0);
#if SDL_VERSION_ATLEAST(2, 0, 0)
            gRenderBackend.RenderCopy();
#else
			SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
#endif
            g_wShakeTime--;
         }
         else
         {
            dstrect.x = 0;
            dstrect.y = screenRealY;
            dstrect.w = gpScreenReal->w;
            dstrect.h = screenRealHeight;

            SDL_SoftStretch(gpScreenBak, NULL, gpScreenReal, &dstrect);
			if (gpScreenReal240)
			{
				dstrect.x = 0; dstrect.y = 0;
				dstrect.w = 320; dstrect.h = 240;
				SDL_SoftStretch(gpScreenBak240, NULL, gpScreenReal240, &dstrect);
			}
#if SDL_VERSION_ATLEAST(2, 0, 0)
            gRenderBackend.RenderCopy();
#else
            SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
#endif
         }
      }
   }

   if (SDL_MUSTLOCK(gpScreenReal))
   {
      SDL_UnlockSurface(gpScreenReal);
   }

   if (gpScreenReal240 && SDL_MUSTLOCK(gpScreenReal))
   {
	   SDL_UnlockSurface(gpScreenReal240);
   }

   //
   // Draw the result buffer to the screen as the final step
   //
   VIDEO_UpdateScreen(NULL);
}

void
VIDEO_SetWindowTitle(
	const char*     pszTitle
)
/*++
  Purpose:

    Set the caption of the window.

  Parameters:

    [IN]  pszTitle - the new caption of the window.

  Return value:

    None.

--*/
{
#if SDL_VERSION_ATLEAST(2,0,0)
	SDL_SetWindowTitle(gpWindow, PAL_CONVERT_UTF8(pszTitle));
#else
	SDL_WM_SetCaption(pszTitle, NULL);
#endif
}

SDL_Surface *
VIDEO_CreateCompatibleSurface(
	SDL_Surface    *pSource
)
{
	return VIDEO_CreateCompatibleSizedSurface(pSource, NULL);
}

SDL_Surface *
VIDEO_CreateCompatibleSizedSurface(
	SDL_Surface    *pSource,
	const SDL_Rect *pSize
)
/*++
  Purpose:

    Create a surface that compatible with the source surface.

  Parameters:

    [IN]  pSource   - the source surface from which attributes are taken.
    [IN]  pSize     - the size (width & height) of the created surface.

  Return value:

    None.

--*/
{
	//
	// Create the surface
	//
	SDL_Surface *dest = SDL_CreateRGBSurface(pSource->flags,
		pSize ? pSize->w : pSource->w,
		pSize ? pSize->h : pSource->h,
		pSource->format->BitsPerPixel,
		pSource->format->Rmask, pSource->format->Gmask,
		pSource->format->Bmask, pSource->format->Amask);

	if (dest)
	{
		VIDEO_UpdateSurfacePalette(dest);
	}

	return dest;
}

SDL_Surface *
VIDEO_DuplicateSurface(
	SDL_Surface    *pSource,
	const SDL_Rect *pRect
)
/*++
  Purpose:

    Duplicate the selected area from the source surface into new surface.

  Parameters:

    [IN]  pSource - the source surface.
	[IN]  pRect   - the area to be duplicated, NULL for entire surface.

  Return value:

    None.

--*/
{
	SDL_Surface* dest = VIDEO_CreateCompatibleSizedSurface(pSource, pRect);

	if (dest)
	{
		VIDEO_CopySurface(pSource, pRect, dest, NULL);
	}

	return dest;
}

void
VIDEO_UpdateSurfacePalette(
	SDL_Surface    *pSurface
)
/*++
  Purpose:

    Use the global palette to update the palette of pSurface.

  Parameters:

    [IN]  pSurface - the surface whose palette should be updated.

  Return value:

    None.

--*/
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetSurfacePalette(pSurface, gpPalette);
#else
	if (gpPalette != NULL)
	{
		SDL_SetPalette(pSurface, SDL_PHYSPAL | SDL_LOGPAL, gpPalette->colors, 0, 256);
	}
#endif
}

VOID
VIDEO_DrawSurfaceToScreen(
    SDL_Surface    *pSurface
)
/*++
  Purpose:

    Draw a surface directly to screen.

  Parameters:

    [IN]  pSurface - the surface which needs to be drawn to screen.

  Return value:

    None.

--*/
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
   //
   // Draw the surface to screen.
   //
   if (g_bRenderPaused)
   {
      return;
   }
   SDL_BlitScaled(pSurface, NULL, gpScreenReal, NULL);
   gRenderBackend.RenderCopy();
#else
   SDL_Surface   *pCompatSurface;
   SDL_Rect       rect;

   rect.x = rect.y = 0;
   rect.w = pSurface->w;
   rect.h = pSurface->h;

   pCompatSurface = VIDEO_CreateCompatibleSizedSurface(gpScreenReal, &rect);

   //
   // First convert the surface to compatible format.
   //
   SDL_BlitSurface(pSurface, NULL, pCompatSurface, NULL);

   //
   // Draw the surface to screen.
   //
   SDL_SoftStretch(pCompatSurface, NULL, gpScreenReal, NULL);

   SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
   SDL_FreeSurface(pCompatSurface);
#endif
}

void VIDEO_Clean240()
{
	memset(g_ListMenu.ListScreen->pixels, 1, 320 * 240);
	if (gpScreen240 != NULL)
	{
		memset(gpScreen240->pixels, 1, 320 * 240);
		memset(gpScreenBak240->pixels, 1, 320 * 240);
		memset(gpScreenReal240->pixels, 0, 320 * 240 * 4);
	}
}

int VIDEO_BackupScreen(SDL_Surface * src)
{
	if (gDraw240 && src == gpScreen)
	{
		memcpy(gpScreenBak240->pixels, gpScreen240->pixels, 320 * 240);
	}

	return SDL_BlitSurface(src, NULL, gpScreenBak, NULL);
}

int VIDEO_RestoreScreen(SDL_Surface * dst)
{
	if (gDraw240 && dst == gpScreen)
	{
		memset(gpScreenReal240->pixels, 0, 320 * 240 * 4);
		memcpy(gpScreen240->pixels, gpScreenBak240->pixels, 320 * 240);
		SDL_SoftStretch(gpScreen240, NULL, gpScreenReal240, NULL);
	}

	return SDL_BlitSurface(gpScreenBak, NULL, dst, NULL);
}

int VIDEO_CopySurface(SDL_Surface * src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *tr)
{
	return SDL_BlitSurface(src, sr, dst, tr);
}

int VIDEO_CopyEntireSurface(SDL_Surface *src, SDL_Surface *dst)
{
	return SDL_BlitSurface(src, NULL, dst, NULL);
}
