//
// ZoomHistory
//

#include "stdafx.h"


// AboutDialog dialog
ZoomHistory::ZoomHistory()
{
	m_sp = 0;
	tf = &m_stack[m_sp];
}

ZoomHistory::~ZoomHistory()
{
}

void ZoomHistory::set_world(UNIVERSE *u)
{
	world.left	= 0.0 - 0.5;
	world.right	= u->width + 0.5;
	world.top	= 0.0 - 0.5;
	world.bottom	= u->height + 0.5;
}

void ZoomHistory::set_window(CRect rect)
{
	CPoint p1, p2;

	p1 = rect.TopLeft();
	p2 = rect.BottomRight();

	win.left	= p1.x;
	win.top		= p1.y;
	win.right	= p2.x;
	win.bottom	= p2.y;

	window = win;

	preserve_aspect_ratio();
}

void ZoomHistory::resize()
{
	preserve_aspect_ratio();
	TF_Set(tf, &win, &world);
}

void ZoomHistory::view_all()
{
	m_sp = 0;
	tf = &m_stack[ m_sp ];

	world = tf->world;
	win = tf->win;
}

/*
 * Zoom in, but when no drag rectangle was specified
 * 
 */
void ZoomHistory::zoom_in()
{
	double w, h;

	CRect rect;

	/*
	 * Create 'rect' as a 1/4 smaller rectangle inside of
	 * 'window'.
	 */
	w = (win.right - win.left) / 8.0;
	h = (win.bottom - win.top) / 8.0;

	rect.left	= (int) (win.left	+ w);
	rect.top	= (int) (win.top	+ h);
	rect.right	= (int) (win.right	- w);
	rect.bottom	= (int) (win.bottom	- h);

	zoom_in(rect);
}

void ZoomHistory::zoom_in(CRect rect)
{
	TF_RECT zr, wr;

	if( m_sp+1 >= sizeof(m_stack)/sizeof(m_stack[0]) )
		return;

	if( rect.right >= rect.left ) {
		zr.left = rect.left;
		zr.right = rect.right;
	} else {
		zr.left = rect.right;
		zr.right = rect.left;
	}

	if( rect.bottom >= rect.top ) {
		zr.top = rect.top;
		zr.bottom = rect.bottom;
	} else {
		zr.top = rect.bottom;
		zr.bottom = rect.top;
	}

	TF_WinToWorld(tf, zr.left, zr.top, &wr.left, &wr.top);
	TF_WinToWorld(tf, zr.right, zr.bottom, &wr.right, &wr.bottom);

	m_sp++;
	tf = &m_stack[ m_sp ];

	world = wr;
	preserve_aspect_ratio();
	TF_Set(tf, &win, &world);
	pan(0, 0);
}

/*
 * If we are popping the last transform, then
 * restore that view. Otherwise pan the old view to be
 * centered to where the window is currently viewing.
 *
 */
void ZoomHistory::zoom_out()
{
	double ax, ay;
	double bx, by;
	double cx, cy;

	if( m_sp == 1 ) {
		/*
		 * restore to top-level view (show all)
		 */
		m_sp -= 1;
		tf = &m_stack[ m_sp ];
		world = tf->world;
		win = tf->win;

	} else if( m_sp > 1 ) {
		/*
		 * restore previous view, but pan to be centered where
		 * the we are currently positioned.
		 */

		ax = (world.right + world.left)/2.0;
		ay = (world.bottom + world.top)/2.0;

		m_sp -= 1;
		tf = &m_stack[ m_sp ];
		world = tf->world;
		win = tf->win;

		bx = (world.right + world.left)/2.0;
		by = (world.bottom + world.top)/2.0;

		cx = ax - bx;
		cy = ay - by;

		world.left	+= cx;
		world.top	+= cy;
		world.right	+= cx;
		world.bottom	+= cy;
		TF_Set(tf, &win, &world);
	}

}

/*
 * Pan display. (cx, cy) are values in window units
 */
void ZoomHistory::pan(int cx, int cy)
{
	TF_RECT tmp_win;
	TF_RECT new_world;

	tmp_win = win;

	tmp_win.left	+= cx;
	tmp_win.top	+= cy;
	tmp_win.right	+= cx;
	tmp_win.bottom	+= cy;

	TF_WinToWorld(tf, tmp_win.left, tmp_win.top, &new_world.left, &new_world.top);
	TF_WinToWorld(tf, tmp_win.right, tmp_win.bottom, &new_world.right, &new_world.bottom);

	world = new_world;

	TF_Set(tf, &win, &world);
	
}

/*
 * Change 'win' so its aspect ration matches the aspect ratio
 * of the 'world' rectangle.
 *
 */
void ZoomHistory::preserve_aspect_ratio()
{
	double window_ratio;
	double world_ratio;

	window_ratio = (win.bottom - win.top) / (win.right - win.left);
	world_ratio = (world.bottom - world.top) / (world.right - world.left);

	if( world_ratio > window_ratio ) {
		win.right = win.left +
				(win.bottom - win.top) / world_ratio;
	} else {
		win.bottom = win.top +
				(win.right - win.left) * world_ratio;
	}

}


