// CalibrationView.cpp : implementation file
//

#include "stdafx.h"
#include "E2BoxManager.h"
#include "CalibrationView.h"
#include "afxdialogex.h"


// CCalibrationView dialog

IMPLEMENT_DYNAMIC(CCalibrationView, CDockablePaneChildView2)

CCalibrationView::CCalibrationView(CWnd* pParent /*=NULL*/)
: CDockablePaneChildView2(CCalibrationView::IDD, pParent)
{

}

CCalibrationView::~CCalibrationView()
{
}

void CCalibrationView::DoDataExchange(CDataExchange* pDX)
{
	CDockablePaneChildView2::DoDataExchange(pDX);
}


BEGIN_ANCHOR_MAP(CCalibrationView)
	ANCHOR_MAP_ENTRY(IDC_COMBO_SER, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_CONNECT, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_EDIT_SER_OUTPUT, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_EDIT_SER_INPUT, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
END_ANCHOR_MAP()


BEGIN_MESSAGE_MAP(CCalibrationView, CDockablePaneChildView2)
	ON_BN_CLICKED(IDOK, &CCalibrationView::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCalibrationView::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CCalibrationView::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_CNXY, &CCalibrationView::OnBnClickedButtonCnxy)
	ON_BN_CLICKED(IDC_BUTTON_CNXY180, &CCalibrationView::OnBnClickedButtonCnxy180)
	ON_BN_CLICKED(IDC_BUTTON_CNZ, &CCalibrationView::OnBnClickedButtonCnz)
	ON_BN_CLICKED(IDC_BUTTON_CNZ180, &CCalibrationView::OnBnClickedButtonCnz180)
	ON_BN_CLICKED(IDC_BUTTON_CNXY_AUTO, &CCalibrationView::OnBnClickedButtonCnxyAuto)
	ON_BN_CLICKED(IDC_BUTTON_CNZ_AUTO, &CCalibrationView::OnBnClickedButtonCnzAuto)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CCalibrationView message handlers

BOOL CCalibrationView::OnInitDialog()
{
	CDockablePaneChildView2::OnInitDialog();

	InitAnchors();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCalibrationView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePaneChildView2::OnSize(nType, cx, cy);

	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


void CCalibrationView::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedButtonConnect()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedButtonCnxy()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedButtonCnxy180()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedButtonCnz()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedButtonCnz180()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedButtonCnxyAuto()
{
	// TODO: Add your control notification handler code here
}


void CCalibrationView::OnBnClickedButtonCnzAuto()
{
	// TODO: Add your control notification handler code here
}

