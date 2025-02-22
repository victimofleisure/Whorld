// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27mar18	initial version
		01		21feb25	customize for Whorld
		
*/

#pragma once

#include "PropertiesGrid.h"
#include "Options.h"

// COptionsDlg dialog

class COptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionsDlg)

public:
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptionsDlg();

protected:
// Dialog Data
	enum { IDD = IDD_OPTIONS };
	CPropertiesGridCtrl	m_Grid;

// Helpers

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnClickedResetAll();
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
};
