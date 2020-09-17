#include "pch.h"
#include "mhDetect.h"
#include "miniMapHack.h"

const float d2r = 3.141592654f / 180.0f;
static char buff[256];
static float duration = 4.0f;
static mhDetect::OnDispatchUnitSelectionModify aOnDispatchUnitSelectionModify;
static mhDetect::OnDispatchSelectableSelectionModify aOnDispatchSelectableSelectionModify;
static mhDetect::OnPlayerOrder aOnPlayerOrder;
extern MiniMapHack* aMiniMapHack;

inline float mhDetect::GetUnitX(void* unit)
{
	return *(float*)((unsigned int)unit + 0x284);
}

inline float mhDetect::GetUnitY(void* unit)
{
	return *(float*)((unsigned int)unit + 0x288);
}

void mhDetect::testSelection(unsigned int objId1, unsigned int objId2, UnitSelectionModify* command)
{
	__try {
		void* aUnit = jass::GetUnitThroughId(objId1, objId2);
		if (aUnit != nullptr) {
			unsigned int hUnit = ObjectToHandle(aUnit);
			unsigned int hOwner = jass::GetOwningPlayer(hUnit);
			unsigned int eventOwner = jass::Player(command->playerNumber);
			if (jass::GetPlayerAlliance(eventOwner, hOwner, ALLIANCE_SHARED_VISION)) return;
			bool unitVisible = jass::IsUnitVisible(hUnit, eventOwner);

			if (unitVisible) {
				/*if (jass::IsPlayerEnemy(hOwner, eventOwner)) {
					_snprintf_s(buff, _TRUNCATE, "%s%s|r |cff00FF00->|r %s%s %s|r",
						GetPlayerColorString(command->playerNumber), jass::GetPlayerName(eventOwner),
						GetPlayerColorString(jass::GetPlayerColor(hOwner)), jass::GetUnitName(hUnit), jass::GetPlayerName(hOwner)
					);
					DisplayText(buff, 7.0f);
				}*/
				return;
			}

			float unitX = GetUnitX(aUnit);
			float unitY = GetUnitY(aUnit);
			float unitFacing = jass::GetUnitFacing(hUnit).fl * d2r;
			float ms = jass::GetUnitMoveSpeed(hUnit).fl / 2;      //roughly 0.5s delay
			float prevX = unitX - ms * cos(unitFacing);
			float prevY = unitY - ms * sin(unitFacing);
			bool prevlocVisible = jass::IsVisibleToPlayer(&prevX, &prevY, eventOwner);
			bool locVisible = jass::IsVisibleToPlayer(&unitX, &unitY, eventOwner);

			jass::PingMinimapEx(&unitX, &unitY, &duration, 255, 0, 0, true);
			char* clickType = (char*)"|cffFF0000";
			if (!locVisible && prevlocVisible) {
				clickType = (char*)"|cffFFFF00";
			}

			_snprintf_s(buff, _TRUNCATE, "%s%s|r %s->|r %s%s %s|r",
				GetPlayerColorString(command->playerNumber), jass::GetPlayerName(eventOwner),
				clickType,
				GetPlayerColorString(jass::GetPlayerColor(hOwner)), jass::GetUnitName(hUnit), jass::GetPlayerName(hOwner)
			);
			DisplayText(buff, 18.0f);
			if (!locVisible && prevlocVisible) {
				logger->info("mhDetect::testSelection possible {0} {1} -> {2} {3} {4}", 
					command->playerNumber, jass::GetPlayerName(eventOwner), jass::GetPlayerColor(hOwner), jass::GetUnitName(hUnit), jass::GetPlayerName(hOwner)
				);
			}
			else {
				logger->info("mhDetect::testSelection {0} {1} -> {2} {3} {4}",
					command->playerNumber, jass::GetPlayerName(eventOwner), jass::GetPlayerColor(hOwner), jass::GetUnitName(hUnit), jass::GetPlayerName(hOwner)
				);
			}
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		logger->warn("mhDetect::testSelection crashed");
	}
}

void __fastcall mhDetect::HookOnDispatchUnitSelectionModify(UnitSelectionModify* command)
{
	__try {
		aOnDispatchUnitSelectionModify(command);
		if (command->type == 1) {
			for (unsigned int i = 0; i < command->unitCount[0]; i++) {
				testSelection(command->selectedUnits[i].ObjectID1, command->selectedUnits[i].ObjectID2, command);
			}
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		logger->warn("mhDetect::HookOnDispatchUnitSelectionModify crashed");
	}
	
}

void __fastcall mhDetect::HookOnDispatchSelectableSelectionModify(SelectableSelectionModify* command)
{
	__try {
		aOnDispatchSelectableSelectionModify(command);
		void* aItem = jass::GetUnitThroughId(command->selectedSelectable.ObjectID1, command->selectedSelectable.ObjectID2);

		if (aItem != nullptr) {
			unsigned int eventOwner = jass::Player(command->playerNumber);
			unsigned int hItem = ObjectToHandle(aItem);
			if (hItem) {
				float itemX = jass::GetItemX(hItem).fl;
				float itemY = jass::GetItemY(hItem).fl;
				if (itemX == 0.00 && itemY == 0.00) return;
				if (!jass::IsVisibleToPlayer(&itemX, &itemY, eventOwner)) {
					jass::PingMinimapEx(&itemX, &itemY, &duration, 255, 0, 0, false);
					_snprintf_s(buff, _TRUNCATE, "%s%s|r |cffFF0000->|r %s",
						GetPlayerColorString(command->playerNumber), jass::GetPlayerName(eventOwner), jass::GetItemName(hItem)
					);
					DisplayText(buff, 18.0f);
					logger->info("mhDetect::OnDispatchSelectableSelectionModify {0} {1} -> {2}", command->playerNumber, jass::GetPlayerName(eventOwner), jass::GetItemName(hItem));
				}
			}
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		logger->warn("mhDetect::HookOnDispatchSelectableSelectionModify crashed");
	}
}

void mhDetect::DetectImpossibleOrder(ddd* d, void* targetObject, unsigned int eventOwner)
{
	__try {
		char* itemName = NULL;
		char* unitName = NULL;
		unsigned int h = ObjectToHandle(targetObject);
		itemName = jass::GetItemName(h);

		if (itemName && !jass::IsVisibleToPlayer(&d->x, &d->y, eventOwner)) {
			_snprintf_s(buff, _TRUNCATE, "%s%s|r [%s] |cffFF0000->|r %s",
				GetPlayerColorString(d->playerId), jass::GetPlayerName(eventOwner), ConvertOrderId(d->orderId), itemName
			);
			DisplayText(buff, 18.0f);
			jass::PingMinimapEx(&d->x, &d->y, &duration, 255, 0, 0, false);
			logger->info("mhDetect::DetectImpossibleOrder {0} {1} {2} {3}", d->playerId, jass::GetPlayerName(eventOwner), ConvertOrderId(d->orderId), itemName);
		}
		else if (unitName = jass::GetUnitName(h), unitName && !jass::IsUnitVisible(h, eventOwner)) {
			_snprintf_s(buff, _TRUNCATE, "%s%s|r [%s] |cffFF0000->|r %s",
				GetPlayerColorString(d->playerId), jass::GetPlayerName(eventOwner), ConvertOrderId(d->orderId), unitName
			);
			DisplayText(buff, 18.0f);
			jass::PingMinimapEx(&d->x, &d->y, &duration, 255, 0, 0, false);
			logger->info("mhDetect::DetectImpossibleOrder {0} {1} {2} {3}", d->playerId, jass::GetPlayerName(eventOwner), ConvertOrderId(d->orderId), unitName);
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		logger->warn("mhDetect::DetectImpossibleOrder crashed");
	}
}

void __fastcall mhDetect::HookOnPlayerOrder(void* triggerUnit, ddd* d, unsigned int dwZero1, unsigned int dwZero2)
{
	__try {
#ifndef LIMITED
		unsigned int eventOwner = jass::Player(d->playerId);
		void* targetObject = jass::GetUnitThroughId(d->targetObject.ObjectID1, d->targetObject.ObjectID2);
		if (FilterOrderId(d->orderId))	DetectImpossibleOrder(d, targetObject, eventOwner);
#endif // !LIMITED
		aOnPlayerOrder(triggerUnit, d, dwZero1, dwZero2);
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		logger->warn("mhDetect::HookOnPlayerOrder crashed");
	}
}

void mhDetect::init()
{
	aOnDispatchUnitSelectionModify = (OnDispatchUnitSelectionModify)(gameDll + 0x69F080);
	aOnDispatchSelectableSelectionModify = (OnDispatchSelectableSelectionModify)(gameDll + 0x69B7D0);
	aOnPlayerOrder = (OnPlayerOrder)(gameDll + 0x69BC60);
	int error = DetourTransactionBegin();
	if (error == NO_ERROR)
	{
		DetourUpdateThread(GetCurrentThread());
#ifndef LIMITED
		DetourAttach(&(PVOID&)aOnDispatchUnitSelectionModify, HookOnDispatchUnitSelectionModify);
		DetourAttach(&(PVOID&)aOnDispatchSelectableSelectionModify, HookOnDispatchSelectableSelectionModify);
#endif // !LIMITED
		DetourAttach(&(PVOID&)aOnPlayerOrder, HookOnPlayerOrder);
		DetourTransactionCommit();
	}	
}
