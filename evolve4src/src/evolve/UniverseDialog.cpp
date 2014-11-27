// UniverseDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "UniverseDialog.h"


// UniverseDialog dialog

IMPLEMENT_DYNAMIC(UniverseDialog, CDialog)
UniverseDialog::UniverseDialog(CWnd* pParent /*=NULL*/)
	: CDialog(UniverseDialog::IDD, pParent)
{
	m_doc = NULL;
}

UniverseDialog::~UniverseDialog()
{
}

void UniverseDialog::SetDocument(CevolveDoc *doc)
{
	m_doc = doc;
}

void UniverseDialog::set_int(int id, int value)
{
	char buf[1000];

	CStatic *pST = (CStatic*) GetDlgItem(id);

	itoa_comma(buf, value);

	pST->SetWindowText(buf);
}

void UniverseDialog::set_long_long(int id, LONG_LONG value)
{
	char buf[1000];

	CStatic *pST = (CStatic*) GetDlgItem(id);

	lltoa_comma(buf, value);

	pST->SetWindowText(buf);
}

void UniverseDialog::set_ulong(int id, unsigned long value)
{
	char buf[1000];

	CStatic *pST = (CStatic*) GetDlgItem(id);

	sprintf(buf, "%lu", value);

	pST->SetWindowText(buf);
}

void UniverseDialog::set_str(int id, CString& str)
{
	CStatic *pST = (CStatic*) GetDlgItem(id);
	pST->SetWindowText(str);
}

void UniverseDialog::DoDataExchange(CDataExchange* pDX)
{
	UNIVERSE *u;
	UNIVERSE_INFORMATION uinfo;
	CString str;

	CDialog::DoDataExchange(pDX);

	if( m_doc == NULL )
		return;

	if( pDX->m_bSaveAndValidate )
		return;

	u = m_doc->universe;
	Universe_Information(u, &uinfo);

	set_ulong(IDC_U_SEED, u->seed);

	set_long_long(IDC_U_STEPS, u->step);
	set_int(IDC_U_ENERGY, uinfo.energy);
	set_int(IDC_U_ORGANISMS, u->norganism);
	set_int(IDC_U_NSEXUAL, uinfo.num_sexual);

	set_int(IDC_U_CELLS, uinfo.num_cells);
	set_long_long(IDC_U_NBORN, u->nborn);
	set_long_long(IDC_U_NDIE, u->ndie);
	set_int(IDC_U_HEIGHT, u->height);
	set_int(IDC_U_WIDTH, u->width);

	str = m_doc->GetPathName();
	set_str(IDC_U_FILENAME, str);

	set_int(IDC_U_INSTRUCTIONS, uinfo.num_instructions);
	set_int(IDC_U_CSTACK, uinfo.call_stack_nodes);
	set_int(IDC_U_DSTACK, uinfo.data_stack_nodes);
	set_int(IDC_U_STACK, uinfo.call_stack_nodes + uinfo.data_stack_nodes );
	set_int(IDC_U_ORGANIC, uinfo.num_organic);
	set_int(IDC_U_SPORES, uinfo.num_spores);

	set_int(IDC_U_GMEM, uinfo.grid_memory);
	set_int(IDC_U_CSMEM, uinfo.cstack_memory);
	set_int(IDC_U_DSMEM, uinfo.dstack_memory);
	set_int(IDC_U_PMEM, uinfo.program_memory);
	set_int(IDC_U_OMEM, uinfo.organism_memory);
	set_int(IDC_U_SMEM, uinfo.spore_memory);

	set_int(IDC_U_TMEM, uinfo.grid_memory
			+ uinfo.cstack_memory
			+ uinfo.dstack_memory
			+ uinfo.program_memory
			+ uinfo.organism_memory
			+ uinfo.spore_memory);

	set_int(IDC_U_SPORE_ENERGY, uinfo.spore_energy);
	set_int(IDC_U_ORGANIC_ENERGY, uinfo.organic_energy);
}

void UniverseDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\universe_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

BEGIN_MESSAGE_MAP(UniverseDialog, CDialog)
	ON_COMMAND(IDC_U_HELP, Help)
END_MESSAGE_MAP()


// UniverseDialog message handlers
