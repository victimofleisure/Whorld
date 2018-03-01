// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct05	initial version
		01		17feb06	allow variable property count

        MIDI auto assign dialog
 
*/

#if !defined(AFX_MIDIAUTOASSIGNDLG_H__E6F1D6D9_75E4_4565_B1B3_C0C2F48AAC5D__INCLUDED_)
#define AFX_MIDIAUTOASSIGNDLG_H__E6F1D6D9_75E4_4565_B1B3_C0C2F48AAC5D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiAutoAssignDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiAutoAssignDlg dialog

#include "NoteEdit.h"
#include "NumSpin.h"

class CMidiAutoAssignDlg : public CDialog
{
	DECLARE_DYNAMIC(CMidiAutoAssignDlg);
// Construction
public:
	CMidiAutoAssignDlg(CWnd* pParent = NULL);   // standard constructor

// Types
	typedef struct tagINFO {
		bool	PropRange;
		int		PropFirst;
		int		PropLast;
		int		Event;
		int		Chan;
		bool	ChanRange;
		int		Param;
		bool	ParamRange;
		bool	SetChanOnly;
	} INFO;

// Attributes
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiAutoAssignDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMidiAutoAssignDlg)
	enum { IDD = IDD_MIDI_AUTO_ASSIGN };
	CComboBox	m_PropLast;
	CComboBox	m_PropFirst;
	CStatic	m_ParmCap;
	int		m_Event;
	BOOL	m_ChanRange;
	CNoteEdit	m_Param;
	CNumSpin	m_ParamSpin;
	BOOL	m_ParamRange;
	CNumEdit	m_Chan;
	CNumSpin	m_ChanSpin;
	BOOL	m_SetChanOnly;
	int		m_PropRange;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMidiAutoAssignDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEventType();
	afx_msg void OnPropRange();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const INFO m_Defaults;

// Data members
	INFO	m_Info;
	int		m_Props;

// Helpers
	void	UpdateUI();
};

inline void CMidiAutoAssignDlg::GetInfo(INFO& Info) const
{
	Info = m_Info;
}

inline void CMidiAutoAssignDlg::SetInfo(const INFO& Info)
{
	m_Info = Info;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDIAUTOASSIGNDLG_H__E6F1D6D9_75E4_4565_B1B3_C0C2F48AAC5D__INCLUDED_)
