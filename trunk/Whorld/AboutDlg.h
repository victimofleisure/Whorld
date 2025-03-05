// Copyleft 2017 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26mar17	initial version

        about dialog
 
*/

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
// Construction
public:
	CAboutDlg();

// Constants

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };
	CEdit	m_License;
	CStatic	m_Version;
};
