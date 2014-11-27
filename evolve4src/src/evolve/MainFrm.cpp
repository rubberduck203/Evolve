// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "evolve.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_INDICATOR_SCRL,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_rca = RCA_NOTHING;
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		 !m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	//
	// set up status bar
	//
	m_wndStatusBar.SetPaneInfo(0, 1, SBPS_NORMAL, 200);
	m_wndStatusBar.SetPaneInfo(1, 2, SBPS_NORMAL, 150);
	m_wndStatusBar.SetPaneInfo(2, 3, SBPS_NORMAL, 130);
	m_wndStatusBar.SetPaneInfo(3, 3, SBPS_NORMAL, 130);
	m_wndStatusBar.SetPaneInfo(4, 4, SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(5, 5, SBPS_NORMAL, 150);

	set_status("", 9999, 999999999, 999999999, 9999999);

	//
	// Create pop-menu object
	//
	VERIFY(m_popup.LoadMenu(IDR_RIGHTCLICK_POPUP));

	return 0;
}

void CMainFrame::set_status_cursor(int x, int y)
{
	CMDIChildWnd *child;
	char buf[100];

	child = MDIGetActive();

	if( child ) {
		sprintf(buf, "(%d, %d)", x, y);
		m_wndStatusBar.SetPaneText(5, buf);
	} else {
		m_wndStatusBar.SetPaneText(5, "");
	}
}

void CMainFrame::set_status_cursor2(int x, int y, int energy)
{
	CMDIChildWnd *child;
	char buf[100];

	child = MDIGetActive();

	if( child ) {
		sprintf(buf, "(%d, %d)  energy: %d", x, y, energy);
		m_wndStatusBar.SetPaneText(5, buf);
	} else {
		m_wndStatusBar.SetPaneText(5, "");
	}
}

void CMainFrame::set_status(const char *status,
			int organisms, LONG_LONG born, LONG_LONG die, LONG_LONG steps)
{
	CMDIChildWnd *child;
	char buf[100];
	char buf1[100];

	ASSERT( status != NULL );

	child = MDIGetActive();

	if( child ) {
		sprintf(buf, "Status: %s", status);
		m_wndStatusBar.SetPaneText(0, buf);

		itoa_comma(buf1, organisms);
		sprintf(buf, "Organisms: %s", buf1);
		m_wndStatusBar.SetPaneText(1, buf);

		lltoa_comma(buf1, born);
		sprintf(buf, "Born: %s", buf1);
		m_wndStatusBar.SetPaneText(2, buf);

		lltoa_comma(buf1, die);
		sprintf(buf, "Die: %s", buf1);
		m_wndStatusBar.SetPaneText(3, buf);

		lltoa_comma(buf1, steps);
		sprintf(buf, "Steps: %s", buf1);
		m_wndStatusBar.SetPaneText(4, buf);

	} else {
		m_wndStatusBar.SetPaneText(0, "No model loaded.");
		m_wndStatusBar.SetPaneText(1, "");
		m_wndStatusBar.SetPaneText(2, "");
		m_wndStatusBar.SetPaneText(3, "");
		m_wndStatusBar.SetPaneText(4, "");
		m_wndStatusBar.SetPaneText(5, "");
	}

}

//
// Set the right click action to 'type'
// set the check mark in the menu to
// the correct item.
//
void CMainFrame::set_rca(RCA_TYPE type)
{
	CWnd *pMain = AfxGetMainWnd();
	CMenu *menu = pMain->GetMenu();

	m_rca = type;

	//
	// clear all check marks
	//
	menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_NOTHING, MF_UNCHECKED);
	menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_BARRIER, MF_UNCHECKED);
	menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_THIN_BARRIER, MF_UNCHECKED);
	menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_DYE, MF_UNCHECKED);
	menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_MOVE, MF_UNCHECKED);
	menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_ENERGY, MF_UNCHECKED);

	//
	// Clear all check marks for popup menu too
	//
	CMenu* popup = m_popup.GetSubMenu(0);

	popup->CheckMenuItem(0, MF_BYPOSITION|MF_UNCHECKED);
	popup->CheckMenuItem(2, MF_BYPOSITION|MF_UNCHECKED);
	popup->CheckMenuItem(3, MF_BYPOSITION|MF_UNCHECKED);
	popup->CheckMenuItem(4, MF_BYPOSITION|MF_UNCHECKED);
	popup->CheckMenuItem(5, MF_BYPOSITION|MF_UNCHECKED);
	popup->CheckMenuItem(6, MF_BYPOSITION|MF_UNCHECKED);

	//
	// set the check mark for this rca type
	//
	switch( m_rca ) {
	case RCA_NOTHING:
		menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_NOTHING, MF_CHECKED);
		popup->CheckMenuItem(0, MF_BYPOSITION|MF_CHECKED);
		break;

	case RCA_BARRIER:
		menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_BARRIER, MF_CHECKED);
		popup->CheckMenuItem(2, MF_BYPOSITION|MF_CHECKED);
		break;

	case RCA_THIN_BARRIER:
		menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_THIN_BARRIER, MF_CHECKED);
		popup->CheckMenuItem(3, MF_BYPOSITION|MF_CHECKED);
		break;

	case RCA_DYE:
		menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_DYE, MF_CHECKED);
		popup->CheckMenuItem(4, MF_BYPOSITION|MF_CHECKED);
		break;

	case RCA_MOVE:
		menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_MOVE, MF_CHECKED);
		popup->CheckMenuItem(5, MF_BYPOSITION|MF_CHECKED);
		break;

	case RCA_ENERGY:
		menu->CheckMenuItem(ID_EDIT_RIGHTCLICK_ENERGY, MF_CHECKED);
		popup->CheckMenuItem(6, MF_BYPOSITION|MF_CHECKED);
		break;

	default:
		ASSERT(0);
	}

}

RCA_TYPE CMainFrame::get_rca()
{
	return m_rca;
}

void CMainFrame::on_right_click_choose()
{
	RECT tmp;
	RECT wr;
	int x, y;

	m_wndToolBar.GetWindowRect(&wr);
	m_wndToolBar.GetItemRect(18, &tmp);

	x = wr.left + tmp.left + 10;
	y = wr.top + tmp.bottom + 4;

	//
	// Activiate the popup menu
	//
	CMenu* popup = m_popup.GetSubMenu(0);
	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, this);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

