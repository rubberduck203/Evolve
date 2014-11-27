// EnergyTweakerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "EnergyTweakerDialog.h"


// EnergyTweakerDialog dialog

IMPLEMENT_DYNAMIC(EnergyTweakerDialog, CDialog)
EnergyTweakerDialog::EnergyTweakerDialog(CWnd* pParent /*=NULL*/)
	: CDialog(EnergyTweakerDialog::IDD, pParent)
{
	m_energy = 0;
	m_x = 0;
	m_y = 0;
}

EnergyTweakerDialog::~EnergyTweakerDialog()
{
}

void EnergyTweakerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CommaValuePositive(pDX, IDC_ET_ENERGY, m_energy);
}

void EnergyTweakerDialog::setup(int x, int y, int energy)
{
	m_energy = energy;
	m_x = x;
	m_y = y;
}

int EnergyTweakerDialog::get_energy()
{
	return m_energy;
}

afx_msg BOOL EnergyTweakerDialog::OnInitDialog()
{
	CRect rect;

	CDialog::OnInitDialog();

	GetClientRect(&rect);

	MoveWindow(m_x, m_y, rect.Width(), rect.Height() );

	return TRUE;
}

BEGIN_MESSAGE_MAP(EnergyTweakerDialog, CDialog)
END_MESSAGE_MAP()


// EnergyTweakerDialog message handlers
