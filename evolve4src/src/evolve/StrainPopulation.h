#pragma once


//
// StrainPopulation dialog
//
class StrainPopulation : public CDialog
{
	DECLARE_DYNAMIC(StrainPopulation)

public:
	StrainPopulation(CWnd* pParent = NULL);
	virtual ~StrainPopulation();

	void SetDocument(CevolveDoc *doc);

	// Dialog Data
	enum { IDD = IDD_STRAIN_POPULATION };

private:
	CevolveDoc *m_doc;

	Appearance m_appearance;

	void OnHelp();
	BOOL OnInitDialog();

	void setbg(CColorStaticST &sc, int i);
	void setup_marker(CColorStaticST &m);
	int label_id(int i);
	int edit_id(int i);
	int static_id(int i);

	CColorStaticST m_strain1;
	CColorStaticST m_strain2;
	CColorStaticST m_strain3;
	CColorStaticST m_strain4;
	CColorStaticST m_strain5;
	CColorStaticST m_strain6;
	CColorStaticST m_strain7;
	CColorStaticST m_strain8;

	CColorStaticST m_marker1;
	CColorStaticST m_marker2;
	CColorStaticST m_marker3;
	CColorStaticST m_marker4;
	CColorStaticST m_marker5;
	CColorStaticST m_marker6;
	CColorStaticST m_marker7;
	CColorStaticST m_marker8;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

};
