#pragma once

#include <steam_api.h>
#include <ISteamRemoteStorage.h>
#include <string> 


#define cchFolderSize 1024
typedef struct tagMOD_ITEM
{
	bool enable;
	char FileID[21];
	PublishedFileId_t nPublishedFileID;
	uint64 punSizeOnDisk;
	char pchFolder[cchFolderSize];
	uint32 punTimeStamp;
	uint32 state;
} MOD_ITEM;

class MOD_LIST
{
public:
	uint32 SubscribedItemCount; //訂閱物品總數

	MOD_ITEM* list;

	MOD_LIST()
	{
		memset(this, 0, sizeof(MOD_LIST));
	}
	~MOD_LIST()
	{
		if (list != nullptr)
		{
			delete list;
		}
	}
	void init();

	MOD_ITEM* GetMod(int index)
	{
		return &list[index];
	}

	std::string GetModString(int index)
	{
		MOD_ITEM* mod = &list[index];
		std::string var;
		if ((mod->state & k_EItemStateInstalled) != 0)
		{
			if (mod->enable)
				var.append("v  ");
			else
				var.append("   ");
			char tmp[255];
			std::sprintf(tmp, "%lu ", mod->nPublishedFileID);
			var.append(tmp);
		}
		return var;

	}

	void FindModFile(const char * fname);

};
