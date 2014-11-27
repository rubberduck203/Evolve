
#pragma once

class CevolveDocTemplate : public CMultiDocTemplate
{
public:
	CevolveDocTemplate(UINT nIDResource,
				CRuntimeClass* pDocClass,
				CRuntimeClass* pFrameClass,
				CRuntimeClass* pViewClass);

	virtual ~CevolveDocTemplate();

	virtual BOOL GetDocString(CString& rString, enum DocStringIndex index);

};

