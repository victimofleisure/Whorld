// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version

        generic row form
 
*/

#if !defined(AFX_ROWDIALOGROW_H3__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_)
#define AFX_ROWDIALOGROW_H__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RowDialogRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRowDialogRow dialog

class CRowDialogRow : public CDialog
{
	DECLARE_DYNAMIC(CRowDialogRow);
// Construction
public:
	CRowDialogRow(UINT Template, CWnd* pParent = NULL);

// Attributes
	void	SetNotifyWnd(CWnd *Wnd, HACCEL Accel);
	int		GetRowIndex() const;
	void	SetRowIndex(int Idx);
	int		GetRowPos() const;
	void	SetRowPos(int Pos);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRowDialogRow)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CRowDialogRow)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog Data
	//{{AFX_DATA(CRowDialogRow)
	//}}AFX_DATA

// Member data
	CWnd	*m_NotifyWnd;	// notifications are sent to this window
	int		m_RowIdx;		// row's index in parent array
	int		m_RowPos;		// row's display position; may differ from index
	HACCEL	m_Accel;		// if non-null, row's keyboard accelerators
};

inline void CRowDialogRow::SetNotifyWnd(CWnd *Wnd, HACCEL Accel)
{
	m_NotifyWnd = Wnd;
	m_Accel = Accel;
}

inline int CRowDialogRow::GetRowIndex() const
{
	return(m_RowIdx);
}

inline void CRowDialogRow::SetRowIndex(int Idx)
{
	m_RowIdx = Idx;
}

inline int CRowDialogRow::GetRowPos() const
{
	return(m_RowPos);
}

inline void CRowDialogRow::SetRowPos(int Pos)
{
	m_RowPos = Pos;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROWDIALOGROW_H__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_)
