#pragma once


// UniverseDialog dialog

class UniverseDialog : public CDialog
{
	DECLARE_DYNAMIC(UniverseDialog)

public:
	UniverseDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~UniverseDialog();

	void SetDocument(CevolveDoc *doc);

	// Dialog Data
	enum { IDD = IDD_UNIVERSE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CevolveDoc *m_doc;

	void set_int(int id, int value);
	void set_long_long(int id, LONG_LONG value);
	void set_str(int id, CString& str);
	void set_ulong(int id, unsigned long value);
	void Help();

};
