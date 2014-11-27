#pragma once


//
// WatchpointDialog dialog
//

class WatchpointDialog : public CDialog
{
	DECLARE_DYNAMIC(WatchpointDialog)

public:
	WatchpointDialog(CWnd* pParent = NULL);
	virtual ~WatchpointDialog();

	enum { IDD = IDD_WATCHPOINTS };

	void debugging_cell(BOOL f);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	afx_msg void OnHelp();
	afx_msg void OnRemove();
	afx_msg void OnDisable();
	afx_msg void OnInstructions();
	void OnOK();
	BOOL OnInitDialog();

	BOOL	m_debugging_cell;
	
};
