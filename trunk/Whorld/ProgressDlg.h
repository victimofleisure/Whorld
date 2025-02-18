// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30aug05	initial version

        progress dialog
 
*/

#if !defined(AFX_PROGRESSDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_)
#define AFX_PROGRESSDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg);
// Construction
public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	~CProgressDlg();
	bool	Create(CWnd* pParent = NULL);

// Attributes
	void	SetPos(int Pos);
	void	SetRange(int Lower, int Upper);
	bool	Canceled() const;

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
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CProgressDlg)
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
    CWnd	*m_pParentWnd;		// pointer to parent window
	bool	m_bParentDisabled;	// true if parent window is disabled
	bool	m_Canceled;			// true if cancel button was pressed

// Helpers
	void	ReenableParent();
	void	PumpMessages();
};

inline bool CProgressDlg::Canceled() const
{
	return(m_Canceled);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_)
