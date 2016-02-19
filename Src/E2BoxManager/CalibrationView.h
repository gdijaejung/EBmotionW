#pragma once


// CCalibrationView dialog

class CCalibrationView : public CDockablePaneChildView2
{
	DECLARE_DYNAMIC(CCalibrationView)

public:
	CCalibrationView(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCalibrationView();

// Dialog Data
	enum { IDD = IDD_DIALOG_CALIB };

	common::cSerialAsync m_serial;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_ANCHOR_MAP();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonCnxy();
	afx_msg void OnBnClickedButtonCnxy180();
	afx_msg void OnBnClickedButtonCnz();
	afx_msg void OnBnClickedButtonCnz180();
	afx_msg void OnBnClickedButtonCnxyAuto();
	afx_msg void OnBnClickedButtonCnzAuto();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
