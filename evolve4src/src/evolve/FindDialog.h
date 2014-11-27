#pragma once


// FindDialog dialog

class FindDialog : public CDialog
{
	DECLARE_DYNAMIC(FindDialog)

public:
	FindDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~FindDialog();

	void SetFindExpression(CString str);
	void GetFindExpression(CString &str);
	bool GetResetFlag();

// Dialog Data
	enum { IDD = IDD_FIND };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void Help();
	void Instructions();
	void OnOK();
	BOOL OnInitDialog();

	CString m_find_expression;
	CString m_last_instruction;

};
