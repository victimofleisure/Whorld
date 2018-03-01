// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jul05	initial version
 
		snapshot slide show dialog
 
*/

#if !defined(AFX_SLIDESHOWDLG_H__4AD2A4E7_EB2C_41BC_8F7B_1FD7749EC9DF__INCLUDED_)
#define AFX_SLIDESHOWDLG_H__4AD2A4E7_EB2C_41BC_8F7B_1FD7749EC9DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SlideShowDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSlideShowDlg dialog

class CMainFrame;

class CSlideShowDlg : public CDialog
{
	DECLARE_DYNAMIC(CSlideShowDlg);
// Construction
public:
	CSlideShowDlg(CWnd* pParent = NULL);   // standard constructor

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSlideShowDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CSlideShowDlg)
	enum { IDD = IDD_SLIDE_SHOW };
	CSpinButtonCtrl	m_SecsSpin;
	int		m_Seconds;
	BOOL	m_FullScreen;
	BOOL	m_Loop;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CSlideShowDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		TIMER_ID = 1		// identifies our timer instance
	};

// Member data
	CMainFrame	*m_Frm;		// pointer to main frame window
	int		m_Slides;		// number of slides loaded
	int		m_CurPos;		// index of current slide
	int		m_Timer;		// if non-zero, our timer instance
	bool	m_WasFullScreen;	// true if app was full-screen prior to show
	bool	m_Hidden;		// true if dialog has window size of zero
	CRect	m_InitRect;		// initial window rect for restoring size

// Helpers
	void	Hide(bool Enable);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDESHOWDLG_H__4AD2A4E7_EB2C_41BC_8F7B_1FD7749EC9DF__INCLUDED_)
