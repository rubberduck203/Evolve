// CellVisionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "CellVisionDialog.h"


// CellVisionDialog dialog

IMPLEMENT_DYNCREATE(CellVisionDialog, CDHtmlDialog)

CellVisionDialog::CellVisionDialog(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CellVisionDialog::IDD, CellVisionDialog::IDH, pParent)
{
}

CellVisionDialog::~CellVisionDialog()
{
}

void CellVisionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CellVisionDialog::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CellVisionDialog, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CellVisionDialog)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CellVisionDialog message handlers

HRESULT CellVisionDialog::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;  // return TRUE  unless you set the focus to a control
}

HRESULT CellVisionDialog::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;  // return TRUE  unless you set the focus to a control
}
// CellVisionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "CellVisionDialog.h"


// CellVisionDialog dialog

IMPLEMENT_DYNAMIC(CellVisionDialog, CDialog)
CellVisionDialog::CellVisionDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CellVisionDialog::IDD, pParent)
{
}

CellVisionDialog::~CellVisionDialog()
{
}

void CellVisionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CellVisionDialog, CDialog)
END_MESSAGE_MAP()


// CellVisionDialog message handlers
