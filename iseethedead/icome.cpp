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

//MiniMapHack* aMiniMapHack = new MiniMapHack();
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

//void icome::miniMapHotKeys(DWORD dwTime) {
//	static DWORD lastToggleCount = 0;
//	static DWORD exitCode1;
//	if (GetAsyncKeyState(VK_HOME) && (dwTime - lastToggleCount) > 200) {
//		GetExitCodeThread(DrawMiniMapThread, &exitCode1);
//		if (exitCode1 != STILL_ACTIVE) {
//			CloseHandle(DrawMiniMapThread);
//			DrawMiniMapThread = CreateThread(NULL, NULL, icome::DrawMiniMap, NULL, NULL, NULL);
//		}
//		lastToggleCount = dwTime;
//	}
//	GetExitCodeThread(DrawMiniMapThread, &exitCode1);
//	if (DrawMiniMapThread != 0 && exitCode1 != STILL_ACTIVE) {
//		DisplayText("|cFFffff33MiniMap Drawer has crashed|r", 18.0f);
//		CloseHandle(DrawMiniMapThread);
//		DrawMiniMapThread = 0;
//	}
//}

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
			//miniMapHotKeys(dwTime);
		}
		else {
			if (firstBoot == false) {
				firstBoot = true;
				unitTrack::allunits.clear();
				unitTrack::onUnitGenQueue.clear();
				//aMiniMapHack->Clear();
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
	mhDetect::init();
#ifndef LIMITED
	memedit::applyDetour();
	safeClick::init();
#endif
	unitTrack::hook();
	std::mt19937 g(GetTickCount());
	while (!SetTimer(hWnd, g(), 100, (TIMERPROC)timer));
	logger->info("My prey is near.");
}

void icome::traverseUnits() {
	VM_TIGER_WHITE_START
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
	VM_TIGER_WHITE_END
}

#include "unitTracker.h"

//DWORD WINAPI icome::DrawMiniMap(LPVOID para) {
//	__try
//	{
//		logger->info("icome::DrawMiniMap started");
//		if (IsInGame()) {
//			aMiniMapHack->Clear();
//			while (true) {
//				if (IsInGame()) {
//					aMiniMapHack->DrawMiniMap(); 
//					Sleep(150);
//				}
//				else {
//					break;
//				}
//			}
//		}
//		logger->info("icome::DrawMiniMap returned normally");
//		return 0;
//	}
//	__except (filter(GetExceptionCode(), GetExceptionInformation()))
//	{
//		logger->error("icome::DrawMiniMap crashed");
//		return 0;
//	}
//}