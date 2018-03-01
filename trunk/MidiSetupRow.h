// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		23jan08	replace MIDI range scaler with start/end
		02		28jan08	support Unicode

		MIDI setup dialog row
 
*/

#if !defined(AFX_MIDISETUPROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_)
#define AFX_MIDISETUPROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiSetupRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupRow dialog

#include "RowDialogRow.h"
#include "NoteEdit.h"
#include "NumSpin.h"

class CMidiSetupRow : public CRowDialogRow
{
	DECLARE_DYNAMIC(CMidiSetupRow);
// Construction
public:
	CMidiSetupRow(CWnd* pParent = NULL);   // standard constructor

// Types
	typedef struct tagRANGE {
		double	Start;
		double	End;
	} DRANGE;
	typedef struct tagINFO {
		DRANGE	Range;
		int		Event;
		int		Chan;
		int		Ctrl;
	} INFO;

// Constants
	enum {	// event types
		ET_OFF,
		ET_CTRL,
		ET_NOTE,
		ET_PITCH
	};

// Attributes
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);
	void	Assign(int Event, int Chan, int Ctrl);
	int		GetEvent() const;
	void	GetRange(DRANGE& Range) const;
	void	SetCaption(LPCTSTR Title);
	void	SetValue(int Val);

// Operations
	void	SetSelected(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiSetupRow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMidiSetupRow)
	enum { IDD = IDD_MIDI_SETUP_ROW };
	CNumSpin	m_RangeStartSpin;
	CNumEdit	m_RangeStart;
	CNumSpin	m_RangeEndSpin;
	CNumEdit	m_RangeEnd;
	CNumSpin	m_CtrlSpin;
	CNumSpin	m_ChanSpin;
	CComboBox	m_Event;
	CNumEdit	m_Value;
	CNumEdit	m_Chan;
	CNoteEdit	m_Ctrl;
	CStatic	m_Title;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMidiSetupRow)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeEvent();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const COLORREF m_SelColor;	// selection color
	static const CBrush	m_SelBrush;		// selection brush

// Data members
	bool	m_Selected;	// true if we're selected
};

inline void CMidiSetupRow::GetRange(DRANGE& Range) const
{
	Range.Start = m_RangeStart.GetVal();
	Range.End = m_RangeEnd.GetVal();
}

inline int CMidiSetupRow::GetEvent() const
{
	return(m_Event.GetCurSel());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDISETUPROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_)
