#pragma once

namespace icome {
	extern bool isHackOn;
	void updateTag();
	bool firstBOOT();
	void miniMapHotKeys(DWORD dwTime);
	void CALLBACK timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	void icome();
	inline void traverseUnits();
	DWORD WINAPI DrawMiniMap(LPVOID para);
}