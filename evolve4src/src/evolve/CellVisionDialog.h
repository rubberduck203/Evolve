#pragma once


// CellVisionDialog dialog

class CellVisionDialog : public CDialog
{
	DECLARE_DYNCREATE(CellVisionDialog)

public:
	CellVisionDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CellVisionDialog();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_CELL_VISION, IDH = IDR_HTML_ELLVISIONDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
#pragma once


// CellVisionDialog dialog

class CellVisionDialog : public CDialog
{
	DECLARE_DYNAMIC(CellVisionDialog)

public:
	CellVisionDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CellVisionDialog();

// Dialog Data
	enum { IDD = IDD_CELL_VISION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
