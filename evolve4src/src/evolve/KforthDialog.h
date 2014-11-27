#pragma once


class KforthDialog : public CDialog
{
	DECLARE_DYNAMIC(KforthDialog)

public:
	KforthDialog(CWnd* pParent = NULL);
	virtual ~KforthDialog();

	enum { IDD = IDD_KFORTH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	KFORTH_PROGRAM		*kfp;
	KFORTH_MACHINE		*kfm;
	KFORTH_OPERATIONS	*kfops;
	KFORTH_DISASSEMBLY	*kfd;

	CString		source_code;
	CString		disassembly;
	CString		error_message;
	CString		m_filename;
	bool		disassembly_updated;

	int		cursor_start;
	int		cursor_end;

	BPLIST		*m_breakpoints;

	void DDX_machine(CDataExchange* pDX);
	void DDX_machine_registers(CDataExchange* pDX);
	void DDX_data_stack(CDataExchange* pDX);
	void DDX_call_stack(CDataExchange* pDX);

	afx_msg void Compile();
	afx_msg void Load();
	afx_msg void Save();
	afx_msg void Run();
	afx_msg void Reset();
	afx_msg void Insert();
	afx_msg void SingleStep();
	afx_msg void StepOver();
	afx_msg void Breakpoint();
	afx_msg void Watchpoint();
	afx_msg void Help();

	CString find_instruction();
	void nice_select(int cpos1, int cpos2);
	void disassemble_program();
	void set_new_position();

	BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG* pMsg);
	void OnOK();
	BOOL CheckAndSave();
	bool breakpoint_reached();
	bool is_call_instruction(KFORTH_MACHINE *kfm, int cb, int pc);
};
