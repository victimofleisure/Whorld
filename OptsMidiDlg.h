// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		13aug05	initial version

        MIDI options property page
 
*/

#if !defined(AFX_OPTSMIDIDLG_H__81298FE8_B61C_49BB_9AED_59E8AEAE9287__INCLUDED_)
#define AFX_OPTSMIDIDLG_H__81298FE8_B61C_49BB_9AED_59E8AEAE9287__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsMidiDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsMidiDlg dialog

#include "PersistVal.h"

class COptsMidiDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(COptsMidiDlg)
// Construction
public:
	COptsMidiDlg();
	~COptsMidiDlg();

// Attributes
	int		GetDevice() const;
	bool	IsSync() const;

// Operations
	void	SetDefaults();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsMidiDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsMidiDlg)
	enum { IDD = IDD_OPTS_MIDI };
	CComboBox	m_MidiDev;
	BOOL	m_MidiSync;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsMidiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeMidiDev();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagSTATE {
		int		MidiDev;	// index of MIDI device in m_DevList
		bool	MidiSync;	// if true, sync tempo to MIDI clocks
	} STATE;

// Constants
	static	const STATE	m_Default;

// Member data
	STATE	m_st;
	CStringArray	m_DevList;
	PersistVal<bool>	m_RegMidiSync;

// Helpers
	void	UpdateUI();
};

inline int COptsMidiDlg::GetDevice() const
{
	return(m_st.MidiDev);
}

inline bool COptsMidiDlg::IsSync() const
{
	return(m_st.MidiSync);
}


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSMIDIDLG_H__81298FE8_B61C_49BB_9AED_59E8AEAE9287__INCLUDED_)
