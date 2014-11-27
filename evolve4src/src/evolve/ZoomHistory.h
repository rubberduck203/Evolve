#pragma once

//
// AboutDlg dialog used for App About
//
class ZoomHistory
{
public:
	ZoomHistory();
	~ZoomHistory();

	TF_TRANSFORM *tf;

	void set_world(UNIVERSE *u);
	void set_window(CRect rect);
	void resize();
	void view_all();
	void zoom_in();
	void zoom_in(CRect rect);
	void zoom_out();
	void pan(int cx, int cy);

	TF_RECT win;		// aspect ratio adjusted window viewport
	TF_RECT window;		// actual window viewport
	TF_RECT world;

private:
	void preserve_aspect_ratio();

	int m_sp;
	TF_TRANSFORM m_stack[100];

};

