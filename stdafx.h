#pragma once
#include <streams.h>
#include <shlwapi.h>
#include <sstream>
#include <cctype>       // std::toupper
#include <atlbase.h>
#include <atlconv.h>
#include <fstream>
#include <stdarg.h>
#include <cstdlib>
#include <time.h>
#include <stdio.h>
#include <initguid.h>
#include <regex>
#include <string.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil\avstring.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include "libswresample/swresample.h"
}

//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#define new DEBUG_NEW
//#endif