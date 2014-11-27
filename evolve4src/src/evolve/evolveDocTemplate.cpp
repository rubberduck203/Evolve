//
// evolveDocTemplate.cpp : implementation of the CevolveDocTemplate class
//

#include "stdafx.h"
#include "evolve.h"

#include "evolveDocTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CevolveDocTemplate::CevolveDocTemplate(UINT nIDResource,
				CRuntimeClass* pDocClass,
				CRuntimeClass* pFrameClass,
				CRuntimeClass* pViewClass)
{
//	CMultiDocTemplate::CMultiDocTemplate(nIDResource,
//					pDocClass,
//					pFrameClass,
//					pViewClass);
}

CevolveDocTemplate::~CevolveDocTemplate()
{
}

BOOL CevolveDocTemplate::GetDocString(CString& rString, enum DocStringIndex index)
{
	return CMultiDocTemplate::GetDocString(rString, index);
}
