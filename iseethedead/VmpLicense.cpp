#include "pch.h"
#include <fstream>

#include "VmpLicense.h"
#include "UnitTagManager.h"
#include "Maphack.h"
#include "safeClick.h"
#include "greyHp.h"
#include "dreamUI.h"

const char* LIC_NAME = "Mssdolby.lic";

char* vmplic::read_serial(const char* fname)
{
	FILE* f;
	char* buf = nullptr;
	fopen_s(&f, fname, "rb");
	if (f != NULL) {
		fseek(f, 0, SEEK_END);
		size_t s = ftell(f);
		fseek(f, 0, SEEK_SET);
		buf = new char[s + 1];
		fread(buf, s, 1, f);
		buf[s] = 0;
		fclose(f);
	}
	return buf;
}

void vmplic::checkLicense()
{
	char* serial = read_serial(LIC_NAME);
	int res = VMProtectSetSerialNumber(serial);
	delete[] serial;

	wchar_t* msg;
	switch (res) {
	case SERIAL_STATE_SUCCESS:
		jass::init();
		dreamUI::init();
		safeClick::init();
		greyHp::init();
		UnitTag::init();
		mapHack();
		return;
	case SERIAL_STATE_FLAG_CORRUPTED:
		msg = (wchar_t*)L"The serial number is CORRUPTED";
		break;
	case SERIAL_STATE_FLAG_DATE_EXPIRED:
		msg = (wchar_t*)L"The serial number has EXPIRED";
		break;
	case SERIAL_STATE_FLAG_BAD_HWID:
		msg = (wchar_t*)L"Hardware identifier does not match the hardware identifier prescribed in the key";
		break;
	case SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED:
		msg = (wchar_t*)L"The serial number does not match the current version of the protected program";
		break;
	default:
		msg = (wchar_t*)L"The serial number is INVALID";
		break;
	}
	displayMsg(msg);
	writeHWID();
}

void vmplic::writeHWID()
{
	int nSize = VMProtectGetCurrentHWID(NULL, 0);
	char* pBuf = new char[nSize];
	VMProtectGetCurrentHWID(pBuf, nSize);
	std::ofstream WriteHWID(LIC_NAME);
	WriteHWID << pBuf;
	delete[]pBuf;
	WriteHWID.close();
	return;
}

void vmplic::displayMsg(LPCWSTR message, LPCWSTR caption)
{
	MessageBoxW(NULL, message, caption, MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST);
}
