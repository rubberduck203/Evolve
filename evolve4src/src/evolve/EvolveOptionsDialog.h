#pragma once


// EvolveOptionsDialog dialog

class EvolveOptionsDialog : public CDialog
{
	DECLARE_DYNAMIC(EvolveOptionsDialog)

public:
	EvolveOptionsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~EvolveOptionsDialog();

	enum { IDD = IDD_EVOLVE_OPTIONS };

	int	MapUpdateSteps;
	int	EnableCheckpoint;
	CString	CheckpointFilename;
	int	CheckpointInterval;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void Help();
	void Browse();
	void Macros();
	void Defaults();
};
