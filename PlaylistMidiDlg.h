// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05oct05	initial version

        playlist MIDI mapping dialog
 
*/

#if !defined(AFX_PLAYLISTMIDIDLG_H__7973600F_E5B4_478C_921B_572B1CCAE017__INCLUDED_)
#define AFX_PLAYLISTMIDIDLG_H__7973600F_E5B4_478C_921B_572B1CCAE017__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlaylistMidiDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlaylistMidiDlg dialog

#include "NoteEdit.h"
#include "NumSpin.h"

class CPlaylistMidiDlg : public CDialog
{
	DECLARE_DYNAMIC(CPlaylistMidiDlg);
// Construction
public:
	CPlaylistMidiDlg(CWnd* pParent = NULL);   // standard constructor

// Constants
	enum {	// mapping schemes
		MS_NONE,		// no mapping
		MS_PROG_CHG,	// program changes
		MS_NOTES,		// range of notes
		SCHEMES
	};

// Types
	typedef struct tagINFO {
		int		Scheme;
		int		Chan;
		int		First;
		int		Last;
	} INFO;

// Attributes
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);
	static	void	GetDefaults(INFO& Info);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlaylistMidiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPlaylistMidiDlg)
	enum { IDD = IDD_PLAYLIST_MIDI };
	CNumSpin	m_LastSpin;
	CNoteEdit	m_Last;
	CNumSpin	m_FirstSpin;
	CNoteEdit	m_First;
	CNumSpin	m_ChanSpin;
	CNumEdit	m_Chan;
	int		m_Scheme;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPlaylistMidiDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnScheme();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static	const	INFO	m_Defaults;

// Member data
	INFO	m_Info;

// Helpers
	void	UpdateUI();
};

inline void CPlaylistMidiDlg::GetInfo(INFO& Info) const
{
	Info = m_Info;
}

inline void CPlaylistMidiDlg::SetInfo(const INFO& Info)
{
	m_Info = Info;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYLISTMIDIDLG_H__7973600F_E5B4_478C_921B_572B1CCAE017__INCLUDED_)
