// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __linux__

//
// WINDOWS
//

#pragma once


#include <iostream>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <crtdbg.h>
#include <windows.h>

#else
//
// LUNIX STUFF
//
#include <iostream>

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

typedef long long               __int64;
#define _ASSERTE(x)             assert(x)
#define stricmp(x,y)            strcasecmp(x,y)
#define strnicmp(x,y,z)         strncasecmp(x,y,z)
#define _atoi64(x)              atoll(x)
#define _strdup(x)              strdup(x)

#define _CRT_WARN               1
#define _CRT_ERROR              1
#define _CRT_ASSERT             1
#define _CRTDBG_MODE_FILE       1
#define _CRTDBG_FILE_STDOUT     1
#define _CrtSetReportMode(x,y)  0
#define _CrtSetReportFile(x,y)  0
#define _CrtDumpMemoryLeaks()   0
#define _tmain(x,y)             main(x,y)


#endif

#include "evolve_simulator.h"
