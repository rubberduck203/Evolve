// evolveDoc.h : interface of the CevolveDoc class
//


#pragma once

class CevolveDoc : public CDocument
{
protected: // create from serialization only
	CevolveDoc();
	DECLARE_DYNCREATE(CevolveDoc)

// Attributes
public:
	UNIVERSE *universe;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

// Implementation
public:
	virtual ~CevolveDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

