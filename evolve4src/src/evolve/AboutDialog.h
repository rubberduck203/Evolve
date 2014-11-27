#pragma once

//
// AboutDlg dialog used for App About
//
class AboutDlg : public CDialog
{
public:
	AboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	void Help();
	BOOL OnInitDialog();


protected:
	DECLARE_MESSAGE_MAP()

};

