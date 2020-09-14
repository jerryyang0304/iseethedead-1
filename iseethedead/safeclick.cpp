#include "pch.h"
#include "player.h"
#include "safeclick.h"

extern gamePlayerInfo* aPlayerInfo;
static unsigned int _4EFB0, UnitArray_v;
safeClick::CUnit_IsUnitVisible_t CUnit_IsUnitVisible;

bool __stdcall safeClick::SendActionHandler(CAction* lpAction)
{
	bool ret = TRUE;
	__try {
		VM_TIGER_WHITE_START
		if (lpAction->GetData()[0] == 0x16) {
			SelectionAction_t* SelectionAction = (SelectionAction_t*)lpAction->GetData();
			if (SelectionAction->wCount == 1 && SelectionAction->bMode == 1) {
				CUnit* aUnit = (CUnit*)jass::GetUnitThroughId(SelectionAction->dwUnitIds[0][0], SelectionAction->dwUnitIds[0][1]);
				if (aUnit != nullptr && !aUnit->IsVisibleToPlayer(PlayerLocal())) {
					unsigned int hUnit = ObjectToHandle(aUnit);
					if (!jass::GetPlayerAlliance(aPlayerInfo->getLocalPlayer(), jass::GetOwningPlayer(hUnit), 5)) {
						ret = false;
					}
				}
			}
		}
		VM_TIGER_WHITE_END
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		logger->warn("safeClick::SendActionHandler crashed");
	}
	return ret;
}

void __declspec(naked) safeClick::SendActionHook()
{
	VM_TIGER_WHITE_START;
	__asm
	{
		PUSH EBX
		CALL SendActionHandler
		TEST eax, eax
		JZ   dontsend
		MOV  ECX, 0Dh
		CALL DWORD PTR DS : [_4EFB0]
		RET

		dontsend :
		ADD ESP, 0x4
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RET
	}
	VM_TIGER_WHITE_END;
}

void safeClick::init()
{
	VM_TIGER_WHITE_START
	CUnit_IsUnitVisible = CUnit_IsUnitVisible_t(gameDll + 0x6516E0);
	UnitArray_v = gameDll + 0xBB9D88;
	_4EFB0 = gameDll + 0x4EFB0;
	//{0x30F1C2,safeClick::SendActionHook,5,nullptr}
	PlantDetourCall((BYTE*)gameDll + 0x30F1C2, (BYTE*)SendActionHook, 5);
	VM_TIGER_WHITE_END
}

bool safeClick::CUnit::IsVisibleToPlayer(int nPlayerIndex)
{
	return CUnit_IsUnitVisible(this, nPlayerIndex, 0, 4);
}
