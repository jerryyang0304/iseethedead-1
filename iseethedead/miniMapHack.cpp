/*
	无法精确的找到大地图转换小地图的方法
	暂时（永久）停用
*/
#include "pch.h"
#include "miniMapHack.h"

inline float GetUnitX(void* unit) {
	return *(float*)((unsigned int)unit + 0x284);
};
inline float GetUnitY(void* unit) {
	return *(float*)((unsigned int)unit + 0x288);
};

MiniMapHack::MiniMapHack() {
	miniMapBackup = (uint32_t*)new uint32_t[255][255];
}

MiniMapHack::~MiniMapHack() {
	delete[]miniMapBackup;
}

uint32_t MiniMapHack::CoordToMinimap(float Loc, DWORD offst)
{
#define ADDR(X,REG)\
	__asm MOV REG,DWORD PTR DS:[X] \
	__asm MOV REG,DWORD PTR DS:[REG]
	//+88BE3F
	DWORD _W3XConversion, addrConvertCoord1, addrConvertCoord2;
	uint32_t result = 0;
	float Result = 0;
	_W3XConversion = gameDll + 0xBB82BC;
	addrConvertCoord1 = gameDll + 0x528B0;
	addrConvertCoord2 = gameDll + 0x52F30;
	//'game.dll' + 2F71C + 305A9
	__asm
	{
		ADDR(_W3XConversion, EAX)
		add EAX, dword ptr[offst]
		PUSH EAX
		LEA EDX, Loc	//edx == p -> float
		LEA ECX, Result	//ecx == p -> dword
		CALL addrConvertCoord1
		MOV ECX,DWORD PTR DS:[EAX]
		LEA EAX,DWORD PTR DS:[ECX+0xFD000000]
		XOR EAX,ECX
		SAR EAX,0x1F
		LEA EDX,DWORD PTR DS:[ECX+0xFD800000]
		NOT EAX
		AND EAX,EDX
		LEA ECX,Result
		MOV DWORD PTR DS:[ECX],EAX
		CALL addrConvertCoord2
		MOV result, EAX
	}
	logger->info("{} Loc {} result {}", offst, Loc, result);
	return result;
}

void MiniMapHack::ConvertMmap(MmapLoc& loc)
{
	uint32_t addrW3Minimap = gameDll + 0xBE6DC4;
	uint32_t base = *(uint32_t*)(addrW3Minimap);
	maxX = *(int*)(base + 0x224);
	minX = *(int*)(base + 0x228);
	minY = *(int*)(base + 0x22C);
	maxY = *(int*)(base + 0x230);
	xMult = *(float*)(base + 0x21c);
	yMult = *(float*)(base + 0x220);
	unk = *(float*)(gameDll + 0x961898);
	unk2 = *(uint32_t*)(base + 0x214);
	unk3 = *(uint32_t*)(base + 0x218);
	int X = loc.X / 4, Y = loc.Y / 4;
	X = max(X, minX); X = min(X, maxX);
	Y = max(Y, minY); Y = min(Y, maxY);
	X -= minX; Y -= minY;
	float fX = X, fY = Y;
	fX = fX * xMult + unk;
	fY = fY * yMult + unk;
	__asm
	{
		MOV EAX, fX
		SHR EAX, 0xE
		MOVZX EAX, AL
		MOV X, EAX
		MOV EAX, fY
		SHR EAX, 0xE
		MOVZX EAX, AL
		MOV Y, EAX
	};
	X += unk2 - 2;
	Y = 0x100 - Y - unk3 - 2;
	logger->info("Loc {},{} result {} {}", loc.X, loc.Y, X, Y);
	loc.X = X;
	loc.Y = Y;
}

MmapLoc MiniMapHack::LocationToMinimap(float x, float y)
{
	MmapLoc ret;
	ret.X = CoordToMinimap(x, 0x6C);
	ret.Y = CoordToMinimap(y, 0x70);
	//logger->info("ret0 {},{}", ret.X, ret.Y);
	ConvertMmap(ret);
	//logger->info("ret1 {},{}", ret.X, ret.Y);
	//Loc test{ x,y };
	//Loc testret{ x,y };
	//CalMiniMapLoc(test, testret);
	//logger->info("testret {},{}", testret.x, testret.y);
	return ret;
}

inline void MiniMapHack::draw_line(int x0, int y0, int x1, int y1, uint32_t c) {
	// Bresenham's line algorithm
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	while (true) {
		DrawPixel(x0 , y0, c);
		DrawPixel(x0 + 1, y0, c);
		DrawPixel(x0 - 1, y0, c);
		DrawPixel(x0, y0 + 1, c);
		DrawPixel(x0, y0 - 1, c);
		if (x0 == x1 && y0 == y1) break;
		e2 = err;
		if (e2 > -dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void MiniMapHack::draw_unit(void* unit, Unit& obj)
{
	MmapLoc ret;
	ret.X = CoordToMinimap(GetUnitX(unit), 0x6C);
	ret.Y = CoordToMinimap(GetUnitY(unit), 0x70);
	ConvertMmap(ret);
	DrawPixel(ret.X, ret.Y, obj.color);
	DrawPixel(ret.X + 1, ret.Y, obj.color);
	DrawPixel(ret.X - 1, ret.Y, obj.color);
	DrawPixel(ret.X - 1, ret.Y + 1, obj.color);
	DrawPixel(ret.X - 1, ret.Y - 1, obj.color);
}

void MiniMapHack::DrawPixel(int x, int y, uint32_t color)
{
	size_t index = 0;
	__asm
	{

		MOV EAX, y
		SHL EAX, 8
		MOV ECX, x
		ADD EAX, ECX
		MOV index, EAX
	}
	int offset = (y << 8) + x;
	restore[index] = true;
	gameMiniMap[index] = color;
}

void MiniMapHack::addLine(void* unit, float x, float y, unsigned int color) {
	MmapLoc to = LocationToMinimap(x, y);
	lines[unit] = UnitLine{ ObjectToHandle(unit), to.X, to.Y , color };
}

void MiniMapHack::delLine(void* unit)
{
	lines.erase(unit);
}

void MiniMapHack::addUnit(void* unit,unsigned int cha, unsigned int color)
{
	units[unit] = Unit{ cha, color };
}

void MiniMapHack::delUnit(void* unit)
{
	lines.erase(unit);
}

void MiniMapHack::Clear()
{
	lines.clear();
	restore.clear();
}

void MiniMapHack::Clean()
{
	for (auto iter = lines.begin(); iter != lines.end();) {
		if (jass::IsUnitDead(iter->second.unitHandle)) {
			lines.erase(iter++);
		}
		else {
			iter++;
		}
	}
}

void MiniMapHack::Refresh()
{
	uint32_t addrW3Minimap = gameDll + 0xBE6DC4;
	uint32_t base = *(uint32_t*)(addrW3Minimap);
	if (base) {
		{
			uint32_t eax, ebx, ecx, edx;
			ebx = base;
			edx = *(uint32_t*)(ebx + 0x218);
			ecx = edx;
			//eax = *(uint32_t*)(ebx + 0x1d8);
			ecx <<= 0xA;
			//eax += ecx;
			eax = *(uint32_t*)(ebx + 0x17c);
			eax = *(uint32_t*)eax;
			eax += ecx;
			if ((uint32_t*)eax != gameMiniMap) {
				Clear();
				memcpy_s(miniMapBackup, 255 * 255 * 4, (uint32_t*)eax, 255 * 255 * 4);
				gameMiniMap = (uint32_t*)eax;
			}
		}
	}
}

void MiniMapHack::CalMiniMapLoc(const Loc& main, Loc& mini) {
	//uint32_t W3Minimap = *(uint32_t*)(gameDll + 0xBE6DC4) + 0x750;
	//void* addrMainMapLoc2MiniMapLoc = (void *)(gameDll + 0xD0C20);
	//void* pmini = &mini;
	//void* pmain = &main;

	//_asm {
	//	push W3Minimap
	//	mov ecx, pmini
	//	mov edx, pmain
	//	call addrMainMapLoc2MiniMapLoc
	//}
	const int CAMERABOUNDFIX = 1024;

	float WorldMinX = jass::GetCameraBoundMinX().fl - CAMERABOUNDFIX;
	float WorldMaxX = jass::GetCameraBoundMaxX().fl + CAMERABOUNDFIX;
	float WorldMinY = jass::GetCameraBoundMinY().fl - CAMERABOUNDFIX;
	float WorldMaxY = jass::GetCameraBoundMaxY().fl + CAMERABOUNDFIX;
	float WorldSizeX = WorldMaxX - WorldMinX;
	float WorldSizeY = WorldMaxY - WorldMinY;
	mini.x = 255 * (main.x - WorldMinX) / WorldSizeX;
	mini.y = 255 - 255 * (main.y - WorldMinY) / WorldSizeY;
}

void MiniMapHack::DrawMiniMap()
{
	Refresh();
	Clean();
	for (auto iter = lines.begin(); iter != lines.end(); iter++) {
		draw_line(iter->first, iter->second);
	}
	/*for (auto iter = units.begin(); iter != units.end(); iter++) {
		draw_unit(iter->first, iter->second);
	}*/
	RestorMiniMap();
}

void MiniMapHack::draw_line(void* unit, UnitLine& obj)
{
	float ux, uy;
	ux = GetUnitX(unit), uy = GetUnitY(unit);
	if (abs(ux - obj.x) < 64 && abs(uy - obj.y) < 64) return;

	MmapLoc from = LocationToMinimap(ux, uy);
	draw_line(from.X, from.Y, obj.x, obj.y, obj.color);
}

void MiniMapHack::RestorMiniMap()
{
	for (auto it = restore.begin(); it != restore.end(); it++) {
		if (it->second == true) {
			it->second = false;
		}
		else {
			gameMiniMap[it->first] = miniMapBackup[it->first];
		}
	}
}