#include "pch.h"
#include "icome.h"
#include "player.h"
#include "miniMapHack.h"
#include "TextTagManager.h"
#include "unitTracker.h"
#include "memedit.h"
#include "mhDetect.h"
#include "safeclick.h"
#include <random>
HANDLE DrawMiniMapThread = 0;
unsigned int timerNO;
gamePlayerInfo* aPlayerInfo = new gamePlayerInfo();

inline void icome::updateTag() {
	static long long time = 0;
	time++;
	for (auto it = unitTrack::allunits.begin(); it != unitTrack::allunits.end();) {
		auto unitobject = jass::GetAddrByHandle(it->second->getHandle());
		if (unitobject) {
			//logger->info("refreshing unit {0:x} {1:x}", it->first, unitobject);
			it->second->refreshTag();
			if (time % 7 == 0) it->second->minimapIndicate();
			it++;
		}
		else {
			unitTrack::allunits.erase(it++);
		}
	}
}
static bool firstBoot = true;

bool icome::firstBOOT() {
	if (PostChatMessage("|cFFffff33[Author] \xE4\xBA\x8C\xE6\x9C\x88\xE9\x9C\x9C\xE5\x8D\x8E|r") &&
		PostChatMessage("|cFFffff33[In memory of] the machine and samaritan|r") &&
		PostChatMessage("|cFFffff33[We] are being watched|r")) {
		DisplayChatMessage("|cFFffff33[Author] \xE4\xBA\x8C\xE6\x9C\x88\xE9\x9C\x9C\xE5\x8D\x8E|r");
		DisplayChatMessage("|cFFffff33[In memory of] the machine and samaritan|r");
		DisplayChatMessage("|cFFffff33[We] are being watched|r");
		firstBoot = false;
		traverseUnits();
		return true;
	}
	else {
		return false;
	}
}

void CALLBACK icome::timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	static DWORD lastTime = 0;
	__try
	{
		if (IsInGame()) {
			aPlayerInfo->fresh();
			if (firstBoot) {
				firstBOOT(); 
				return;
			}
			unitTrack::processUnitCreationEvent();
			updateTag();
			if (dwTime - 10000 >= lastTime) {
				logger->flush();
				lastTime = dwTime;
			}
		}
		else {
			if (firstBoot == false) {
				firstBoot = true;
				unitTrack::allunits.clear();
				unitTrack::onUnitGenQueue.clear();
			}
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation()))
	{
		firstBoot = true;
		logger->error("icome::timer crashed gamedll {0:x}", (unsigned int)GetModuleHandle(L"Game.dll"));
		return;
	}
}

void icome::icome()
{
	unsigned int allowLocalFile = gameDll + 0x21080;
	_asm {
		push ecx
		mov ecx, 0x1
		call allowLocalFile;
		pop ecx
	}
	jass::init();
	memedit::applyPatch();
#ifndef LIMITED
	memedit::applyDetour();
	mhDetect::init();
	safeClick::init();
#endif
	unitTrack::hook();
	std::mt19937 g(GetTickCount());
	while (!SetTimer(hWnd, g(), 100, (TIMERPROC)timer));
	logger->info("My prey is near.");
}

void icome::traverseUnits() {
	unitTrack::allunits.clear();
	unitTrack::onUnitGenQueue.clear();
	void** arr = (void**)*(unsigned int*)(*(unsigned int*)(*(unsigned int*)(gameDll + 0xBE6350) + 0x3bc) + 0x608);
	unsigned int nCount = *(unsigned int*)(*(unsigned int*)(*(unsigned int*)(gameDll + 0xBE6350) + 0x3bc) + 0x604);
	for (unsigned int i = 0; i < nCount; ++i) {
		if (IsGameObjectPresent()) {
			unitTrack::UnitCreationEvent a;
			a.tickCount64 = GetTickCount64();
			a.unitAddr = (unsigned int)arr[i];
			unitTrack::onUnitGenQueue.push_back(a);
		}
	}
}