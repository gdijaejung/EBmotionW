
// E2BoxManager.h : main header file for the E2BoxManager application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CE2BoxManagerApp:
// See E2BoxManager.cpp for the implementation of this class
//

class CE2BoxManagerApp : public CWinAppEx
{
public:
	CE2BoxManagerApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CE2BoxManagerApp theApp;
