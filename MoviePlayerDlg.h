// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31aug05	initial version
		01		03may06	add selection range and export
		02		28jan08	support Unicode

        snapshot movie player
 
*/

#if !defined(AFX_MOVIEPLAYERDLG_H__DAD756C5_689F_44EE_82E7_00382AFA01DE__INCLUDED_)
#define AFX_MOVIEPLAYERDLG_H__DAD756C5_689F_44EE_82E7_00382AFA01DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MoviePlayerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMoviePlayerDlg dialog

#include "SnapMovie.h"
#include "ClickSliderCtrl.h"
#include "PersistDlg.h"

class CMainFrame;

class CMoviePlayerDlg : public CPersistDlg
{
	DECLARE_DYNAMIC(CMoviePlayerDlg);
// Construction
public:
	CMoviePlayerDlg(CWnd* pParent = NULL);   // standard constructor

// Constants
	enum {	// transport states
		STOP,
		PAUSE,
		PLAY
	};

// Attributes
	void	SetTransport(int State);
	int		GetTransport() const;
	bool	IsOpen() const;
	void	Hide(bool Enable);
	bool	IsHidden() const;
	void	SetLoop(bool Enable);
	bool	IsLooped() const;
	void	SetRange(int In, int Out);
	void	GetRange(int& In, int& Out) const;
	bool	HasRange() const;
	void	ClearRange();

// Operations
	bool	Open(LPCTSTR Path);
	void	Seek(int Frame);
	void	FullScreen(bool Enable);
	static	bool	Play(LPCTSTR Path);
	static	bool	Play();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMoviePlayerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMoviePlayerDlg)
	enum { IDD = IDD_MOVIE_PLAYER };
	CButton	m_GotoEnd;
	CButton	m_GotoStart;
	CButton	m_SetOut;
	CButton	m_SetIn;
	CButton	m_TakeSnap;
	CButton	m_Open;
	CButton	m_Save;
	CStatic	m_Time;
	CClickSliderCtrl	m_Pos;
	CButton	m_Loop;
	CButton	m_Stop;
	CButton	m_Play;
	CButton	m_Pause;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMoviePlayerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnPause();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnLoop();
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSave();
	afx_msg void OnOpen();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnTakeSnap();
	afx_msg void OnSetIn();
	afx_msg void OnSetOut();
	afx_msg void OnGotoStart();
	afx_msg void OnGotoEnd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		TIMER_ID = 1
	};

// Member data
	CSnapMovie	m_Movie;		// snapshot movie container
	CMainFrame	*m_Frm;			// pointer to main frame window
	CString	m_Path;				// path of currently open movie file
	CString	m_Caption;			// dialog's original caption, from resource
	int		m_Transport;		// transport state: see enum above
	int		m_Timer;			// if non-zero, our timer instance
	int		m_Period;			// timer period, in milliseconds
	bool	m_IsFullScreen;		// true if we're full-screen
	bool	m_Hidden;			// true if dialog has window size of zero
	CRect	m_InitRect;			// initial window rect for restoring size
	int		m_RangeIn;			// selection range first frame, or -1 if no range
	int		m_RangeOut;			// selection range last frame, or -1 if no range

// Helpers
	void	EnableTimer(int Period);
	void	SetTitle();
	void	UpdatePos();
};

inline bool CMoviePlayerDlg::IsOpen() const
{
	return(m_Movie.IsOpen());
}

inline bool CMoviePlayerDlg::IsHidden() const
{
	return(m_Hidden);
}

inline int CMoviePlayerDlg::GetTransport() const
{
	return(m_Transport);
}

inline bool CMoviePlayerDlg::IsLooped() const
{
	return(m_Loop.GetCheck() != 0);
}

inline void CMoviePlayerDlg::GetRange(int& In, int& Out) const
{
	In = m_RangeIn;
	Out = m_RangeOut;
}

inline bool CMoviePlayerDlg::HasRange() const
{
	return(m_RangeIn >= 0 && m_RangeOut >= 0);
}

inline void CMoviePlayerDlg::ClearRange()
{
	SetRange(-1, -1);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVIEPLAYERDLG_H__DAD756C5_689F_44EE_82E7_00382AFA01DE__INCLUDED_)
