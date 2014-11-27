//
// evolveView.h : interface of the CevolveView class
//

#pragma once

class CevolveView : public CView
{
protected:
	CevolveView();
	virtual ~CevolveView();

	DECLARE_DYNCREATE(CevolveView)

public:
	CevolveDoc* GetDocument() const;
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

	void OnViewUniverse();
	void OnViewOrganism();
	void OnView3D();
	void OnViewStrainPopulation();
	void OnSimulatorProperties();

	void OnRightClickChoose();
	void OnRightClickNothing();
	void OnRightClickBarrier();
	void OnRightClickThinBarrier();
	void OnRightClickDye();
	void OnRightClickMove();
	void OnRightClickEnergy();
	void OnClearDye();

	void OnCut();
	void OnCopy();
	void OnPaste();
	void OnDelete();
	void OnFind();

	void OnSimulationStart();
	void OnSimulationStop();
	void OnSimulationSingleStep();
	void OnSimulationContinue();
	void OnZoomIn();
	void OnZoomOut();
	void OnViewAll();

protected:
	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	void draw_select_box(CDC* pDC, int x, int y, int dist);
	void draw_selection(CDC* pDC, UNIVERSE *u, ORGANISM *o);
	void draw_element(CDC* pDC, GRID_TYPE type, UNIVERSE_GRID& ugrid, TF_RECT& ugwin);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL OnEraseBkgnd(CDC* pDC);

	void update_statusbar();
	void update_status_cursor(CPoint &point);
	void create_barrier_grid(CDC *dc, int x, int y);
	void create_barrier_grids(CDC *dc, int x, int y);
	void create_barrier();
	void move_organism();
	void draw_organism_outline();
	void set_rca(RCA_TYPE type);
	RCA_TYPE get_rca();
	void OnRButtonDown_Barrier(UINT nFlags, CPoint point);
	void OnRButtonDown_Dye(UINT nFlags, CPoint point);
	void OnRButtonDown_Move(UINT nFlags, CPoint point);
	void OnRButtonDown_Energy(UINT nFlags, CPoint point);

	bool m_drawingBarrier;
	bool m_barrierOn;
	bool m_stopped;
	bool m_panning;
	bool m_movingOrganism;
	CPoint m_Point0;
	CPoint m_Point1;
	CPoint m_Point2;

	ZoomHistory m_zh;

	Appearance m_appearance;

	int	m_lastx;
	int	m_lasty;

	CString	m_find_expression;

	CELL	*m_move_cell;

};

#ifndef _DEBUG  // debug version in evolveView.cpp
inline CevolveDoc* CevolveView::GetDocument() const
   { return reinterpret_cast<CevolveDoc*>(m_pDocument); }
#endif

