#pragma once


class SimulationOptionsDialog : public CDialog
{
	DECLARE_DYNAMIC(SimulationOptionsDialog)

public:
	SimulationOptionsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~SimulationOptionsDialog();

	void SetKfmo(KFORTH_MUTATE_OPTIONS *kfmo);

	bool modified;
	KFORTH_MUTATE_OPTIONS m_kfmo;

	enum { IDD = IDD_SIMULATION_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	KFORTH_MUTATE_OPTIONS m_orig_kfmo;

	void Help();


};
