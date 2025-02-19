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
	void	SetPos(int Pos);
	void	SetRange(int Lower, int Upper);
	bool	Canceled() const;
	void	ShowPercent(bool Enable);
	void	SetMarquee(bool bEnable = true, int nInterval = 0);

// Operations
	void	PumpMessages();
	static	void	PumpMessages(HWND hWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CProgressDlg)
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl	m_Progress;
	CStatic	m_Percent;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CProgressDlg)
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	UINT	m_IDTemplate;		// template resource ID
    CWnd	*m_ParentWnd;		// pointer to parent window
	bool	m_ParentDisabled;	// true if parent window is disabled
	bool	m_Canceled;			// true if cancel button was pressed
	bool	m_ShowPercent;		// true if showing percentage
	int		m_Lower;			// lower limit of range
	int		m_Upper;			// upper limit of range
	int		m_PrevPercent;		// previous percentage

// Helpers
	void	ReenableParent();
};

inline bool CProgressDlg::Canceled() const
{
	return(m_Canceled);
}

inline void CProgressDlg::ShowPercent(bool Enable)
{
	m_ShowPercent = Enable;
}

inline void CProgressDlg::PumpMessages()
{
	PumpMessages(m_hWnd);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
