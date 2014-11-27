//
// WatchpointDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "WatchpointDialog.h"

//
// WatchpointDialog dialog
//
// This dialog is used by both the Kforth Interpreter and when tracing
// a cell from the View Organism Dialog.
//
// The puropose of this dialog is to allow the user to
// enter and verify watchpoint expressions. It also
// lets the user see a list of his breakpoints and he
// can remove them.
//
// Before calling DoModal(), the client must
// populate this class with the information that they
// want the user to manage.
//
// The user has 5 watchpoints and 10 breakpoints
//
//

IMPLEMENT_DYNAMIC(WatchpointDialog, CDialog)
WatchpointDialog::WatchpointDialog(CWnd* pParent /*=NULL*/)
	: CDialog(WatchpointDialog::IDD, pParent)
{
	m_debugging_cell = FALSE;
}

WatchpointDialog::~WatchpointDialog()
{
}

void WatchpointDialog::debugging_cell(BOOL f)
{
	m_debugging_cell = f;
}

void WatchpointDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void WatchpointDialog::OnHelp()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\watchpoint_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void WatchpointDialog::OnRemove()
{
}

void WatchpointDialog::OnDisable()
{
}

void WatchpointDialog::OnInstructions()
{
	KforthInstructionDialog dlg;
	INT_PTR nRet;

	if( m_debugging_cell ) {
		//dlg.show_instruction_set(KforthInstructionDialog::IS_WATCHPOINT_CELL);
	} else {
		//dlg.show_instruction_set(KforthInstructionDialog::IS_WATCHPOINT);
	}

	nRet = dlg.DoModal();
	if( nRet == IDCANCEL )
		return;

	//
	// insert instruction into current watchpoint field, if any
	// have focus.
	//

}

BOOL WatchpointDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

void WatchpointDialog::OnOK()
{
	CDialog::OnOK();
}

BEGIN_MESSAGE_MAP(WatchpointDialog, CDialog)
	ON_COMMAND(IDC_WP_HELP, OnHelp)
	ON_COMMAND(IDC_WP_REMOVE, OnRemove)
	ON_COMMAND(IDC_WP_DISABLE_ALL, OnDisable)
	ON_COMMAND(IDC_WP_INSTRUCTIONS, OnInstructions)
END_MESSAGE_MAP()


// WatchpointDialog message handlers
