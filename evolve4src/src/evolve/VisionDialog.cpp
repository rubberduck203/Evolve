// VisionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "VisionDialog.h"


// VisionDialog dialog

IMPLEMENT_DYNAMIC(VisionDialog, CDialog)
VisionDialog::VisionDialog(CWnd* pParent /*=NULL*/)
	: CDialog(VisionDialog::IDD, pParent)
{
	m_cell = NULL;
}

VisionDialog::~VisionDialog()
{
}

void VisionDialog::setbg(CColorStaticST &sc, int i)
{
	CBrush *brush;
	LOGBRUSH lb;

	ASSERT( i >= 0 && i < 8 );

	brush = m_appearance.cellBrush(i);
	brush->GetLogBrush( &lb );

	sc.SetBkColor( lb.lbColor );
}

BOOL VisionDialog::OnInitDialog()
{
	m_cell_pic.SubclassDlgItem(IDC_CV_CELL, this);

	CDialog::OnInitDialog();

	return TRUE;
}


void VisionDialog::DoDataExchange(CDataExchange* pDX)
{
	CELL_VISION_DATA cvd;

	CDialog::DoDataExchange(pDX);

	if( m_cell == NULL )
		return;

	if( pDX->m_bSaveAndValidate )
		return;

	set_cell(IDC_CV_CELL);

	Universe_CellVisionData(m_cell, &cvd);

	set_where(IDC_CV_N_WHERE,	cvd.where[0]);
	set_what( IDC_CV_N_WHAT,	cvd.what[0]);

	set_where(IDC_CV_NE_WHERE,	cvd.where[1]);
	set_what( IDC_CV_NE_WHAT,	cvd.what[1]);

	set_where(IDC_CV_E_WHERE,	cvd.where[2]);
	set_what( IDC_CV_E_WHAT,	cvd.what[2]);

	set_where(IDC_CV_SE_WHERE,	cvd.where[3]);
	set_what( IDC_CV_SE_WHAT,	cvd.what[3]);

	set_where(IDC_CV_S_WHERE,	cvd.where[4]);
	set_what( IDC_CV_S_WHAT,	cvd.what[4]);

	set_where(IDC_CV_SW_WHERE,	cvd.where[5]);
	set_what( IDC_CV_SW_WHAT,	cvd.what[5]);

	set_where(IDC_CV_W_WHERE,	cvd.where[6]);
	set_what( IDC_CV_W_WHAT,	cvd.what[6]);

	set_where(IDC_CV_NW_WHERE,	cvd.where[7]);
	set_what( IDC_CV_NW_WHAT,	cvd.what[7]);

}

void VisionDialog::set_cell(int id)
{
	char buf[100];
	int idx;
	ORGANISM *o;
	CELL *c;

	idx = 1;
	o = m_cell->organism;
	for(c=o->cells; c; c=c->next) {
		if( c == m_cell )
			break;
		idx += 1;
	}

	sprintf(buf, "%d", idx);
	m_cell_pic.SetWindowText(buf);

	setbg(m_cell_pic, m_cell->organism->strain);

}

void VisionDialog::set_where(int id, int value)
{
	char buf[1000];

	CStatic *pST = (CStatic*) GetDlgItem(id);

	itoa_comma(buf, value);

	pST->SetWindowText(buf);
}

void VisionDialog::set_what(int id, int value)
{
	char *str;

	CStatic *pST = (CStatic*) GetDlgItem(id);

	switch(value) {
	case 0:
		str = "Nothing";
		break;

	case 1:
		str = "Cell";
		break;

	case 2:
		str = "Spore";
		break;

	case 4:
		str = "Organic";
		break;

	case 8:
		str = "Barrier";
		break;

	default:
		ASSERT(0);
	}

	pST->SetWindowText(str);
}

void VisionDialog::SetCell(CELL *c)
{
	m_cell = c;
}

void VisionDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\vision_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);

}

BEGIN_MESSAGE_MAP(VisionDialog, CDialog)
	ON_COMMAND(IDC_CV_HELP, Help)
END_MESSAGE_MAP()


// VisionDialog message handlers
