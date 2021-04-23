
#ifdef _WINDOWS
#ifdef PAL_STEAM

#include "palcfg.h"
#include <steam_api.h>
#include <ISteamRemoteStorage.h>
#include "SteamMOD.h"

#include <sys/stat.h>
#include <iostream>

static MOD_LIST steammods;


extern "C" {
#include "common.h"
	extern void	UTIL_LogOutput(LOGLEVEL level, const char    *fmt, ...);
	extern FILE * UTIL_OpenFileAtPath(LPCSTR lpszPath, LPCSTR lpszFileName);
	extern bool UTIL_DirExists(const std::string &path);
	extern VOID UTIL_CloseFile(FILE *fp);

	char MOD_PATH[1024];
	char OGG_MOD_PATH[1024];
}


extern "C" int steam_ugc()
{
	steammods.init();
	return 0;
}

extern "C" bool steam_FindModFile(const char * filename)
{
	steammods.FindModFile(filename);
	if (MOD_PATH[0]!='\0')
		return true;
	return false;
}

extern "C" int steam_init()
{
	if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steam_appid.txt from the game depot.

		UTIL_LogOutput(LOGLEVEL_ERROR, "Steam must be running to play this game.\n");
		return -1;
	}

	// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
	// You don't necessarily have to though if you write your code to check whether all the Steam
	// interfaces are NULL before using them and provide alternate paths when they are unavailable.
	//
	// This will also load the in-game steam overlay dll into your process.  That dll is normally
	// injected by steam when it launches games, but by calling this you cause it to always load,
	// even when not launched via steam.
	if (!SteamAPI_Init())
	{
		UTIL_LogOutput(LOGLEVEL_ERROR, "Steam must be running to play this game.\n");
		return -1;
	}

	return 0;
}

static void FillMOD(MOD_ITEM* mod, PublishedFileId_t pvecPublishedFileID)
{
	if (mod->FileID[0] == '\0')
	{
		std::string id = std::to_string(pvecPublishedFileID);
		strcpy(mod->FileID, id.c_str());
	}
	mod->nPublishedFileID = pvecPublishedFileID;
	SteamUGC()->GetItemInstallInfo(mod->nPublishedFileID, &mod->punSizeOnDisk, mod->pchFolder, cchFolderSize, &mod->punTimeStamp);
	mod->state = SteamUGC()->GetItemState(mod->nPublishedFileID);
	if (*mod->pchFolder == 0)
	{
		mod->enable = false;
	}

}

void MOD_LIST::init()
{
	OGG_MOD_PATH[0] = '\0';
	SubscribedItemCount = SteamUGC()->GetNumSubscribedItems();
	if (SubscribedItemCount > 0)
	{
		PublishedFileId_t* pvecPublishedFileID;
		pvecPublishedFileID = new PublishedFileId_t[SubscribedItemCount];

		uint32 res = SteamUGC()->GetSubscribedItems(pvecPublishedFileID, SubscribedItemCount);
		if (res != SubscribedItemCount)
		{
			delete pvecPublishedFileID;
			SubscribedItemCount = 0;
			return;
		}

		uint32 TimeStamp = 0;
		list = new MOD_ITEM[SubscribedItemCount];
		for (int si = 0; si < SubscribedItemCount; si++)
		{
			memset(&list[si], 0, sizeof(MOD_ITEM));
		}

		FILE * fp = UTIL_OpenFileAtPath(gConfig.pszGamePath, "mod.txt");
		if (fp)
		{
			

			fseek(fp, 0, SEEK_END);
			long i = ftell(fp);

			LPBYTE temp = (LPBYTE)malloc(i);
			if (temp != NULL)
			{
				fseek(fp, 0, SEEK_SET);
				if (fread(temp, 1, i, fp) < (size_t)i)
				{
					free(temp);
				}
				else
				{
					//load mod.txt
					int count = 0;
					MOD_ITEM* mod;

					int p = 0;
					while (p < (int)i)
					{
						if (temp[p] == '+' || temp[p] == '-')
						{
							mod = &list[count];
							if (temp[p] == '+')
								mod->enable = true;
							else
								mod->enable = false;
							int c = 1;
							while (temp[p + c] != '\n' && temp[p + c] != '=')
							{
								mod->FileID[c - 1] = temp[p + c];
								c++;
							}
							mod->FileID[c] = '\0';
							count++;
							p += c - 1;
							while (temp[p] != '\n' && p < (int)i) //next line
								p++;
						}
						p++;
					}
				}
			}
			fclose(fp);
		}

		MOD_ITEM* mod;
		int pos = 0;
		int newpos = 0;
		while (pos < SubscribedItemCount)
		{
			std::string id = std::to_string(pvecPublishedFileID[pos]);
			bool find = false;
			for (int si = 0; si < SubscribedItemCount; si++)
			{
				mod = &list[si];
				if (id.compare(mod->FileID) == 0)
				{
					FillMOD(mod, pvecPublishedFileID[pos]);
					find = true;
					break;
				}
			}
			if (!find)
			{
				for (int si = newpos; si < SubscribedItemCount; si++)
				{
					mod = &list[si];
					if (mod->nPublishedFileID == 0)
					{
						FillMOD(mod, pvecPublishedFileID[pos]);
						mod->enable = true;
						newpos = si + 1;
						break;
					}
				}
			}
			pos++;
			
		}
		delete pvecPublishedFileID;

		//find ogg dir
		for (int si = SubscribedItemCount - 1; si >= 0; si--)
		{
			mod = &list[si];
			if (mod->nPublishedFileID != 0 && mod->enable)
			{
				std::string dir(mod->pchFolder);
				dir.append("\\ogg");
				if (UTIL_DirExists(dir))
				{
					strcpy(OGG_MOD_PATH, mod->pchFolder);
					break;
				}
			}
		}
	}
}



void MOD_LIST::FindModFile(const char * fname)
{
	MOD_PATH[0] = '\0';
	if (SubscribedItemCount <= 0)
		return;
	uint32 TimeStamp = 0;
	
	for (int i = SubscribedItemCount-1; i >= 0 ; i--)
	{
		MOD_ITEM* mod = &list[i];
		if (mod->nPublishedFileID != 0 && mod->enable)
		{
			FILE * fp = UTIL_OpenFileAtPath(mod->pchFolder, fname);
			if (fp)
			{
				UTIL_CloseFile(fp);
				strcpy(MOD_PATH, mod->pchFolder);
				break;
			}
		}
	}
}
#endif
#endif