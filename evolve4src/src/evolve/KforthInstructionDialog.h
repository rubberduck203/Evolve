#pragma once

//
// KforthInstructionDialog dialog
//
class KforthInstructionDialog : public CDialog
{
	DECLARE_DYNAMIC(KforthInstructionDialog)

public:
	KforthInstructionDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~KforthInstructionDialog();

	enum {
		IS_INTERPRETER,
		IS_CELL,
		IS_FIND,
	};

	void show_instruction_set(int iset);
	void insert_button(bool flag);
	void go_to(CString str);

	CString GetInstruction();

	// Dialog Data
	enum { IDD = IDD_KFORTH_INSTRUCTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	BOOL OnInitDialog();
	void LoadList();
	void InsertInstruction();
	void SelectInstruction();

	CString		m_description;
	int		m_item;
	int		m_lbitem;
	int		m_mask;
	int		m_insert_button;
	CString		m_goto;
};
