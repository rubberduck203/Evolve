// evolve.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "evolve.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "evolveDoc.h"
#include "evolveView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//
// CevolveApp
//

BEGIN_MESSAGE_MAP(CevolveApp, CWinApp)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_TOOLS_KFORTH_INTERPRETER, OnAppKforthInterpreter)
	ON_COMMAND(ID_TOOLS_OPTIONS, OnEvolveOptions)
	ON_COMMAND(ID_HELP_CONTENTS, OnHelp)
	ON_COMMAND(ID_HELP_KFORTH, OnHelpKforth)
	ON_COMMAND(ID_HELP_ORGANISMS, OnHelpOrganisms)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()

//
// CevolveApp construction
//
CevolveApp::CevolveApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	organism = NULL;
}

//
// The one and only CevolveApp object
//
CevolveApp theApp;

//
// CevolveApp initialization
//
BOOL CevolveApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored

	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

	SetRegistryKey( _T("StaufferComputerConsulting") );

	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(IDR_evolveTYPE,
		RUNTIME_CLASS(CevolveDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CevolveView));

	if (!pDocTemplate)
		return FALSE;

	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	m_pMainWnd = pMainFrame;

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if ( !ProcessShellCommand(cmdInfo) )
		return FALSE;

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	return TRUE;
}

void CevolveApp::OnAppAbout()
{
	AboutDlg aboutDlg;

	aboutDlg.DoModal();
}

void CevolveApp::OnFileNew()
{
	static NewUniverseDialog dlg;
	static int first_time = 1;
	INT_PTR result;

	//
	// hack to prevent new universe dialog when
	// program first begins
	//
	if( first_time ) {
		first_time = 0;
		return;
	}

	result = dlg.DoModal();

	if( result == IDOK ) {
		//
		// create new document
		//
		nuo = dlg.nuo;

		CWinApp::OnFileNew();
	}
}

void CevolveApp::OnFileOpen()
{
	CWinApp::OnFileOpen();
}

void CevolveApp::OnAppKforthInterpreter()
{
	KforthDialog kforthDlg;

	kforthDlg.DoModal();
}

void CevolveApp::OnSetInstallPath()
{
	// set registry entries for the install path
}

void CevolveApp::OnHelp()
{
	CString str;

	str = GetProfileString("help", "path");

	str = str + "\\contents.html";

	ShellExecute(m_pMainWnd->m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void CevolveApp::OnHelpKforth()
{
	CString str;

	str = GetProfileString("help", "path");

	str = str + "\\kforth_reference.html";

	ShellExecute(m_pMainWnd->m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void CevolveApp::OnHelpOrganisms()
{
	CString str;

	str = GetProfileString("help", "path");

	str = str + "\\organism_reference.html";

	ShellExecute(m_pMainWnd->m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void CevolveApp::OnEvolveOptions()
{
	INT_PTR nRet;

	EvolveOptionsDialog dlg;

	nRet = dlg.DoModal();
	if( nRet == IDOK ) {

	}
}

NewUniverseOptions *CevolveApp::GetNewUniverseOptions()
{
	return &nuo;
}

void CevolveApp::SetOrganism(ORGANISM *o)
{
	if( organism != NULL )
		Universe_FreeOrganism(organism);

	organism = o;
}

ORGANISM *CevolveApp::GetOrganism()
{
	return organism;
}
