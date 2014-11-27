// SimulationOptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "SimulationOptionsDialog.h"


// SimulationOptionsDialog dialog

IMPLEMENT_DYNAMIC(SimulationOptionsDialog, CDialog)
SimulationOptionsDialog::SimulationOptionsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(SimulationOptionsDialog::IDD, pParent)
{
}

SimulationOptionsDialog::~SimulationOptionsDialog()
{
}

void SimulationOptionsDialog::SetKfmo(KFORTH_MUTATE_OPTIONS *kfmo)
{
	ASSERT( kfmo != NULL );

	m_kfmo		= *kfmo;
	modified	= false;
	m_orig_kfmo	= m_kfmo;
}

void SimulationOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_CommaValuePositive(pDX, IDC_SO_MAX_CB, m_kfmo.max_code_blocks);
	DDX_RangeValue(pDX, IDC_SO_MAX_APPLY, m_kfmo.max_apply, 0, MUTATE_MAX_APPLY_LIMIT);
	DDX_MutationRate(pDX, IDC_SO_MUTATE_CB, m_kfmo.prob_mutate_codeblock);
	DDX_MutationRate(pDX, IDC_SO_DUPLICATE, m_kfmo.prob_duplicate);
	DDX_MutationRate(pDX, IDC_SO_DELETE, m_kfmo.prob_delete);
	DDX_MutationRate(pDX, IDC_SO_INSERT, m_kfmo.prob_insert);
	DDX_MutationRate(pDX, IDC_SO_TRANSPOSITION, m_kfmo.prob_transpose);
	DDX_MutationRate(pDX, IDC_SO_MODIFICATION, m_kfmo.prob_modify);

	if( m_kfmo.max_code_blocks != m_orig_kfmo.max_code_blocks )
		modified = true;

	if( m_kfmo.max_apply != m_orig_kfmo.max_apply )
		modified = true;

	if( m_kfmo.prob_mutate_codeblock != m_orig_kfmo.prob_mutate_codeblock )
		modified = true;

	if( m_kfmo.prob_duplicate != m_orig_kfmo.prob_duplicate )
		modified = true;

	if( m_kfmo.prob_delete != m_orig_kfmo.prob_delete )
		modified = true;

	if( m_kfmo.prob_insert != m_orig_kfmo.prob_insert )
		modified = true;

	if( m_kfmo.prob_transpose != m_orig_kfmo.prob_transpose )
		modified = true;

	if( m_kfmo.prob_modify != m_orig_kfmo.prob_modify )
		modified = true;

}

void SimulationOptionsDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\simulation_options_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

BEGIN_MESSAGE_MAP(SimulationOptionsDialog, CDialog)
	ON_COMMAND(IDC_SO_HELP, Help)
END_MESSAGE_MAP()


// SimulationOptionsDialog message handlers
