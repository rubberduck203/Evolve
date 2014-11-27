#pragma once


// EnergyTweakerDialog dialog

class EnergyTweakerDialog : public CDialog
{
	DECLARE_DYNAMIC(EnergyTweakerDialog)

public:
	EnergyTweakerDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~EnergyTweakerDialog();
	afx_msg BOOL OnInitDialog();

	void setup(int x, int y, int energy);
	int get_energy();

// Dialog Data
	enum { IDD = IDD_ENERGY_TWEAKER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	int	m_energy;
	int	m_x;
	int	m_y;
};
