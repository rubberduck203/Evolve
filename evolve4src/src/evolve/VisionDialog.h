#pragma once


// VisionDialog dialog

class VisionDialog : public CDialog
{
	DECLARE_DYNAMIC(VisionDialog)

public:
	VisionDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~VisionDialog();

	void SetCell(CELL *c);
	void Help();

	// Dialog Data
	enum { IDD = IDD_CELL_VISION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	Appearance m_appearance;
	BOOL OnInitDialog();
	CColorStaticST m_cell_pic;
	void setbg(CColorStaticST &sc, int i);

	void set_cell(int id);
	void set_where(int id, int value);
	void set_what(int id, int value);


	CELL *m_cell;

};
