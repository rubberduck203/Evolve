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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <crtdbg.h>

#else

//
// LUNIX PORTING STUFF
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

#endif

#include "evolve_simulator.h"
#include "evolve_simulator_private.h"
#include "phascii.h"
