// NewUniverseDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "NewUniverseDialog.h"


// NewUniverseDialog dialog

IMPLEMENT_DYNAMIC(NewUniverseDialog, CDialog)

NewUniverseDialog::NewUniverseDialog(CWnd* pParent /*=NULL*/)
	: CDialog(NewUniverseDialog::IDD, pParent)
{
	error = "";
}

NewUniverseDialog::~NewUniverseDialog()
{
}

int NewUniverseDialog::energy_id(int i)
{
	ASSERT( i >= 0 && i < 8 );

	switch(i) {
	case 0:		return IDC_NU_ENERGY;
	case 1:		return IDC_NU_ENERGY2;
	case 2:		return IDC_NU_ENERGY3;
	case 3:		return IDC_NU_ENERGY4;
	case 4:		return IDC_NU_ENERGY5;
	case 5:		return IDC_NU_ENERGY6;
	case 6:		return IDC_NU_ENERGY7;
	case 7:		return IDC_NU_ENERGY8;
	default:
		ASSERT(0);
		return 0;
	}
}

int NewUniverseDialog::program_id(int i)
{
	ASSERT( i >= 0 && i < 8 );

	switch(i) {
	case 0:		return IDC_NU_PROGRAM;
	case 1:		return IDC_NU_PROGRAM2;
	case 2:		return IDC_NU_PROGRAM3;
	case 3:		return IDC_NU_PROGRAM4;
	case 4:		return IDC_NU_PROGRAM5;
	case 5:		return IDC_NU_PROGRAM6;
	case 6:		return IDC_NU_PROGRAM7;
	case 7:		return IDC_NU_PROGRAM8;
	default:
		ASSERT(0);
		return 0;
	}
}

void NewUniverseDialog::DoDataExchange(CDataExchange* pDX)
{
	int i;

	CDialog::DoDataExchange(pDX);

	DDX_CommaValue(pDX, IDC_NU_SEED, nuo.seed);
	DDX_CommaValue(pDX, IDC_NU_WIDTH, nuo.width);
	DDX_CommaValue(pDX, IDC_NU_HEIGHT, nuo.height);

	for(i=0; i<8; i++) {
		DDX_CommaValue(pDX, energy_id(i), nuo.energy[i]);

		DDX_ProgramFilename(pDX, program_id(i),
			nuo.sexonly[i], nuo.population[i], nuo.filename[i]);
	}

	DDX_Check(pDX, IDC_NU_BARRIER, nuo.want_barrier);
	DDX_Text(pDX, IDC_NU_ERROR, error);
}

void NewUniverseDialog::OnOK()
{
	char buf[ 5000 ];
	int i, count;
	int flen, eval;
	FILE *fp;

	UpdateData(true);

	if( nuo.width < EVOLVE_MIN_BOUNDS || nuo.width > EVOLVE_MAX_BOUNDS ) {
		sprintf(buf, "Width must be between %d and %d",
				EVOLVE_MIN_BOUNDS, EVOLVE_MAX_BOUNDS);
		error = buf;
		UpdateData(false);
		return;
	}

	if( nuo.height < EVOLVE_MIN_BOUNDS || nuo.height > EVOLVE_MAX_BOUNDS ) {
		sprintf(buf, "Height must be between %d and %d",
				EVOLVE_MIN_BOUNDS, EVOLVE_MAX_BOUNDS);
		error = buf;
		UpdateData(false);
		return;
	}

	if( nuo.seed < 0 ) {
		sprintf(buf, "Seed must be positive");
		error = buf;
		UpdateData(false);
		return;
	}

	/*
	 * Check each energy/filename control for data, and validate it
	 */

	count = 0;
	for(i=0; i<8; i++) {
		nuo.has_strain[i] = FALSE;

		if( StringIsBlank(nuo.filename[i]) )
			flen = 0;
		else
			flen = nuo.filename[i].GetLength();

		eval = nuo.energy[i];

		if( flen == 0 && eval == 0 ) {
			continue;
		}

		if( flen > 0 && eval == 0 ) {
			sprintf(buf, "Strain %d, energy value must be greater than 0.", i+1);
			error = buf;
			UpdateData(false);
			return;
		}

		if( flen == 0 && eval > 0 ) {
			sprintf(buf, "Strain %d, filename is blank.", i+1);
			error = buf;
			UpdateData(false);
			return;
		}

		if( nuo.energy[i] < 1 ) {
			sprintf(buf, "Strain %d, energy must be 1 or more", i+1);
			error = buf;
			UpdateData(false);
			return;
		}

		fp = fopen(nuo.filename[i], "r");
		if( fp == NULL ) {
			sprintf(buf, "Strain %d, File \"%s\" %s", i+1, nuo.filename[i], strerror(errno));
			error = buf;
			UpdateData(false);
			return;
		}
		fclose(fp);

		nuo.has_strain[i] = TRUE;
		count += 1;
	}

	if( count == 0 ) {
		sprintf(buf, "Please specify some starting organisms.");
		error = buf;
		UpdateData(false);
		return;
	}

	nuo.num_strains = count;

	error = "";
	UpdateData(false);

	CDialog::OnOK();
}

void NewUniverseDialog::OnCancel()
{
	error = "";
	CDialog::OnCancel();
}

void NewUniverseDialog::OnMutationSettings()
{
	SimulationOptionsDialog dlg;
	INT_PTR nRet;

	dlg.SetKfmo(&nuo.kfmo);
	nRet = dlg.DoModal();
	if( nRet == IDOK && dlg.modified ) {
		nuo.kfmo = dlg.m_kfmo;
	}
}

void NewUniverseDialog::OnHelp()
{
	CString str;
	CWinApp *app;

	//
	// Launch web browser, and display help for this
	// dialog.
	//
	app = AfxGetApp();
	str = app->GetProfileString("help", "path");

	str = str + "\\new_universe_dialog.html";

	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

void NewUniverseDialog::browse_any(int i)
{
	char filters[] = "KFORTH Files (*.kf)|*.kf|All Files (*.*)|*.*||";
	INT_PTR result;

	ASSERT( i >= 0 && i < 8 );

	UpdateData(true);

	CFileDialog fileDlg(true, "kf", "*.kf",
		OFN_FILEMUSTEXIST, filters, this);

	result = fileDlg.DoModal();
	if( result == IDOK ) {
		nuo.filename[i] = fileDlg.GetPathName();
		UpdateData(false);
	}
}

void NewUniverseDialog::OnBrowse()
{
	browse_any(0);
}

void NewUniverseDialog::OnBrowse2()
{
	browse_any(1);
}

void NewUniverseDialog::OnBrowse3()
{
	browse_any(2);
}

void NewUniverseDialog::OnBrowse4()
{
	browse_any(3);
}

void NewUniverseDialog::OnBrowse5()
{
	browse_any(4);
}

void NewUniverseDialog::OnBrowse6()
{
	browse_any(5);
}

void NewUniverseDialog::OnBrowse7()
{
	browse_any(6);
}

void NewUniverseDialog::OnBrowse8()
{
	browse_any(7);
}

void NewUniverseDialog::setbg(CColorStaticST &sc, int i)
{
	CBrush *brush;
	LOGBRUSH lb;

	ASSERT( i >= 0 && i < 8 );

	brush = m_appearance.cellBrush(i);
	brush->GetLogBrush( &lb );

	sc.SetBkColor( lb.lbColor );
}


BOOL NewUniverseDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_strain1.SubclassDlgItem(IDC_NU_STRAIN, this);
	m_strain2.SubclassDlgItem(IDC_NU_STRAIN2, this);
	m_strain3.SubclassDlgItem(IDC_NU_STRAIN3, this);
	m_strain4.SubclassDlgItem(IDC_NU_STRAIN4, this);
	m_strain5.SubclassDlgItem(IDC_NU_STRAIN5, this);
	m_strain6.SubclassDlgItem(IDC_NU_STRAIN6, this);
	m_strain7.SubclassDlgItem(IDC_NU_STRAIN7, this);
	m_strain8.SubclassDlgItem(IDC_NU_STRAIN8, this);

	setbg(m_strain1, 0);
	setbg(m_strain2, 1);
	setbg(m_strain3, 2);
	setbg(m_strain4, 3);
	setbg(m_strain5, 4);
	setbg(m_strain6, 5);
	setbg(m_strain7, 6);
	setbg(m_strain8, 7);

	nuo.seed = (long) GetTickCount();

	return TRUE;
}

BEGIN_MESSAGE_MAP(NewUniverseDialog, CDialog)
	ON_COMMAND(IDC_NU_HELP, OnHelp)
	ON_COMMAND(IDC_NU_MUTATION_SETTINGS, OnMutationSettings)
	ON_COMMAND(IDC_NU_BROWSE, OnBrowse)
	ON_COMMAND(IDC_NU_BROWSE2, OnBrowse2)
	ON_COMMAND(IDC_NU_BROWSE3, OnBrowse3)
	ON_COMMAND(IDC_NU_BROWSE4, OnBrowse4)
	ON_COMMAND(IDC_NU_BROWSE5, OnBrowse5)
	ON_COMMAND(IDC_NU_BROWSE6, OnBrowse6)
	ON_COMMAND(IDC_NU_BROWSE7, OnBrowse7)
	ON_COMMAND(IDC_NU_BROWSE8, OnBrowse8)
END_MESSAGE_MAP()



