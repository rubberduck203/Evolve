// evolveDoc.cpp : implementation of the CevolveDoc class
//

#include "stdafx.h"
#include "evolve.h"

#include "evolveDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CevolveDoc

IMPLEMENT_DYNCREATE(CevolveDoc, CDocument)

BEGIN_MESSAGE_MAP(CevolveDoc, CDocument)
END_MESSAGE_MAP()


// CevolveDoc construction/destruction

CevolveDoc::CevolveDoc()
{
	universe = NULL;
}

CevolveDoc::~CevolveDoc()
{
	if( universe != NULL ) {
		Universe_Delete(universe);
		universe = NULL;
	}
}

BOOL CevolveDoc::OnNewDocument()
{
	UNIVERSE *u;
	CevolveApp *app;
	char errbuf[1000];
	NewUniverseOptions *nuo;

	if( ! CDocument::OnNewDocument() )
		return FALSE;

	app = (CevolveApp *) AfxGetApp();

	nuo = app->GetNewUniverseOptions();

	u = CreateUniverse(nuo, errbuf);
	if( u == NULL ) {
		AfxMessageBox(errbuf, MB_OK, 0);
		return false;
	}

	SetModifiedFlag(true);

	universe = u;
	return true;
}


void CevolveDoc::Serialize(CArchive& ar)
{
	//
	// do nothing as we have arn't using seriaize mechanism
	//
	return;

}

void CevolveDoc::DeleteContents()
{
	if( universe != NULL ) {
		Universe_Delete(universe);
		universe = NULL;
	}
}

BOOL CevolveDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	char errbuf[1000];
	UNIVERSE *u;

	u = LoadUniverse(lpszPathName, errbuf);
	if( u == NULL ) {
		AfxMessageBox(errbuf, MB_OK, 0);
		universe = NULL;
		return false;
	}

	universe = u;

	return true;
}

BOOL CevolveDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	int n;
	char errbuf[1000];

	n = StoreUniverse(lpszPathName, universe, errbuf);
	if( n == 0 ) {
		AfxMessageBox(errbuf, MB_OK, 0);
		return false;
	}

	SetModifiedFlag(false);

	return true;
}


// CevolveDoc diagnostics

#ifdef _DEBUG
void CevolveDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CevolveDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CevolveDoc commands
