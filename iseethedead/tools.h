#pragma once
#pragma comment(lib,"Version.lib")

unsigned int WarcraftVersion();
bool IsGameObjectPresent();
unsigned int ObjectToHandle(void*);
void PlantDetourCall(BYTE* , BYTE* , size_t );
bool IsInGame();
void DisplayText(const char* , float fDuration = 16.0f);
const char* ConvertOrderId(unsigned int);
uint32_t GetPlayerColorHEX(unsigned int c);
war3::MapInfo* MapInfoGet();
int filter(unsigned int code, struct _EXCEPTION_POINTERS* ep);
void HideLDRTable(HMODULE module);
bool FilterOrderId(unsigned int orderId);