// MainFrm.h : interface of the CMainFrame class
//
#pragma once

//
// Right Click Action
//
typedef enum {
	RCA_NOTHING,
	RCA_BARRIER,
	RCA_THIN_BARRIER,
	RCA_DYE,
	RCA_MOVE,
	RCA_ENERGY,
} RCA_TYPE;


class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	void set_status(const char *status, int organisms,
			LONG_LONG born, LONG_LONG die, LONG_LONG steps);

	void set_status_cursor(int x, int y);
	void set_status_cursor2(int x, int y, int energy);

	void set_rca(RCA_TYPE type);
	RCA_TYPE get_rca();

	void on_right_click_choose();

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStatusBar m_wndStatusBar;
	CToolBar m_wndToolBar;
	CMenu m_popup;

	RCA_TYPE	m_rca;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
};


