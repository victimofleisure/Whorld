// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03sep05	initial version
        01      15mar25	refactor

        movie export options dialog
 
*/

#pragma once

// MovieExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg dialog

class CMovieExportDlg : public CDialog
{
	DECLARE_DYNAMIC(CMovieExportDlg);
// Construction
public:
	CMovieExportDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes

// Public data
	CSize	m_szFrame;
	int		m_nFrameSelType;
	int		m_nTimeUnit;
	int		m_nFrameSizePreset;
	int		m_nScaleToFit;
	int		m_nRangeStart;
	int		m_nRangeEnd;
	int		m_nDuration;

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
// Dialog Data
	enum { IDD = IDD_MOVIE_EXPORT };
	CComboBox m_comboFrameSize;
	CComboBox m_comboScaleToFit;

// Overrides
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	afx_msg void OnUpdateFrameSize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateScaleToFit(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFrameSelType(CCmdUI *pCmdUI);
	afx_msg void OnSelchangeFrameSize();
	afx_msg void OnClickedFrameSelType();
};
