// AboutDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "AboutDialog.h"


// AboutDialog dialog
AboutDlg::AboutDlg() : CDialog(AboutDlg::IDD)
{
}

void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void AboutDlg::Help()
{
	CString str;

	str = "http://www.stauffercom.com/evolve4";

	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
	EndDialog(IDOK);
}

afx_msg BOOL AboutDlg::OnInitDialog()
{
	CStatic *pST;
	char buf[1000];
	char modes[100];

	CDialog::OnInitDialog();

	pST = (CStatic*) GetDlgItem(IDC_AB_INFO);

	strcpy(modes, " ");

#ifndef NDEBUG
	strcat(modes, "DEBUG ");
#else
	strcat(modes, "RELEASE ");
#endif

	sprintf(buf, "%s  %s    (%s)", __DATE__, __TIME__, modes);

	pST->SetWindowText(buf);

	return TRUE;

}

BEGIN_MESSAGE_MAP(AboutDlg, CDialog)
	ON_COMMAND(IDC_AB_HELP, Help)
END_MESSAGE_MAP()

// AboutDialog message handlers
