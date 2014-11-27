//
// evolveView.cpp : implementation of the CevolveView class
//

#include "stdafx.h"
#include "evolve.h"

#include "evolveDoc.h"
#include "evolveView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CevolveView, CView)

BEGIN_MESSAGE_MAP(CevolveView, CView)
	ON_COMMAND(ID_VIEW_UNIVERSE, OnViewUniverse)
	ON_COMMAND(ID_VIEW_ORGANISM, OnViewOrganism)
	ON_COMMAND(ID_VIEW_3D, OnView3D)
	ON_COMMAND(ID_VIEW_STRAIN_POPULATION, OnViewStrainPopulation)
	ON_COMMAND(ID_EDIT_SIM_PROPERTIES, OnSimulatorProperties)

	ON_COMMAND(ID_EDIT_RIGHTCLICK_CHOOSER, OnRightClickChoose)

	ON_COMMAND(ID_EDIT_RIGHTCLICK_NOTHING, OnRightClickNothing)
	ON_COMMAND(ID_EDIT_RIGHTCLICK_BARRIER, OnRightClickBarrier)
	ON_COMMAND(ID_EDIT_RIGHTCLICK_THIN_BARRIER, OnRightClickThinBarrier)
	ON_COMMAND(ID_EDIT_RIGHTCLICK_DYE, OnRightClickDye)
	ON_COMMAND(ID_EDIT_RIGHTCLICK_MOVE, OnRightClickMove)
	ON_COMMAND(ID_EDIT_RIGHTCLICK_ENERGY, OnRightClickEnergy)

	ON_COMMAND(ID_EDIT_CLEAR_DYE, OnClearDye)

	ON_COMMAND(ID_SIMULATION_START, OnSimulationStart)
	ON_COMMAND(ID_SIMULATION_STOP, OnSimulationStop)
	ON_COMMAND(ID_SIMULATION_SINGLE_STEP, OnSimulationSingleStep)
	ON_COMMAND(ID_SIMULATION_CONTINUE, OnSimulationContinue)

	ON_COMMAND(ID_VIEW_ZOOM_IN, OnZoomIn)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, OnZoomOut)
	ON_COMMAND(ID_VIEW_VIEW_ALL, OnViewAll)

	ON_COMMAND(ID_EDIT_CUT, OnCut)
	ON_COMMAND(ID_EDIT_COPY, OnCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnPaste)
	ON_COMMAND(ID_EDIT_DELETE, OnDelete)
	ON_COMMAND(ID_EDIT_FIND, OnFind)

	ON_WM_LBUTTONDOWN() 
	ON_WM_LBUTTONUP() 
	ON_WM_LBUTTONDBLCLK() 
	ON_WM_MOUSEMOVE() 
	ON_WM_CHAR() 
	ON_WM_KEYUP() 
	ON_WM_KEYDOWN() 
	ON_WM_RBUTTONDOWN() 
	ON_WM_RBUTTONUP() 
	ON_WM_MOUSEWHEEL() 

	ON_WM_SETFOCUS() 
	ON_WM_KILLFOCUS() 
	ON_WM_SIZE() 

	ON_WM_ERASEBKGND()

END_MESSAGE_MAP()

CevolveView::CevolveView()
{
	m_stopped = true;
	m_panning = false;
	m_drawingBarrier = false;
	m_find_expression = "";
	m_movingOrganism = false;
	m_move_cell = NULL;
}

CevolveView::~CevolveView()
{
}

BOOL CevolveView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	set_rca(RCA_NOTHING);

	return CView::PreCreateWindow(cs);
}

void CevolveView::update_statusbar()
{
	CevolveApp *app;
	CMainFrame *mainFrame;
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u;
	char *status;

	app = (CevolveApp *) AfxGetApp();
	mainFrame = (CMainFrame*) app->m_pMainWnd;

	u = pDoc->universe;
	ASSERT( u != NULL );

	if( m_stopped )
		status = "Idle";
	else
		status = "Running";

	mainFrame->set_status(status,
		u->norganism,
		u->nborn,
		u->ndie,
		u->step);
}

void CevolveView::update_status_cursor(CPoint &point)
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;
	CevolveApp *app;
	CMainFrame *mainFrame;
	double x, y;
	int xi, yi;
	UNIVERSE_GRID ugrid;
	GRID_TYPE gt;
	bool show_energy;
	int energy;

	app = (CevolveApp *) AfxGetApp();
	mainFrame = (CMainFrame*) app->m_pMainWnd;

	TF_WinToWorld(m_zh.tf, point.x, point.y, &x, &y);
	xi = (int) floor(x+0.5);
	yi = (int) floor(y+0.5);

	show_energy = false;

	if( xi >= 0 && xi < u->width && yi >= 0 && yi < u->height ) {
		gt = Universe_Query(u, xi, yi, &ugrid);
		if( gt == GT_SPORE ) {
			show_energy = true;
			energy = ugrid.u.spore->energy;
		} else if( gt == GT_ORGANIC ) {
			show_energy = true;
			energy = ugrid.u.energy;
		} else if( gt == GT_CELL ) {
			show_energy = true;
			energy = ugrid.u.cell->organism->energy;
		}
	}


	if( show_energy ) {
		mainFrame->set_status_cursor2(xi, yi, energy);
	} else {
		mainFrame->set_status_cursor(xi, yi);
	}
}

void CevolveView::create_barrier_grid(CDC *dc, int x, int y)
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;
	TF_RECT ugworld, ugwin;
	CRect rect;
	GRID_TYPE gt;
	UNIVERSE_GRID ugrid;

	if( x < 0 || x >= u->width )
		return;

	if( y < 0 || y >= u->height )
		return;

	gt = Universe_Query(u, x, y, &ugrid);
	if( gt != GT_BLANK && gt != GT_BARRIER )
		return;

	if( gt == GT_BLANK && m_barrierOn ) {
		Universe_SetBarrier(u, x, y);
		pDoc->SetModifiedFlag(true);

	} else if( gt == GT_BARRIER && !m_barrierOn ) {
		Universe_ClearBarrier(u, x, y);
		pDoc->SetModifiedFlag(true);
	}

	ugworld.left	= x - 0.5;
	ugworld.top	= y - 0.5;
	ugworld.right	= x + 0.5;
	ugworld.bottom	= y + 0.5;

	TF_WorldToWin(m_zh.tf, ugworld.left, ugworld.top,
				&ugwin.left, &ugwin.top);

	TF_WorldToWin(m_zh.tf, ugworld.right, ugworld.bottom,
				&ugwin.right, &ugwin.bottom);

	rect.left	= (int) ugwin.left;
	rect.top	= (int) ugwin.top;
	rect.right	= (int) ugwin.right;
	rect.bottom	= (int) ugwin.bottom;

	//
	// This ensures that we draw somthing, even at very
	// tiny scales.
	//
	if( rect.right - rect.left <= 1
			|| rect.bottom - rect.top <= 1 ) {
		rect.right += 1;
		rect.bottom += 1;
	}

	dc->Rectangle(rect);
}

void CevolveView::create_barrier_grids(CDC *dc, int x, int y)
{
	create_barrier_grid(dc, x, y);

	if( get_rca() == RCA_BARRIER ) {
		create_barrier_grid(dc, x, y-1);
		create_barrier_grid(dc, x, y+1);
		create_barrier_grid(dc, x-1, y);
		create_barrier_grid(dc, x+1, y);
	}
}

//
// Draw barrier between
// m_Point1 - m_Point2
//
void CevolveView::create_barrier()
{
	CDC *dc;
	CBrush *pOldBrush;
	CPen *pOldPen;
	double fx, fy;
	int x0, y0, x1, y1;
	int dx, dy, stepx, stepy, fraction;

	dc = GetDC();

	if( m_barrierOn ) {
		pOldBrush = dc->SelectObject(&m_appearance.barrierBrush);
		pOldPen = dc->SelectObject(&m_appearance.barrierPen);
	} else {
		pOldBrush = dc->SelectObject(&m_appearance.xbarrierBrush);
		pOldPen = dc->SelectObject(&m_appearance.xbarrierPen);
	}

	TF_WinToWorld(m_zh.tf, m_Point1.x, m_Point1.y, &fx, &fy);
	x0 = (int) floor(fx+0.5);
	y0 = (int) floor(fy+0.5);

	TF_WinToWorld(m_zh.tf, m_Point2.x, m_Point2.y, &fx, &fy);
	x1 = (int) floor(fx+0.5);
	y1 = (int) floor(fy+0.5);

	if( (x1 == m_lastx) && (y1 == m_lasty) ) {
		ReleaseDC(dc);
		return;
	}

	m_lastx = x1;
	m_lasty = y1;

	//
	// Traverse a line from (x0, y0) to (x1, y1)
	//
	dy = y1 - y0;
	dx = x1 - x0;

	if( dy < 0 ) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}

	if( dx < 0 ) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}

	dy <<= 1;		// dy is now 2*dy
	dx <<= 1;		// dx is now 2*dx

	create_barrier_grids(dc, x0, y0);

	if (dx > dy) {
		fraction = dy - (dx >> 1);		// same as 2*dy - dx
		while( x0 != x1 ) {
			if( fraction >= 0 ) {
				y0 += stepy;
				fraction -= dx;		// same as fraction -= 2*dx
			}
			x0 += stepx;
			fraction += dy;			// same as fraction -= 2*dy

			create_barrier_grids(dc, x0, y0);
		}
	} else {
		fraction = dx - (dy >> 1);
		while( y0 != y1 ) {
			if( fraction >= 0 ) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;

			create_barrier_grids(dc, x0, y0);
		}
	}

	dc->SelectObject(pOldBrush);
	dc->SelectObject(pOldPen);
	ReleaseDC(dc);
}

//
// During the moving of an organism this routine is
// called each time the mouse moves.
//
// m_Point2 is the current mouse location
// m_Point1 is the previous mouse location
// m_move_cell is the cell of the organism being moved
//
void CevolveView::draw_organism_outline()
{
	ORGANISM *organism;
	CELL *c;
	int x, y;
	int ox, oy;
	int nx, ny;
	TF_RECT ugworld, ugwin;
	CRect rect;
	CDC *dc;
	double fx, fy;
	int pOldROP2;
	CGdiObject *pOldBrush;
	CPen *pOldPen;

	dc = GetDC();

	pOldBrush = dc->SelectStockObject(HOLLOW_BRUSH);
	pOldPen = dc->SelectObject(&m_appearance.dyePen);
	pOldROP2 = dc->SetROP2(R2_XORPEN);

	ox = m_move_cell->x;
	oy = m_move_cell->y;

	//
	// First erase previous outline at point m_Point1
	//
	TF_WinToWorld(m_zh.tf, m_Point1.x, m_Point1.y, &fx, &fy);
	x = (int) floor(fx+0.5);
	y = (int) floor(fy+0.5);

	organism = m_move_cell->organism;
	for(c=organism->cells; c != NULL; c=c->next) {
		nx = x + (c->x - ox);
		ny = y + (c->y - oy);

		ugworld.left	= nx - 0.5;
		ugworld.top	= ny - 0.5;
		ugworld.right	= nx + 0.5;
		ugworld.bottom	= ny + 0.5;

		TF_WorldToWin(m_zh.tf, ugworld.left, ugworld.top,
				&ugwin.left, &ugwin.top);

		TF_WorldToWin(m_zh.tf, ugworld.right, ugworld.bottom,
				&ugwin.right, &ugwin.bottom);

		rect.left	= (int) ugwin.left;
		rect.top	= (int) ugwin.top;
		rect.right	= (int) ugwin.right;
		rect.bottom	= (int) ugwin.bottom;

		//
		// This ensures that we draw somthing, even at very
		// tiny scales.
		//
		if( rect.right - rect.left <= 1
				|| rect.bottom - rect.top <= 1 ) {
			rect.right += 1;
			rect.bottom += 1;
		}

		dc->Rectangle(rect);
	}

	//
	// Draw new outlin at point m_Point2
	//
	TF_WinToWorld(m_zh.tf, m_Point2.x, m_Point2.y, &fx, &fy);
	x = (int) floor(fx+0.5);
	y = (int) floor(fy+0.5);

	organism = m_move_cell->organism;
	for(c=organism->cells; c != NULL; c=c->next) {
		nx = x + (c->x - ox);
		ny = y + (c->y - oy);

		ugworld.left	= nx - 0.5;
		ugworld.top	= ny - 0.5;
		ugworld.right	= nx + 0.5;
		ugworld.bottom	= ny + 0.5;

		TF_WorldToWin(m_zh.tf, ugworld.left, ugworld.top,
				&ugwin.left, &ugwin.top);

		TF_WorldToWin(m_zh.tf, ugworld.right, ugworld.bottom,
				&ugwin.right, &ugwin.bottom);

		rect.left	= (int) ugwin.left;
		rect.top	= (int) ugwin.top;
		rect.right	= (int) ugwin.right;
		rect.bottom	= (int) ugwin.bottom;

		//
		// This ensures that we draw somthing, even at very
		// tiny scales.
		//
		if( rect.right - rect.left <= 1
				|| rect.bottom - rect.top <= 1 ) {
			rect.right += 1;
			rect.bottom += 1;
		}

		dc->Rectangle(rect);
	}

	dc->SelectObject(pOldBrush);
	dc->SelectObject(pOldPen);
	dc->SetROP2(pOldROP2);

	ReleaseDC(dc);
}

//
// Try and move organism.
//
//	m_move_cell is the cell that was first right clicked on to begin the move
//
//	m_Point2 is the screen coordinates when the user let go of the right click button.
//
//
void CevolveView::move_organism()
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;
	GRID_TYPE gt;
	UNIVERSE_GRID ugrid;
	double fx, fy;
	int x, y;
	int ox, oy;
	int nx, ny;
	ORGANISM *organism;
	CELL *c;
	bool vacant_spot;

	TF_WinToWorld(m_zh.tf, m_Point2.x, m_Point2.y, &fx, &fy);
	x = (int) floor(fx+0.5);
	y = (int) floor(fy+0.5);

	organism = m_move_cell->organism;
	ox = m_move_cell->x;
	oy = m_move_cell->y;

	if( ox == x && oy == y ) {
		Invalidate(true);
		return;
	}

	//
	// Make sure the move is possible
	//
	vacant_spot = true;
	for(c=organism->cells; c != NULL; c=c->next) {
		nx = x + (c->x - ox);
		ny = y + (c->y - oy);

		if( nx < 0 || nx >= u->width ) {
			vacant_spot = false;
			break;
		}

		if( ny < 0 || ny >= u->height ) {
			vacant_spot = false;
			break;
		}

		gt = Universe_Query(u, nx, ny, &ugrid);
		if( gt == GT_BLANK ) {
			//
			// Good.
			//

		} else if( gt == GT_CELL ) {
			//
			// Good, except if cell is not our own
			//
			if( ugrid.u.cell->organism != organism ) {
				vacant_spot = false;
				break;
			}
		} else {
			vacant_spot = false;
			break;
		}
	}

	if( ! vacant_spot ) {
		//
		// User moved the organism to an invalid location.
		//
		Invalidate(true);
		return;
	}

	//
	// Move organism
	//
	for(c=organism->cells; c != NULL; c=c->next) {
		Grid_Clear(u, c->x, c->y);
	}

	for(c=organism->cells; c != NULL; c=c->next) {
		nx = x + (c->x - ox);
		ny = y + (c->y - oy);
		c->x = nx;
		c->y = ny;
		Grid_SetCell(u, c);
	}

	//
	// Redraw the view.
	//
	pDoc->SetModifiedFlag(true);
	Invalidate(true);

}

void CevolveView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	update_statusbar();

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CevolveView::draw_select_box(CDC* pDC, int x, int y, int dist)
{
	CRect rect;
	int d;

	d = (dist < 4) ? 4 : dist;

	rect.left	= x - d;
	rect.top	= y - d;
	rect.right	= x + d;
	rect.bottom	= y + d;

	pDC->SelectObject(&m_appearance.barrierBrush);
	pDC->SelectObject(&m_appearance.barrierPen);
	pDC->Rectangle(rect);
}

void CevolveView::draw_selection(CDC* pDC, UNIVERSE *u, ORGANISM *o)
{
	CELL *curr;
	TF_RECT ob;
	bool first;
	double fx, fy, dist;

	ASSERT( pDC != NULL );
	ASSERT( u != NULL );
	ASSERT( o != NULL );

	first = true;
	for(curr=o->cells; curr; curr=curr->next) {
		if( first ) {
			first = false;
			ob.left = curr->x;
			ob.top = curr->y;
			ob.right = curr->x;
			ob.bottom = curr->y;
		} else {
			if( curr->x > ob.right )
				ob.right = curr->x;

			if( curr->x < ob.left )
				ob.left = curr->x;

			if( curr->y > ob.bottom )
				ob.bottom = curr->y;

			if( curr->y < ob.top )
				ob.top = curr->y;
		}
	}

	dist = TF_WorldDistance(m_zh.tf, 1.0, 0.0) / 5.0;

	TF_WorldToWin(m_zh.tf, ob.left - 0.5, ob.top - 0.5, &fx, &fy);
	draw_select_box(pDC, (int) fx, (int) fy, (int) dist);

	TF_WorldToWin(m_zh.tf, ob.left - 0.5, ob.bottom + 0.5, &fx, &fy);
	draw_select_box(pDC, (int) fx, (int) fy, (int) dist);

	TF_WorldToWin(m_zh.tf, ob.right + 0.5, ob.top - 0.5, &fx, &fy);
	draw_select_box(pDC, (int) fx, (int) fy, (int) dist);

	TF_WorldToWin(m_zh.tf, ob.right + 0.5, ob.bottom + 0.5, &fx, &fy);
	draw_select_box(pDC, (int) fx, (int) fy, (int) dist);

}

void CevolveView::draw_element(CDC* pDC, GRID_TYPE type, UNIVERSE_GRID& ugrid, TF_RECT& ugwin)
{
	CRect rect;
	CELL *cell;

	rect.left	= (int) ugwin.left;
	rect.top	= (int) ugwin.top;
	rect.right	= (int) ugwin.right;
	rect.bottom	= (int) ugwin.bottom;

	//
	// This ensures that we draw somthing, even at very
	// tiny scales.
	//
	if( rect.right - rect.left <= 1
			|| rect.bottom - rect.top <= 1 ) {
		rect.right += 1;
		rect.bottom += 1;
	}

	switch( type ) {
	case GT_BLANK:
		break;

	case GT_BARRIER:
		pDC->SelectObject(&m_appearance.barrierBrush);
		pDC->SelectObject(&m_appearance.barrierPen);
		pDC->Rectangle(rect);
		break;

	case GT_ORGANIC:
		pDC->SelectObject(&m_appearance.organicBrush);
		pDC->SelectObject(&m_appearance.organicPen);
		pDC->Rectangle(rect);
		break;

	case GT_SPORE:
		if( ugrid.u.spore->sflags & SPORE_FLAG_RADIOACTIVE ) {
			pDC->SelectObject(&m_appearance.sporeBrush);
			pDC->SelectObject(&m_appearance.dyePen);
		} else {
			pDC->SelectObject(&m_appearance.sporeBrush);
			pDC->SelectObject(&m_appearance.sporePen);
		}

		pDC->Rectangle(rect);
		break;

	case GT_CELL:
		cell = ugrid.u.cell;
		if( cell->kfm->terminated ) {
			pDC->SelectObject(&m_appearance.deadcellBrush);
			if( cell->organism->oflags & ORGANISM_FLAG_RADIOACTIVE ) {
				pDC->SelectObject( &m_appearance.dyePen );
			} else {
				pDC->SelectObject(&m_appearance.deadcellPen);
			}

		} else {
			pDC->SelectObject( m_appearance.cellBrush(cell->organism->strain) );
			if( cell->organism->oflags & ORGANISM_FLAG_RADIOACTIVE ) {
				pDC->SelectObject( &m_appearance.dyePen );
			} else {
				pDC->SelectObject( m_appearance.cellPen(cell->organism->strain) );
			}

		}

		pDC->Rectangle(rect);
		break;

	case GT_PLAYER:
		pDC->SelectObject( m_appearance.playerBrush );
		pDC->SelectObject( m_appearance.playerPen );
		pDC->Rectangle(rect);
		break;

	default:
		ASSERT(0);
	}
}

// CevolveView drawing

void CevolveView::OnDraw(CDC* the_pDC)
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u;
	ORGANISM *o;
	TF_RECT ugworld, ugwin;
	int x, y;
	int xstart, ystart;
	int xend, yend;
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;
	CBrush *pOldBrush;
	CPen *pOldPen;
	double fx, fy;

	ASSERT_VALID(pDoc);

	if( pDoc == NULL )
		return;

	u = pDoc->universe;
	ASSERT( u != NULL );

	CMemDC pDC(the_pDC);

	pOldBrush = pDC->SelectObject(&m_appearance.defaultBrush);
	pOldPen = pDC->SelectObject(&m_appearance.defaultPen);

	TF_WinToWorld(m_zh.tf, m_zh.window.left, m_zh.window.top, &fx, &fy);
	xstart = (int) floor(fx);
	ystart = (int) floor(fy);

	TF_WinToWorld(m_zh.tf, m_zh.window.right, m_zh.window.bottom, &fx, &fy);
	xend = (int) ceil(fx);
	yend = (int) ceil(fy);

	if( xstart < 0 )
		xstart = 0;

	if( xend >= u->width )
		xend = u->width-1;

	if( ystart < 0 )
		ystart = 0;

	if( yend >= u->height )
		yend = u->height-1;

	for(x=xstart; x <= xend; x++) {
		for(y=ystart; y <= yend; y++) {
			type = Universe_Query(u, x, y, &ugrid);
			if( type == GT_BLANK )
				continue;

			ugworld.left	= x - 0.5;
			ugworld.top	= y - 0.5;
			ugworld.right	= x + 0.5;
			ugworld.bottom	= y + 0.5;

			TF_WorldToWin(m_zh.tf, ugworld.left, ugworld.top,
						&ugwin.left, &ugwin.top);

			TF_WorldToWin(m_zh.tf, ugworld.right, ugworld.bottom,
						&ugwin.right, &ugwin.bottom);

			//
			// clipping: ignore ugwin rectangles that
			// will not be visible in the rect_win
			//
			if( ugwin.left > m_zh.window.right )
				continue;

			if( ugwin.right < m_zh.window.left )
				continue;

			if( ugwin.top > m_zh.window.bottom )
				continue;

			if( ugwin.bottom < m_zh.window.top )
				continue;

			draw_element(pDC, type, ugrid, ugwin);
		}
	}

	///////////////////////////////////////////////////
	// draw selection BEGIN
	///////////////////////////////////////////////////
	o = Universe_GetSelection(u);
	if( o != NULL ) {
		draw_selection(pDC, u, o);

	}
	///////////////////////////////////////////////////
	// draw selection END
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// draw border BEGIN
	///////////////////////////////////////////////////
	pDC->SelectObject(&m_appearance.borderBrush);
	pDC->SelectObject(&m_appearance.borderPen);

	TF_WorldToWin(m_zh.tf, 0.0 - 0.5, 0.0 - 0.5, &ugwin.left, &ugwin.top);
	TF_WorldToWin(m_zh.tf, u->width + 0.5, u->height + 0.5, &ugwin.right, &ugwin.bottom);
	CRect r;
	r.left		= (int) ugwin.left;
	r.top		= (int) ugwin.top;
	r.right		= (int) ugwin.right;
	r.bottom	= (int) ugwin.bottom;

	pDC->Rectangle(r);
	///////////////////////////////////////////////////
	// draw border END
	///////////////////////////////////////////////////

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CevolveView::OnInitialUpdate()
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u;
	CRect rect;

	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	m_panning = false;
	m_drawingBarrier = false;

	u = pDoc->universe;
	ASSERT( u != NULL );

	m_zh.set_world(u);
	GetClientRect(rect);

	m_zh.set_window(rect);
	m_zh.resize();
}

//
// begin zoom box draw
//
void CevolveView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDC *dc;

	m_panning = true;

	m_Point0 = point;
	m_Point1 = point;
	m_Point2 = point;

	dc = GetDC();

	CView::OnLButtonDown(nFlags, point);

	ReleaseDC(dc);
}

//
// finish panning, or select an organism
//
void CevolveView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDC *dc;

	if( ! m_panning )
		return;

	m_panning = false;

	m_Point2 = point;

	dc = GetDC();

	if( m_Point0 == m_Point2 ) {
		CevolveDoc* pDoc = GetDocument();
		UNIVERSE *u;
		UNIVERSE_GRID ugrid;
		GRID_TYPE type;
		double fx, fy;
		int x, y;

		TF_WinToWorld(m_zh.tf, point.x, point.y, &fx, &fy);
		x = (int) floor(fx+0.5);
		y = (int) floor(fy+0.5);

		u = pDoc->universe;

		if( x >= 0 && x < u->width && y >= 0 && y < u->height ) {
			type = Universe_Query(u, x, y, &ugrid);
			if( type == GT_CELL ) {
				Universe_SelectOrganism(u, ugrid.u.cell->organism);
				Invalidate(true);
			} else {
				Universe_ClearSelectedOrganism(u);
				Invalidate(true);
			}
		}
	}

	CView::OnLButtonUp(nFlags, point);

	ReleaseDC(dc);
}

void CevolveView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_Point1 = point;
	m_Point2 = point;

	OnLButtonUp(nFlags, point);
	CView::OnLButtonDblClk(nFlags, point);
	OnViewOrganism();
}


//
// Pan if mouse still down.
//
void CevolveView::OnMouseMove(UINT nFlags, CPoint point)
{
	int xdiff, ydiff;
	CDC *dc;

	/*
	 * If the left or right mouse buttons are no
	 * longer clicked, but we are still in a "mode",
	 * we call the appropriate ButtonUp routine.
	 */
	if( m_panning && (nFlags & MK_LBUTTON) == 0 ) {
		OnLButtonUp(nFlags, point);
		CView::OnMouseMove(nFlags, point);
		return;

	} else if( m_drawingBarrier && (nFlags & MK_RBUTTON) == 0 ) {
		OnRButtonUp(nFlags, m_Point2);
		CView::OnMouseMove(nFlags, point);
		return;

	} else if( m_movingOrganism && (nFlags & MK_RBUTTON) == 0 ) {
		OnRButtonUp(nFlags, m_Point2);
		CView::OnMouseMove(nFlags, point);
		return;
	}

	update_status_cursor(point);

	m_Point2 = point;

	if( m_panning ) {
		dc = GetDC();

		xdiff = m_Point1.x - m_Point2.x;
		ydiff = m_Point1.y - m_Point2.y;

		if( xdiff != 0 || ydiff != 0 ) {
			m_zh.pan(xdiff, ydiff);
			Invalidate(true);
		}

		ReleaseDC(dc);

	} else if( m_drawingBarrier ) {
		create_barrier();
	} else if( m_movingOrganism ) {
		draw_organism_outline();
	}

	m_Point1 = point;

	CView::OnMouseMove(nFlags, point);
}

void CevolveView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CView::OnChar(nChar, nRepCnt, nFlags);
}

void CevolveView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//
	// scroll buttons
	//
	switch( nChar ) {
	case VK_LEFT:
		m_zh.pan(-50, 0);
		Invalidate(true);
		break;

	case VK_RIGHT:
		m_zh.pan(50, 0);
		Invalidate(true);
		break;

	case VK_UP:
		m_zh.pan(0, -50);
		Invalidate(true);
		break;

	case VK_DOWN:
		m_zh.pan(0, 50);
		Invalidate(true);
		break;

	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CevolveView::OnRButtonDown_Barrier(UINT nFlags, CPoint point)
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;
	GRID_TYPE gt;
	UNIVERSE_GRID ugrid;
	double fx, fy;
	int x, y;

	TF_WinToWorld(m_zh.tf, point.x, point.y, &fx, &fy);
	x = (int) floor(fx+0.5);
	y = (int) floor(fy+0.5);

	if( x < 0 || x >= u->width )
		return;

	if( y < 0 || y >= u->height )
		return;

	m_drawingBarrier = true;

	//
	// The first thing we right click on determines
	// if we draw barrier or clear barrier.
	//
	gt = Universe_Query(u, x, y, &ugrid);
	if( gt == GT_BARRIER )
		m_barrierOn = false;
	else 
		m_barrierOn = true;

	m_lastx = -1;
	m_lasty = -1;

	m_Point1 = point;
}

void CevolveView::OnRButtonDown_Dye(UINT nFlags, CPoint point)
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;
	GRID_TYPE gt;
	UNIVERSE_GRID ugrid;
	double fx, fy;
	int x, y;
	ORGANISM *organism;
	SPORE *spore;

	//
	// Set dye flag for organism (or spore).
	//
	TF_WinToWorld(m_zh.tf, point.x, point.y, &fx, &fy);
	x = (int) floor(fx+0.5);
	y = (int) floor(fy+0.5);

	if( x < 0 || x >= u->width )
		return;

	if( y < 0 || y >= u->height )
		return;

	gt = Universe_Query(u, x, y, &ugrid);
	if( gt == GT_CELL ) {
		organism = ugrid.u.cell->organism;
		Universe_SetOrganismTracer(organism);
		pDoc->SetModifiedFlag(true);
		Invalidate(true);

	} else if( gt == GT_SPORE ) {
		spore = ugrid.u.spore;
		Universe_SetSporeTracer(spore);
		pDoc->SetModifiedFlag(true);
		Invalidate(true);
	}
}

void CevolveView::OnRButtonDown_Move(UINT nFlags, CPoint point)
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;
	GRID_TYPE gt;
	UNIVERSE_GRID ugrid;
	double fx, fy;
	int x, y;

	//
	// figure out where the user right clicked
	//
	TF_WinToWorld(m_zh.tf, point.x, point.y, &fx, &fy);
	x = (int) floor(fx+0.5);
	y = (int) floor(fy+0.5);

	if( x < 0 || x >= u->width )
		return;

	if( y < 0 || y >= u->height )
		return;

	gt = Universe_Query(u, x, y, &ugrid);
	if( gt != GT_CELL )
		return;

	m_movingOrganism = true;
	m_move_cell = ugrid.u.cell;
	m_Point1 = point;

}

void CevolveView::OnRButtonDown_Energy(UINT nFlags, CPoint point)
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;
	GRID_TYPE gt;
	UNIVERSE_GRID ugrid;
	double fx, fy;
	int x, y, energy1, energy2;
	ORGANISM *organism;
	SPORE *spore;

	//
	// Allow user to edit energy of item
	//
	TF_WinToWorld(m_zh.tf, point.x, point.y, &fx, &fy);
	x = (int) floor(fx+0.5);
	y = (int) floor(fy+0.5);

	if( x < 0 || x >= u->width )
		return;

	if( y < 0 || y >= u->height )
		return;

	gt = Universe_Query(u, x, y, &ugrid);
	if( gt == GT_CELL ) {
		organism = ugrid.u.cell->organism;
		energy1 = organism->energy;

	} else if( gt == GT_SPORE ) {
		spore = ugrid.u.spore;
		energy1 = spore->energy;

	} else if( gt == GT_ORGANIC ) {
		energy1 = ugrid.u.energy;
	} else {
		return;
	}

	EnergyTweakerDialog dlg;
	INT_PTR nRet;
	CPoint screen_point;

	screen_point = point;
	ClientToScreen(&screen_point);

	dlg.setup(screen_point.x, screen_point.y, energy1);

	nRet = dlg.DoModal();
	if( nRet == IDOK ) {
		// set energy
		energy2 = dlg.get_energy();
		if( energy2 > 0 && energy2 != energy1 ) {
			if( gt == GT_CELL ) {
				organism->energy = energy2;

			} else if( gt == GT_SPORE ) {
				spore->energy = energy2;

			} else if( gt == GT_ORGANIC ) {
				Grid_SetOrganic(u, x, y, energy2);
			}
			pDoc->SetModifiedFlag(true);
		}
	}
}

//
// Right click event:
//	Check the current right click action and engage that mode.
//
void CevolveView::OnRButtonDown(UINT nFlags, CPoint point)
{

	//
	// Right click tools don't work if
	// simulator is running.
	//
	if( ! m_stopped )
		return;

	if( get_rca() == RCA_NOTHING ) {
		// do nothing

	} else if( get_rca() == RCA_BARRIER || get_rca() == RCA_THIN_BARRIER ) {
		OnRButtonDown_Barrier(nFlags, point);

	} else if( get_rca() == RCA_DYE ) {
		OnRButtonDown_Dye(nFlags, point);

	} else if( get_rca() == RCA_MOVE ) {
		OnRButtonDown_Move(nFlags, point);

	} else if( get_rca() == RCA_ENERGY ) {
		OnRButtonDown_Energy(nFlags, point);
	}

	CView::OnRButtonDown(nFlags, point);
}

void CevolveView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if( get_rca() == RCA_NOTHING ) {
		//
		// no right click action, do nothing
		//

	} else if( get_rca() == RCA_BARRIER || get_rca() == RCA_THIN_BARRIER ) {
		// draw barrier pixel
		m_drawingBarrier = false;
		m_Point2 = point;
		create_barrier();

	} else if( get_rca() == RCA_DYE ) {
		//
		// Set dye flag for organism (or spore).
		// (nothing to do. mouse down event already did it)
		//

	} else if( get_rca() == RCA_MOVE ) {
		//
		// Begin moving organism
		//
		if( m_movingOrganism ) {
			m_Point2 = point;
			m_movingOrganism = false;
			move_organism();
		}

	} else if( get_rca() == RCA_ENERGY ) {
		//
		// Let user edit energy of object
		// (nothing to do. mouse down event already did it)
		//
	}


	CView::OnRButtonUp(nFlags, point);
}

afx_msg BOOL CevolveView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if( zDelta < 0 ) {
		/*
		 * back: Wheel being turned toward the user
		 */
		OnZoomOut();

	} else if( zDelta > 0 ) {
		/*
		 * forward: Wheel being turned away from user
		 */
		OnZoomIn();
	}

	return TRUE;
}

afx_msg void CevolveView::OnSetFocus(CWnd* pOldWnd)
{
	if( m_panning ) {
	}

	CView::OnSetFocus(pOldWnd);
}

afx_msg void CevolveView::OnKillFocus(CWnd* pNewWnd)
{
	if( m_panning ) {
	}

	CView::OnKillFocus(pNewWnd);
}

afx_msg void CevolveView::OnSize(UINT nType, int cx, int cy)
{
	CRect rect;
	GetClientRect(rect);

	m_zh.set_window(rect);
	m_zh.resize();

	Invalidate(true);
	CView::OnSize(nType, cx, cy);
}

BOOL CevolveView::OnEraseBkgnd(CDC* pDC) 
{
      return FALSE;
}

//
// Right click does nothing.
//
void CevolveView::OnRightClickChoose()
{
	CevolveApp *app;
	CMainFrame *mainFrame;

	app = (CevolveApp *) AfxGetApp();
	mainFrame = (CMainFrame*) app->m_pMainWnd;
	mainFrame->on_right_click_choose();
}

//
// Right click does nothing.
//
void CevolveView::OnRightClickNothing()
{
	set_rca(RCA_NOTHING);
}

//
// Draw normal thick barrier
//
void CevolveView::OnRightClickBarrier()
{
	set_rca(RCA_BARRIER);
}

//
// Draw a single grid width barrier
//
void CevolveView::OnRightClickThinBarrier()
{
	set_rca(RCA_THIN_BARRIER);
}

//
// Click on an organism and make it radio-active
//
void CevolveView::OnRightClickDye()
{
	set_rca(RCA_DYE);
}

//
// Allow user to move around organisms
//
void CevolveView::OnRightClickMove()
{
	set_rca(RCA_MOVE);
}

//
// Allow user to move around organisms
//
void CevolveView::OnRightClickEnergy()
{
	set_rca(RCA_ENERGY);
}

//
// Remove the dye marker from all organisms
// and spores.
//
void CevolveView::OnClearDye()
{
	CevolveApp *app = (CevolveApp *) AfxGetApp();
	CevolveDoc *pDoc = GetDocument();
	UNIVERSE *u = pDoc->universe;

	app->DoWaitCursor(1);

	Universe_ClearTracers(u);

	app->DoWaitCursor(-1);
	pDoc->SetModifiedFlag(true);
	Invalidate(true);

}

//
// Set check mark menu item 'id', uncheck the others.
//
void CevolveView::set_rca(RCA_TYPE type)
{
	CevolveApp *app;
	CMainFrame *mainFrame;

	app = (CevolveApp *) AfxGetApp();
	mainFrame = (CMainFrame*) app->m_pMainWnd;
	mainFrame->set_rca(type);
}

RCA_TYPE CevolveView::get_rca()
{
	CevolveApp *app;
	CMainFrame *mainFrame;

	app = (CevolveApp *) AfxGetApp();
	mainFrame = (CMainFrame*) app->m_pMainWnd;
	return mainFrame->get_rca();
}

void CevolveView::OnSimulatorProperties()
{
	CevolveDoc* pDoc = GetDocument();
	SimulationOptionsDialog dlg;
	INT_PTR nRet;

	dlg.SetKfmo(pDoc->universe->kfmo);
	nRet = dlg.DoModal();
	if( nRet == IDOK && dlg.modified ) {
		*pDoc->universe->kfmo = dlg.m_kfmo;
		pDoc->SetModifiedFlag(true);
	}
}

void CevolveView::OnViewUniverse()
{
	UniverseDialog dlg;
	CevolveDoc *doc;

	doc = GetDocument();
	dlg.SetDocument(doc);
	dlg.DoModal();
}

void CevolveView::OnViewOrganism()
{
	ViewOrganismDialog dlg;
	CevolveDoc *doc;
	UNIVERSE *u;
	ORGANISM *o;

	doc = GetDocument();
	u = doc->universe;

	o = Universe_GetSelection(u);
	if( o != NULL ) {
		dlg.SetOrganism(o);
		dlg.DoModal();
		if( dlg.SimulateSteps > 0 ) {
			doc->SetModifiedFlag(true);
			Invalidate(true);
			update_statusbar();
		}
	}
}

/***********************************************************************
 * Launch the 3D explorer program
 *
 * TODO:
 * 	1. Write simulation universe to a temporary file
 *	2. use that filename as argument to program
 *	3. How to cleanup on exit???? (pass -d flag to program)
 */
void CevolveView::OnView3D()
{
	CevolveDoc *doc;
	CString str;
	CWinApp *app;
	UNIVERSE *u;
	char cwd[1000], *p;
	char tmp_filename[1000], *tp;
	char args[1000];
	char errbuf[1000];
	char tmp_path[1000];
	char buf[1000];
	int n;

	doc = GetDocument();
	u = doc->universe;

	app = AfxGetApp();
	str = app->GetProfileString("evolve3d", "program");

	strcpy(cwd, str);

	p = strrchr(cwd, '\\');
	if( p == NULL ) {
		p = strrchr(cwd, '/');
	}

	if( p != NULL ) {
		*p = '\0';
	}

	n = GetTempPath(1000, tmp_path);
	if( n == 0 ) {
		sprintf(buf, "Unable to get TempPath");
		str = buf;
		AfxMessageBox(str, MB_OK, 0);
		return;
	}

	tp = _tempnam(tmp_path, "evolve3d_");
	if( tp != NULL ) {
		strcpy(tmp_filename, tp);
		strcat(tmp_filename, ".evolve");

		n = Universe_Write(u, tmp_filename, errbuf);
		if( !n ) {
			sprintf(buf, "%s", errbuf);
			str = buf;
			AfxMessageBox(str, MB_OK, 0);
			return;
		}
	} else {
		sprintf(buf, "Cannot create a unique filename\n" );
		str = buf;
		AfxMessageBox(str, MB_OK, 0);
		return;
	}

	sprintf(args, "%s /d", tmp_filename);

	ShellExecute(m_hWnd, "open",
		str,
		args,
		cwd,
		SW_SHOWNORMAL);

}

void CevolveView::OnViewStrainPopulation()
{
	StrainPopulation dlg;
	CevolveDoc *doc;

	doc = GetDocument();
	dlg.SetDocument(doc);
	dlg.DoModal();
}

void CevolveView::OnSimulationStart()
{
	CevolveApp *app;
	CevolveDoc *doc;
	int i;
	bool quit_signal;

	set_rca(RCA_NOTHING);

	app = (CevolveApp *) AfxGetApp();
	doc = GetDocument();

	if( ! m_stopped ) {
		m_stopped = true;
		return;
	}

	doc->SetModifiedFlag(true);

	app->DoWaitCursor(1);

	quit_signal = false;
	m_stopped = false;
	while( ! m_stopped ) {
		if( ! m_panning ) {
			for(i=0; i < 50; i++) {
				Universe_Simulate(doc->universe);
			}
			Invalidate(true);
			update_statusbar();
		}

		MSG msg;
		while( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
			if( !app->PumpMessage() ) {
				m_stopped = true;
				quit_signal = true;
				PostQuitMessage(0);
				break;
			}
		}
		app->DoWaitCursor(0);

		//
		// let MFC do its idle processing
		//
		LONG idle = 0;
		while( app->OnIdle(idle++) )
			;
	}

	app->DoWaitCursor(-1);

	if( ! quit_signal ) {
		//
		// doc is still valid, unless a quit
		// occured, in which case don't do anything
		// outside of this if block.
		//
		update_statusbar();
	}
}

void CevolveView::OnSimulationStop()
{
	set_rca(RCA_NOTHING);
	m_stopped = true;
}

void CevolveView::OnSimulationSingleStep()
{
	CevolveApp *app;
	CevolveDoc *doc;

	if( ! m_stopped ) {
		//
		// ignore this button if already simulating
		//
		return;
	}

	set_rca(RCA_NOTHING);

	app = (CevolveApp *) AfxGetApp();
	doc = GetDocument();

	doc->SetModifiedFlag(true);

	app->DoWaitCursor(1);
	Universe_Simulate(doc->universe);
	app->DoWaitCursor(-1);

	Invalidate(true);
	update_statusbar();
}

void CevolveView::OnSimulationContinue()
{
	CevolveApp *app;
	CevolveDoc *doc;
	bool quit_signal;

	set_rca(RCA_NOTHING);

	app = (CevolveApp *) AfxGetApp();
	doc = GetDocument();

	if( ! m_stopped ) {
		m_stopped = true;
		return;
	}

	app->DoWaitCursor(1);

	doc->SetModifiedFlag(true);

	quit_signal = false;
	m_stopped = false;
	while( ! m_stopped ) {
		if( ! m_panning ) {
			Universe_Simulate(doc->universe);
			Invalidate(true);
			update_statusbar();
		}
		MSG msg;
		while( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
			if( !app->PumpMessage() ) {
				m_stopped = true;
				quit_signal = true;
				PostQuitMessage(0);
				break;
			}
		}

		app->DoWaitCursor(0);

		//
		// let MFC do its idle processing
		//
		LONG idle = 0;
		while( app->OnIdle(idle++) )
			;
	}

	app->DoWaitCursor(-1);

	if( ! quit_signal ) {
		//
		// doc is still valid, unless a quit
		// occured, in which case don't do anything
		// outside of this if block.
		//
		update_statusbar();
	}

}

void CevolveView::OnZoomIn()
{
	m_zh.zoom_in();
	Invalidate(true);
}

void CevolveView::OnZoomOut()
{
	m_zh.zoom_out();
	Invalidate(true);
}

void CevolveView::OnViewAll()
{
	CevolveDoc* pDoc = GetDocument();
	UNIVERSE *u;
	CRect rect;

	u = pDoc->universe;
	ASSERT( u != NULL );

	GetClientRect(rect);

	m_zh.view_all();
	m_zh.set_world(u);
	m_zh.set_window(rect);
	m_zh.resize();

	Invalidate(true);
}

void CevolveView::OnDelete()
{
	ORGANISM *o;
	CevolveDoc *doc;

	doc = GetDocument();

	o = Universe_GetSelection(doc->universe);
	if( o == NULL )
		return;

	o = Universe_CutOrganism(doc->universe);

	Organism_delete(o);

	doc->SetModifiedFlag(true);
	Invalidate(true);
}



void CevolveView::OnCut()
{
	ORGANISM *o;
	CevolveApp *app;
	CevolveDoc *doc;

	doc = GetDocument();

	o = Universe_GetSelection(doc->universe);
	if( o == NULL )
		return;

	o = Universe_CutOrganism(doc->universe);

	app = (CevolveApp *) AfxGetApp();

	app->SetOrganism(o);

	doc->SetModifiedFlag(true);
	Invalidate(true);
}

void CevolveView::OnCopy()
{
	ORGANISM *o;
	CevolveApp *app;
	CevolveDoc *doc;

	doc = GetDocument();

	o = Universe_GetSelection(doc->universe);
	if( o == NULL )
		return;

	o = Universe_CopyOrganism(doc->universe);

	app = (CevolveApp *) AfxGetApp();

	app->SetOrganism(o);
}

void CevolveView::OnPaste()
{
	ORGANISM *o, *ocpy;
	CevolveApp *app;
	CevolveDoc *doc;

	app = (CevolveApp *) AfxGetApp();
	o = app->GetOrganism();

	if( o == NULL )
		return;

	doc = GetDocument();

#if 0
	//
	// delete current selection
	//
	osel = Universe_GetSelection(doc->universe);
	if( osel != NULL ) {
		osel = Universe_CutOrganism(doc->universe);
		Organism_delete(osel);
	}
#else
	//
	// Clear current selection
	//
	Universe_ClearSelectedOrganism(doc->universe);
#endif

	ocpy = Universe_DuplicateOrganism(o);

	Universe_PasteOrganism(doc->universe, ocpy);

	doc->SetModifiedFlag(true);
	Invalidate(true);
}

void CevolveView::OnFind()
{
	CevolveApp *app = (CevolveApp *) AfxGetApp();
	CevolveDoc *pDoc = GetDocument();
	FindDialog dlg;
	INT_PTR nRet;
	bool reset_tracers;

	dlg.SetFindExpression(m_find_expression);
	nRet = dlg.DoModal();
	if( nRet == IDOK ) {
		//
		// find expression is syntactically correct.
		//
		dlg.GetFindExpression(m_find_expression);

		reset_tracers = dlg.GetResetFlag();

		OrganismFinder ofc(m_find_expression, reset_tracers);
		ASSERT( ! ofc.error );

		app->DoWaitCursor(1);
		ofc.execute(pDoc->universe);
		app->DoWaitCursor(-1);
		pDoc->SetModifiedFlag(true);
		Invalidate(true);
	}
}

#ifdef _DEBUG
void CevolveView::AssertValid() const
{
	CView::AssertValid();
}

void CevolveView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CevolveDoc* CevolveView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CevolveDoc)));
	return (CevolveDoc*)m_pDocument;
}
#endif //_DEBUG

