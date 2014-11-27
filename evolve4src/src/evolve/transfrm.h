#ifndef _TRANSFRM_H
#define _TRANSFRM_H
/*
 * Copyright (c) 1991 Ken Stauffer, All Rights Reserved
 */
/***********************************************************************
 *
 */

typedef struct {
	double left;
	double right;
	double top;
	double bottom;
} TF_RECT;

typedef struct {
	TF_RECT win;
	TF_RECT world;
	double hratio_world;
	double wratio_world;
	double hratio_win;
	double wratio_win;
} TF_TRANSFORM;

extern TF_TRANSFORM	*TF_Make(void);
extern void		TF_Delete(TF_TRANSFORM *newp);
extern void		TF_Set(TF_TRANSFORM *tf, TF_RECT *win, TF_RECT *world);

extern void		TF_WorldToWin(TF_TRANSFORM *tf, double X, double Y, double *x, double *y);
extern void		TF_WinToWorld(TF_TRANSFORM *tf, double X, double Y, double *x, double *y);
extern double		TF_WorldDistance(TF_TRANSFORM *tf, double x, double y);
extern double		TF_WinDistance(TF_TRANSFORM *tf, double x, double y);

#endif
