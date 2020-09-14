#pragma once
#include "pch.h"
enum color {
	PLAYER_COLOR_RED = 0, PLAYER_COLOR_BLUE,
	PLAYER_COLOR_CYAN,
	PLAYER_COLOR_PURPLE,
	PLAYER_COLOR_YELLOW,
	PLAYER_COLOR_ORANGE,
	PLAYER_COLOR_GREEN,
	PLAYER_COLOR_PINK,
	PLAYER_COLOR_LIGHT_GRAY,
	PLAYER_COLOR_LIGHT_BLUE,
	PLAYER_COLOR_AQUA,
	PLAYER_COLOR_BROWN
};

int PlayerLocal();
war3::CPlayerWar3* PlayerObject(int playerId);
const char* GetPlayerColorString(int c);

typedef struct {
	bool isEnabled;
	unsigned int playerHandle;
	bool isAlly;
	bool isEnemy;
}playerInfo;

class gamePlayerInfo {
private:
	unsigned int loaclPlayerHandle = 0;
	unsigned int localPlayerSlot = 0;
	playerInfo players[12] = { 0 };
	bool ready = false;
public:
	gamePlayerInfo() {};
	~gamePlayerInfo() {};
	void fresh();
	bool isPlayerAlly(unsigned int slot)
	{
		if (slot > 12) return false;
		if (this->ready == false) return false;
		if (this->players[slot].isEnabled == false) return false;
		return this->players[slot].isAlly;
	}

	bool isPlayerEnemy(unsigned int slot)
	{
		if (slot > 12) return false;
		if (this->ready == false) return false;
		if (this->players[slot].isEnabled == false) return false;
		return this->players[slot].isEnemy;
	}

	unsigned int getLocalPlayer()
	{
		return this->loaclPlayerHandle;
	}

	unsigned int getLocalPlayerSlot()
	{
		if (this->ready == false) return 0;
		return this->localPlayerSlot;
	}

	unsigned int getPlayerHandle(unsigned int slot)
	{
		if (slot > 12) return false;
		if (this->ready == false) return 0;
		if (this->players[slot].isEnabled == false) return 0;
		return this->players[slot].playerHandle;
	}
};