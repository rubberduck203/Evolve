// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
#include <afxwin.h>

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


// EVOLVE INCLUDES
#include <assert.h>
#include <math.h>
#include "evolve_simulator.h"
#include "transfrm.h"
#include "ZoomHistory.h"
#include "OrganismFinder.h"

// EVOVLE APP INCLUDES
#include "Resource.h"
#include "NewUniverseOptions.h"
#include "utility.h"
#include "transfrm.h"
#include "evolveDoc.h"
#include "bplist.h"

#include "ColorStaticST.h"
#include "Appearance.h"
#include "WatchpointDialog.h"
#include "KforthDialog.h"
#include "KforthInstructionDialog.h"
#include "SimulationOptionsDialog.h"
#include "EvolveOptionsDialog.h"
#include "UniverseDialog.h"
#include "ViewOrganismDialog.h"
#include "StrainPopulation.h"
#include "FindDialog.h"
#include "NewUniverseDialog.h"
#include "AboutDialog.h"
#include "VisionDialog.h"
#include "EnergyTweakerDialog.h"

#include "memdc.h"

#include "MainFrm.h"
#include "evolveView.h"
#include "evolve.h"

