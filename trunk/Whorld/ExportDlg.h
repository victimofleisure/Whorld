// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28jun05	initial version
        01      21feb25	refactor
        02      09mar25	add export scaling types

        export options dialog
 
*/

#pragma once

// ExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

class CExportDlg : public CDialog
{
	DECLARE_DYNAMIC(CExportDlg);
// Construction
public:
	CExportDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	bool	GetSize(CSize& Size) const;
	bool	GetScaleToFit() const;
	int		GetResolution() const;

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
// Dialog Data
	enum { IDD = IDD_EXPORT };
	BOOL	m_bUseViewSize;
	int		m_nWidth;
	int		m_nHeight;
	CComboBox m_comboScalingType;

// Overrides
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	afx_msg void OnUpdateSize(CCmdUI *pCmdUI);
	afx_msg void OnCbnSelchangeExportScalingTypeCombo();
};
