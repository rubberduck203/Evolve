// evolve.h : main header file for the evolve application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


class CevolveApp : public CWinApp
{
public:
	CevolveApp();


public:
	virtual BOOL InitInstance();

	NewUniverseOptions *GetNewUniverseOptions();

	void SetOrganism(ORGANISM *o);
	ORGANISM *GetOrganism();

private:
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnAppKforthInterpreter();
	afx_msg void OnSetInstallPath();
	afx_msg void OnHelp();
	afx_msg void OnHelpKforth();
	afx_msg void OnHelpOrganisms();
	afx_msg void OnEvolveOptions();

	NewUniverseOptions nuo;

	ORGANISM *organism;

	DECLARE_MESSAGE_MAP()

};

extern CevolveApp theApp;

//#define ASSERT(x)	assert(x)
#define MALLOC(x)	malloc(x)
#define CALLOC(n,s)	calloc(n,s)
#define REALLOC(p,s)	realloc(p,s)
#define FREE(x)		free(x)
#define STRDUP(x)	_strdup(x)

