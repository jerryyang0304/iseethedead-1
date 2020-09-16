#pragma once
#include <unordered_map>
#include <memory>
#include <vector>
/*
	draw mini map at 3BA960
*/
struct UnitLine {
	uint32_t unitHandle;
	int x, y;
	unsigned int color;
};

struct Unit {
	unsigned int character;
	unsigned int color;
};
struct MmapLoc
{
	int X, Y;
};

struct Loc
{
	float x, y;
	float unk = 1.0;
};

class MiniMapHack final {
private:
	std::unordered_map<void*, UnitLine> lines;
	std::unordered_map<void*, Unit> units;
	std::unordered_map<int, bool> restore;
	const int const2 = 2;
	uint32_t* miniMapBackup;
	uint32_t* gameMiniMap;
	float xMult, yMult, unk;
	int unk2, unk3;
	int minX, minY, maxX, maxY;

	MmapLoc LocationToMinimap(float x, float y);
	void draw_line(void* unit, UnitLine& obj);
	uint32_t CoordToMinimap(float Loc, DWORD offst);
	void draw_line(int x0, int y0, int x1, int y1, uint32_t c);
	void draw_unit(void*, Unit&);
	void DrawPixel(int x, int y, uint32_t color);
	void ConvertMmap(MmapLoc& loc);
	void Refresh();
	void CalMiniMapLoc(const Loc& main, Loc& mini);
	void Clean();
	void RestorMiniMap();
public:
	MiniMapHack();
	~MiniMapHack();
	void addLine(void* unit, float x, float y, unsigned int color);
	void delLine(void* unit);
	void addUnit(void* unit, unsigned int cha, unsigned int color);
	void delUnit(void* unit);
	void Clear();
	void DrawMiniMap();
};