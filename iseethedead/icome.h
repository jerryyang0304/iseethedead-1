#pragma once

namespace icome {
	extern bool isHackOn;
	void bootMiniMap();
	void CALLBACK timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	void icome();
	DWORD WINAPI DrawMiniMap(LPVOID para);
}