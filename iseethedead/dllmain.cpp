// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "icome.h"
#include <iostream>
//#include "StealthDataArea.h"
//STEALTH_DATA_AREA(0x40000);
std::shared_ptr<spdlog::logger> logger;
unsigned int gameDll;
unsigned int localplayer = 0;
unsigned int localplayerslot = 0;
unsigned int hIsee = 0;
HWND hWnd;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		VM_TIGER_WHITE_START;
		hIsee = (unsigned int)hModule;
		//获取魔兽版本信息
		if (WarcraftVersion() == 52240) {
			DisableThreadLibraryCalls(hModule);
			HideLDRTable(hModule);
			gameDll = (unsigned int)GetModuleHandle(L"Game.dll");
			hWnd = FindWindowW(L"Warcraft III", L"Warcraft III");
			if (gameDll) {
				try
				{
					logger = spdlog::basic_logger_mt("isee", "isee.txt");
					logger->flush_on(spdlog::level::warn);
				}
				catch (const spdlog::spdlog_ex& ex)
				{
					std::cerr << "Log init failed: " << ex.what() << std::endl;
				}
				icome::icome();
				//vmplic::checkLicense(icome::icome);
			}
		
		}
		else {
			//REFERENCE_STEALTH_DATA_AREA;
			MessageBoxW(NULL, L"Support WarCraft III 1.27 only", L"Warning", MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST);
			return false;
		}
		VM_TIGER_WHITE_END;
		break;
	}
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

