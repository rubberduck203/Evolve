/*
 * Copyright (c) 1991 Ken Stauffer, All Rights Reserved
 */

/***********************************************************************
 *
 * This file contains functions for doing transforms between a
 * "window" coordinate system and a "world" coordinate system
 *
 *	     top
 *	+---------------+
 *	|		|
 * left	|		| right
 *	|		|
 *	|		|
 *	+---------------+
 *	     bottom
 *
 *	The following must be true for all WINDOW transform rects:
 *		left < right && top < bottom
 *
 *	The axis of WORLD transforms may be reversed.
 * 
 *	Each viewport can allocate one transform for the duration of the
 *	program and use TF_Set() to change the transform as the user
 *	zooms/resized the window.
 *
 *			Module Description
 *
 * A TF_TRANSFORM object contains the information about the world and window
 * coordinates. TF_Use() is used to make that transform "active"
 * When done using a TF_TRANSFORM, call TF_Done(). The transform which
 * was active prior to a TF_Use() is restored after a call to  TF_Done().
 *
 * TF_Make() and TF_Delete() simply do the malloc() / free() of the TF_TRANSFORM
 * object.
 *
 * handle = TF_Make();
 *	Allocate a transform handle. (not nessesary).
 *
 * TF_Delete(handle);
 *	Free a handle object. (not nessesary).
 *
 * TF_Set(handle, &win_rect, &world_rect );
 *	Set the window and world rect for 'handle'.
 *	If win_rect is NULL or world_rect
 *	is NULL, it is not changed.
 *
 * TF_WinToWorld(handle, x, y, px, py);
 * TF_WorldToWin(handle, x, y, px, py);
 * TF_WinDist(handle, w, h, pw, ph);
 * TF_WorldDist(handle, w, h, pw, ph);
 *	These functions all use the 'current' transform.
 *
 */
#include <stdafx.h>


/***********************************************************************
 * TF_Make:
 *	Returns a new TF_TRANSFORM object.
 *	Return NULL if malloc fails.
 *
 *	The caller who uses these transform functions may pass their own
 *	pointer to a TF_TRANSFORM structure.
 *
 */
TF_TRANSFORM *TF_Make(void)
{
	TF_TRANSFORM *newp;

	newp = (TF_TRANSFORM *) CALLOC(1, sizeof(TF_TRANSFORM) );
	return newp;
}

void TF_Delete(TF_TRANSFORM *newp)
{
	FREE(newp);
}


/***********************************************************************
 * Initialize the handle with the win/world transform specified.
 *   hratio_win hratio_world wratio_win wratio_world:
 *	Are ratio's based on the height/width of the world and window
 *	rect's. These values reduce the amount of calculations needed 
 *	to perform a single transform.
 */
void TF_Set(TF_TRANSFORM *tf, TF_RECT *win, TF_RECT *world)
{
	if( win )
		tf->win = *win;

	if( world )
		tf->world = *world;

	tf->wratio_world = (win->right - win->left) /
					(world->right - world->left);
	tf->wratio_win = (world->right - world->left) /
					(win->right - win->left);

	tf->hratio_world = (win->bottom - win->top) /
					(world->bottom - world->top);
	tf->hratio_win = (world->bottom - world->top) /
					(win->bottom - win->top);
}

/***********************************************************************
 *	Applies the current world (window) transform to
 *	the (X, Y) pair returning (x, y) pair in window (world)
 *	units.
 *
 *	Uses the transforms from the most recently called
 *	TF_Use() call.
 */
void TF_WorldToWin(TF_TRANSFORM *tf, double X, double Y, double *x, double *y)
{

	*x = tf->win.left + (X - tf->world.left) * tf->wratio_world;

	*y = tf->win.top + (Y - tf->world.top) * tf->hratio_world;
}

void TF_WinToWorld(TF_TRANSFORM *tf, double X, double Y, double *x, double *y)
{
	*x = tf->world.left + (X - tf->win.left) * tf->wratio_win;

	*y = tf->world.top + (Y - tf->win.top) * tf->hratio_win;
}

/***********************************************************************
 *	x and y comprise two components of a vector in
 *	world (window) units. This function returns the length
 *	of the vector in window (world) units.
 */
double TF_WorldDistance(TF_TRANSFORM *tf, double x, double y)
{
	double X, Y;

	X = 0.0F + (x - 0.0F) * tf->wratio_world;
	Y = 0.0F + (y - 0.0F) * tf->hratio_world;

	return( sqrt(X*X + Y*Y) );
}

double TF_WinDistance(TF_TRANSFORM *tf, double x, double y)
{
	double X, Y;

	X = 0.0F + (x - 0.0F) * tf->wratio_win;
	Y = 0.0F + (y - 0.0F) * tf->hratio_win;

	return( sqrt(X*X + Y*Y) );
}


