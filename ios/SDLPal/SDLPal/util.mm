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

#include <Foundation/Foundation.h>
#include <UIKit/UIKit.h>
#include "common.h"
#include "palcfg.h"
#include "util.h"
#include "global.h"
#import <sys/utsname.h>


static char *runningPath = NULL;

LPCSTR
UTIL_BasePath(
   VOID
)
{
   static char buf[4096] = "";

   if (buf[0] == '\0')
   {
      char *p = SDL_GetBasePath();
      if (p != NULL)
      {
         strcpy(buf, p);
         free(p);
      }
   }

   return buf;
}

LPCSTR
UTIL_SavePath(
   VOID
)
{
   static char buf[4096] = "";

   if (buf[0] == '\0')
   {
      NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString *documentsDirectory = [[paths objectAtIndex:0] stringByAppendingString:@"/"];
      strcpy(buf, [documentsDirectory UTF8String]);
   }

   return buf;
}

BOOL
UTIL_GetScreenSize(
                   DWORD *pdwScreenWidth,
                   DWORD *pdwScreenHeight
                   )
{
    CGRect screenBound = [[UIScreen mainScreen] bounds];
    CGSize screenSize = screenBound.size;
    if (screenSize.height > screenSize.width)
    {
        *pdwScreenWidth = screenSize.height;
        *pdwScreenHeight = screenSize.width;
    }
    else
    {
        *pdwScreenWidth = screenSize.width;
        *pdwScreenHeight = screenSize.height;
    }
    return (pdwScreenWidth && pdwScreenHeight && *pdwScreenWidth && *pdwScreenHeight);
}

BOOL
UTIL_IsAbsolutePath(
                    LPCSTR  lpszFileName
                    )
{
    return FALSE;
}

void DetectLanguage()
{
    gpGlobals->wLanguage = 0;
    NSString *location=[[NSLocale preferredLanguages] objectAtIndex:0];
    
    if ([location length]>=5)
    {
        if([[location substringToIndex:3] isEqual:@"zh-"] &&
           (  ([location length]>=7 && [[location substringToIndex:7] isEqual:@"zh-Hant"]) ||
            [location isEqual:@"zh-TW"] ||
            [location isEqual:@"zh-HK"]))
        {
            gpGlobals->wLanguage = 0;
        }
        else if ([[location substringToIndex:3] isEqual:@"zh-"])
        {
            gpGlobals->wLanguage = 1;
        }
    }
}


INT
UTIL_Platform_Init(
                   int argc,
                   char* argv[]
                   )
{
    UTIL_LogAddOutputCallback([](LOGLEVEL, const char* str, const char*)->void {
        NSLog(@"%s",str);
    }, PAL_DEFAULT_LOGLEVEL);
    gConfig.fLaunchSetting = NO;
    runningPath = strdup(PAL_va(0,"%s/running", gConfig.pszGamePath));
    FILE *fp = fopen(runningPath, "w");
    if (fp) fclose(fp);
    
    DetectLanguage();
    return 0;
}

VOID
UTIL_Platform_Quit(
                   VOID
                   )
{
    unlink(runningPath);
    free(runningPath);
}

// 取得設備種類
// 0 iPad 1=iphone4 2=iphone5 3=is_iPhone6 4=iphone6+
int getDeviceMode()
{
    struct utsname systemInfo;
    uname(&systemInfo);
    
    NSString* deviceName = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding]; //getDeviceName
    
    bool is_iPhone4 = ([deviceName isEqualToString:@"iPhone3,1"] || [deviceName isEqualToString:@"iPhone3,3"] || [deviceName isEqualToString:@"iPhone4,1"]);
    bool is_iPhone5 = ([deviceName hasPrefix:@"iPhone5,"] || [deviceName hasPrefix:@"iPhone6,"]);
    bool is_iPhone6 =  ([deviceName isEqualToString:@"iPhone7,2"]);
    bool is_iPhone6Plus =  ([deviceName isEqualToString:@"iPhone7,1"]);
    
    int mode=0;//ipad
    if (is_iPhone4) //iphone4
        mode=1;
    else if (is_iPhone5) //iphone5
        mode=2;
    else if (is_iPhone6) //iphone6
        mode=3;
    else if (is_iPhone6Plus) //iphone6
        mode=4;
    return mode;
}

