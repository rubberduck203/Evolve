// EvolveOptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "EvolveOptionsDialog.h"


IMPLEMENT_DYNAMIC(EvolveOptionsDialog, CDialog)

EvolveOptionsDialog::EvolveOptionsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(EvolveOptionsDialog::IDD, pParent)
{
	MapUpdateSteps = 50;
	EnableCheckpoint = 0;
	CheckpointInterval = 100000000;
	CheckpointFilename = "";
}

EvolveOptionsDialog::~EvolveOptionsDialog()
{
}

void EvolveOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_CommaValue(pDX, IDC_EO_MAP_UPDATE, MapUpdateSteps);
	DDX_Check(pDX, IDC_EO_ENABLE, EnableCheckpoint);
	DDX_Text(pDX, IDC_EO_FILENAME, CheckpointFilename);
	DDX_CommaValue(pDX, IDC_EO_CHECKPOINT_INTERVAL, CheckpointInterval);
}

void EvolveOptionsDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\evolve_options_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void EvolveOptionsDialog::Browse()
{
}

void EvolveOptionsDialog::Macros()
{
}

void EvolveOptionsDialog::Defaults()
{
	MapUpdateSteps = 50;
	EnableCheckpoint = 0;
	CheckpointInterval = 100000000;
	CheckpointFilename = "checkpoint-${date}.evolve";
	UpdateData(false);
}

BEGIN_MESSAGE_MAP(EvolveOptionsDialog, CDialog)
	ON_COMMAND(IDC_EO_HELP, Help)
	ON_COMMAND(IDC_EO_BROWSE, Browse)
	ON_COMMAND(IDC_EO_MACROS, Macros)
	ON_COMMAND(IDC_EO_DEFAULTS, Defaults)
END_MESSAGE_MAP()
