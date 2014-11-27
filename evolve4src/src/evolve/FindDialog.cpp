//
// FindDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "FindDialog.h"


// FindDialog dialog

IMPLEMENT_DYNAMIC(FindDialog, CDialog)
FindDialog::FindDialog(CWnd* pParent /*=NULL*/)
	: CDialog(FindDialog::IDD, pParent)
{
	m_last_instruction = "ID";
}

FindDialog::~FindDialog()
{
}

void FindDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void FindDialog::Instructions()
{
	KforthInstructionDialog dlg;
	CString str;
	INT_PTR nRet;

	dlg.insert_button(TRUE);
	dlg.show_instruction_set(dlg.IS_FIND);
	dlg.go_to(m_last_instruction);

	nRet = dlg.DoModal();
	if( nRet == IDOK ) {
		m_last_instruction = str;
		str = dlg.GetInstruction();
		str = str + " ";
		CEdit *pED = (CEdit*) GetDlgItem(IDC_FO_EDIT);
		pED->ReplaceSel(str);
	}

}

void FindDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\find_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void FindDialog::OnOK()
{
	CString str;
	CEdit *pED = (CEdit*) GetDlgItem(IDC_FO_EDIT);
	pED->GetWindowText(m_find_expression);

	//
	// Perform a synax check, and if it is bogus display a message
	// box.
	//
	OrganismFinder ofc(m_find_expression, true);
	if( ofc.error ) {
		str = "Invalid Find Expression: " + ofc.error_message;
		AfxMessageBox(str, MB_OK, 0);
	} else {
		CDialog::OnOK();
	}

}

BOOL FindDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CEdit *pED = (CEdit*) GetDlgItem(IDC_FO_EDIT);
	pED->SetWindowText(m_find_expression);

	return TRUE;
}

void FindDialog::SetFindExpression(CString str)
{
	m_find_expression = str;
}

void FindDialog::GetFindExpression(CString &str)
{
	str = m_find_expression;
}

bool FindDialog::GetResetFlag()
{
	//
	// Eventually allow the user to set this
	// flag using a checkbox.
	//
	return true;
}


BEGIN_MESSAGE_MAP(FindDialog, CDialog)
	ON_COMMAND(IDC_FO_HELP, Help)
	ON_COMMAND(IDC_FO_INSTRUCTIONS, Instructions)
END_MESSAGE_MAP()


// FindDialog message handlers
