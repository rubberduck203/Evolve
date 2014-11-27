// StrainPopulation.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "StrainPopulation.h"


// StrainPopulation dialog

IMPLEMENT_DYNAMIC(StrainPopulation, CDialog)
StrainPopulation::StrainPopulation(CWnd* pParent /*=NULL*/)
	: CDialog(StrainPopulation::IDD, pParent)
{
	m_doc = NULL;
}

StrainPopulation::~StrainPopulation()
{
}

void StrainPopulation::SetDocument(CevolveDoc *doc)
{
	m_doc = doc;
}

int StrainPopulation::label_id(int i)
{
	ASSERT( i >=0 && i < 8 );

	switch(i) {
	case 0:		return IDC_SP_LABEL1;
	case 1:		return IDC_SP_LABEL2;
	case 2:		return IDC_SP_LABEL3;
	case 3:		return IDC_SP_LABEL4;
	case 4:		return IDC_SP_LABEL5;
	case 5:		return IDC_SP_LABEL6;
	case 6:		return IDC_SP_LABEL7;
	case 7:		return IDC_SP_LABEL8;
	default:
		ASSERT(0);
		return 0;
	}
}

int StrainPopulation::edit_id(int i)
{
	ASSERT( i >=0 && i < 8 );

	switch(i) {
	case 0:		return IDC_SP_EDIT1;
	case 1:		return IDC_SP_EDIT2;
	case 2:		return IDC_SP_EDIT3;
	case 3:		return IDC_SP_EDIT4;
	case 4:		return IDC_SP_EDIT5;
	case 5:		return IDC_SP_EDIT6;
	case 6:		return IDC_SP_EDIT7;
	case 7:		return IDC_SP_EDIT8;
	default:
		ASSERT(0);
		return 0;
	}
}

int StrainPopulation::static_id(int i)
{
	ASSERT( i >=0 && i < 8 );

	switch(i) {
	case 0:		return IDC_SP_STATIC1;
	case 1:		return IDC_SP_STATIC2;
	case 2:		return IDC_SP_STATIC3;
	case 3:		return IDC_SP_STATIC4;
	case 4:		return IDC_SP_STATIC5;
	case 5:		return IDC_SP_STATIC6;
	case 6:		return IDC_SP_STATIC7;
	case 7:		return IDC_SP_STATIC8;
	default:
		ASSERT(0);
		return 0;
	}
}

void StrainPopulation::DoDataExchange(CDataExchange* pDX)
{
	UNIVERSE *u;
	UNIVERSE_INFORMATION uinfo;
	char key[100];
	char value[100];
	int i;
	int pop;

	CDialog::DoDataExchange(pDX);

	if( m_doc == NULL )
		return;

	if( pDX->m_bSaveAndValidate )
		return;

	u = m_doc->universe;

	Universe_Information(u, &uinfo);

	for(i=0; i<8; i++) {
		pop = uinfo.strain_population[i];

		sprintf(key, "HadStrain%d", i);
		Universe_GetAttribute(u, key, value);
		if( stricmp(value, "y") == 0 || pop > 0 ) {
			DDX_CommaValue(pDX, edit_id(i), pop);
		} else {
			CString str;
			str = "";
			DDX_Text(pDX, edit_id(i), str);
		}

		pop = uinfo.radioactive_population[i];
		if( pop > 0 ) {
			DDX_CommaValue(pDX, static_id(i), pop);
		} else {
			CString str;
			str = "";
			DDX_Text(pDX, static_id(i), str);
		}
	}

}

void StrainPopulation::setup_marker(CColorStaticST &m)
{
	m.SetTextColor( RGB(0, 255, 0) );
}

void StrainPopulation::setbg(CColorStaticST &sc, int i)
{
	CBrush *brush;
	LOGBRUSH lb;

	ASSERT( i >= 0 && i < 8 );

	brush = m_appearance.cellBrush(i);
	brush->GetLogBrush( &lb );

	sc.SetBkColor( lb.lbColor );
}

BOOL StrainPopulation::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_strain1.SubclassDlgItem(IDC_SP_LABEL1, this);
	m_strain2.SubclassDlgItem(IDC_SP_LABEL2, this);
	m_strain3.SubclassDlgItem(IDC_SP_LABEL3, this);
	m_strain4.SubclassDlgItem(IDC_SP_LABEL4, this);
	m_strain5.SubclassDlgItem(IDC_SP_LABEL5, this);
	m_strain6.SubclassDlgItem(IDC_SP_LABEL6, this);
	m_strain7.SubclassDlgItem(IDC_SP_LABEL7, this);
	m_strain8.SubclassDlgItem(IDC_SP_LABEL8, this);

	m_marker1.SubclassDlgItem(IDC_SP_STATIC1, this);
	m_marker2.SubclassDlgItem(IDC_SP_STATIC2, this);
	m_marker3.SubclassDlgItem(IDC_SP_STATIC3, this);
	m_marker4.SubclassDlgItem(IDC_SP_STATIC4, this);
	m_marker5.SubclassDlgItem(IDC_SP_STATIC5, this);
	m_marker6.SubclassDlgItem(IDC_SP_STATIC6, this);
	m_marker7.SubclassDlgItem(IDC_SP_STATIC7, this);
	m_marker8.SubclassDlgItem(IDC_SP_STATIC8, this);

	setup_marker(m_marker1);
	setup_marker(m_marker2);
	setup_marker(m_marker3);
	setup_marker(m_marker4);
	setup_marker(m_marker5);
	setup_marker(m_marker6);
	setup_marker(m_marker7);
	setup_marker(m_marker8);

	setbg(m_strain1, 0);
	setbg(m_strain2, 1);
	setbg(m_strain3, 2);
	setbg(m_strain4, 3);
	setbg(m_strain5, 4);
	setbg(m_strain6, 5);
	setbg(m_strain7, 6);
	setbg(m_strain8, 7);

	return TRUE;
}

void StrainPopulation::OnHelp()
{
	CString str;
	CWinApp *app;

	//
	// Launch web browser, and display help for this
	// dialog.
	//
	app = AfxGetApp();
	str = app->GetProfileString("help", "path");

	str = str + "\\strain_population_dialog.html";

	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

BEGIN_MESSAGE_MAP(StrainPopulation, CDialog)
	ON_COMMAND(IDC_SP_HELP, OnHelp)
END_MESSAGE_MAP()


// StrainPopulation message handlers
