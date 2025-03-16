// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30aug05	initial version
		01		10aug07	add template resource ID to ctor
		02		27dec09	add ShowPercent
		03		12dec22	add marquee mode; make pump messages public
		04		03mar25	modernize style

        progress dialog
 
*/

#pragma once

// ProgressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg);
// Construction
public:
	CProgressDlg(UINT nIDTemplate = IDD_PROGRESS, CWnd* pParent = NULL);
	~CProgressDlg();
	bool	Create(CWnd* pParent = NULL);

// Attributes
	void	SetPos(int nPos);
	void	SetRange(int nLower, int nUpper);
	bool	Canceled() const;
	void	ShowPercent(bool bEnable);
	void	SetMarquee(bool bEnable = true, int nInterval = 0);

// Operations
	void	PumpMessages();
	static	void	PumpMessages(HWND hWnd);

// Overrides
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
// Dialog Data
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl	m_wndProgress;
	CStatic	m_wndPercent;

// Generated message map functions
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()

// Member data
	UINT	m_nIDTemplate;		// template resource ID
    CWnd	*m_pParentWnd;		// pointer to parent window
	bool	m_bParentDisabled;	// true if parent window is disabled
	bool	m_bCanceled;		// true if cancel button was pressed
	bool	m_bShowPercent;		// true if showing percentage
	int		m_nPrevPercent;		// previous percentage

// Helpers
	void	ReenableParent();
};

inline bool CProgressDlg::Canceled() const
{
	return m_bCanceled;
}

inline void CProgressDlg::ShowPercent(bool bEnable)
{
	m_bShowPercent = bEnable;
}

inline void CProgressDlg::PumpMessages()
{
	PumpMessages(m_hWnd);
}
