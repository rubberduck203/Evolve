#pragma once


// NewUniverseDialog dialog

class NewUniverseDialog : public CDialog
{
	DECLARE_DYNAMIC(NewUniverseDialog)

public:
	enum {
		IDD = IDD_NEW_UNIVERSE
	};

	NewUniverseDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~NewUniverseDialog();

	NewUniverseOptions nuo;

	CString	error;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	Appearance m_appearance;

	void OnOK();
	void OnCancel();
	void OnHelp();
	void OnMutationSettings();
	void OnBrowse();
	void OnBrowse2();
	void OnBrowse3();
	void OnBrowse4();
	void OnBrowse5();
	void OnBrowse6();
	void OnBrowse7();
	void OnBrowse8();

	BOOL OnInitDialog();

	void browse_any(int i);
	int energy_id(int i);
	int program_id(int i);

	void setbg(CColorStaticST &sc, int i);

	CColorStaticST m_strain1;
	CColorStaticST m_strain2;
	CColorStaticST m_strain3;
	CColorStaticST m_strain4;
	CColorStaticST m_strain5;
	CColorStaticST m_strain6;
	CColorStaticST m_strain7;
	CColorStaticST m_strain8;

};

