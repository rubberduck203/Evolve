//
// Appearance.h : colors for the 8 strains, and other stuff on the universe.
//


#pragma once

class Appearance
{
public:
	Appearance();
	~Appearance();

	CBrush	defaultBrush;
	CPen	defaultPen;

	CBrush	borderBrush;
	CPen	borderPen;

	CBrush	barrierBrush;
	CPen	barrierPen;

	CBrush	xbarrierBrush;
	CPen	xbarrierPen;

	CBrush	organicBrush;
	CPen	organicPen;

	CBrush	sporeBrush;
	CPen	sporePen;

	CBrush	playerBrush;
	CPen	playerPen;

	CBrush	deadcellBrush;
	CPen	deadcellPen;

	CBrush	activeBrush;
	CPen	activePen;

	CPen	dyePen;

	static const int nstrains = 8;

	CBrush*	cellBrush(int strain);
	CPen*	cellPen(int strain);

private:
	CBrush	m_cellBrush[8];
	CPen	m_cellPen[8];
};
