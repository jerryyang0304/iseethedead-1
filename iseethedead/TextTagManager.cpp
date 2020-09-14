#include "pch.h"
#include "TextTagManager.h"

PROTOTYPE_TextTagManager_CreateTag TextTagManager_CreateTag = NULL;

war3::CTextTagManager* GetMapTTM() {
	return MapInfoGet()->textTagManager;
}

uint32_t TTMCreateTag(war3::CTextTagManager* m, const char* text, double size) {
	if (!m || !text) return 0;
	if (!TextTagManager_CreateTag)
		TextTagManager_CreateTag = reinterpret_cast<PROTOTYPE_TextTagManager_CreateTag>(gameDll + 0x876C70);
	int intValue = -1;
	uint32_t realValue = 0;
	uint32_t tagId;

	uint32_t rv = TextTagManager_CreateTag(
		m,
		&intValue,
		text,
		&realValue,
		static_cast<float>(size),
		&realValue,
		static_cast<float>(0.0f),
		static_cast<float>(0.0f),
		&tagId,
		1,
		1,
		-1);
	//Set suspended
	//m->dataArray[tagId].flags |= 0x2;
	return tagId;
}