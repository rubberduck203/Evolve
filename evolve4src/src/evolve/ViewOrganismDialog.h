#pragma once


class ViewOrganismDialog : public CDialog
{
	DECLARE_DYNAMIC(ViewOrganismDialog)

public:
	ViewOrganismDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~ViewOrganismDialog();

	enum { IDD = IDD_VIEW_ORGANISM };

	void SetOrganism(ORGANISM *o);

	int SimulateSteps;

private:
	void Help();
	void Vision();
	void Insert();
	void Simulate();
	void StepOver();
	void Breakpoint();
	void Watchpoint();
	void Run();
	void set_new_position();
	void disassemble_program();
	void DDX_Cell(CDataExchange* pDX);
	void DDX_machine_registers(CDataExchange* pDX);
	void DDX_data_stack(CDataExchange* pDX);
	void DDX_call_stack(CDataExchange* pDX);
	void draw_organism();
	void draw_cell(CDC *pDC, CELL *cell, bool active, int idx, TF_RECT& ugwin);
	void repaint_organism();
	void make_transform();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG* pMsg);

	bool breakpoint_reached();
	bool is_call_instruction(KFORTH_MACHINE *kfm, int cb, int pc);
	CString find_instruction();
	void nice_select(int cpos1, int cpos2);

	BPLIST *m_breakpoints;

	ORGANISM *organism;
	CELL *cell;

	KFORTH_DISASSEMBLY *kfd;
	CString disassembly;
	bool disassembly_updated;
	int cursor_start;
	int cursor_end;

	Appearance m_appearance;

	RECT	win_rect;
	TF_TRANSFORM *tf;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
