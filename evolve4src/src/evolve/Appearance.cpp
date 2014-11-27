//
// Appearance.cpp : implementation of the "Appearance" class
//
// Weird name. I know. This class contains the colors/pens used
// to draw the universe.
//

#include "stdafx.h"
#include "evolve.h"

#include "evolveDoc.h"
#include "evolveView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Appearance::Appearance()
{
	defaultBrush.CreateStockObject(HOLLOW_BRUSH);
	defaultPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	borderBrush.CreateStockObject(HOLLOW_BRUSH);
	borderPen.CreatePen(PS_SOLID, 5, RGB(0x00, 0x00, 0xff));

	/* black */
	barrierBrush.CreateSolidBrush(RGB(0x00, 0x00, 0x00));
	barrierPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	/* white */
	xbarrierBrush.CreateSolidBrush( RGB(0xff, 0xff, 0xff) );
	xbarrierPen.CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0xff));

	/* white */
	organicBrush.CreateSolidBrush( RGB(0xff, 0xff, 0xff) );
	organicPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	/* blue */
	sporeBrush.CreateSolidBrush( RGB(0x00, 0xbf, 0xff) );
	sporePen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	/* player */
	playerBrush.CreateHatchBrush(HS_CROSS, RGB(0xff, 0xff, 0x00) );
	playerPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	/* red */
	deadcellBrush.CreateSolidBrush( RGB(0xff, 0x00, 0x00) );
	deadcellPen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	/* blue */
	activeBrush.CreateStockObject(HOLLOW_BRUSH);
	activePen.CreatePen(PS_SOLID, 3, RGB(0x00, 0x00, 0xff));

	/* STRAIN 0 - yellow (default) */
	m_cellBrush[0].CreateSolidBrush( RGB(0xff, 0xff, 0x00) );
	m_cellPen[0].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));


	/* STRAIN 1 - olive */
	m_cellBrush[1].CreateSolidBrush( RGB(128, 128, 0) );
	m_cellPen[1].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));


	/* STRAIN 2 - dark blue */
	m_cellBrush[2].CreateSolidBrush( RGB(0, 0, 255) );
	m_cellPen[2].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));


	/* STRAIN 3 - purple */
	m_cellBrush[3].CreateSolidBrush( RGB(255, 0, 255) );
	m_cellPen[3].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));


	/* STRAIN 4 - orange */
	m_cellBrush[4].CreateSolidBrush( RGB(255, 128, 0) );
	m_cellPen[4].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));


	/* STRAIN 5 - pink */
	m_cellBrush[5].CreateSolidBrush( RGB(255, 135, 135) );
	m_cellPen[5].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));


	/* STRAIN 6 - green */
	m_cellBrush[6].CreateSolidBrush( RGB(0, 192, 0) );
	m_cellPen[6].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));


	/* STRAIN 7 - grey */
	m_cellBrush[7].CreateSolidBrush( RGB(128, 128, 128) );
	m_cellPen[7].CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	/* green */
	dyePen.CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
}

Appearance::~Appearance()
{
}

CBrush* Appearance::cellBrush(int strain)
{
	ASSERT( strain >= 0 && strain < nstrains );

	return &m_cellBrush[strain];
}

CPen* Appearance::cellPen(int strain)
{
	ASSERT( strain >= 0 && strain < nstrains );

	return &m_cellPen[strain];
}
