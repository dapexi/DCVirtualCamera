
#pragma once
#include "stdafx.h"

const int FilterIndex = 1;
std::string str = "Playout VCam" + std::to_string(FilterIndex);
std::wstring wstr(str.begin(), str.end());
#define g_wszVirtualCamera wstr.c_str()

#define FILTER_GUID "{022460A9-C742-453C-9EED-268AE3649B3"+FilterIndex+"}"
DEFINE_GUID(CLSID_VIRTUALCAMERAFILTER,
	0x22460a9, 0xc742, 0x453c, 0x9e, 0xed, 0x26, 0x8a, 0xe3, 0x64, 0x9b, 0x3 + FilterIndex);


#ifdef __cplusplus
extern "C" {
#endif

DEFINE_GUID(nullGuid, 
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

#ifdef __cplusplus
}
#endif
