// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		20apr10	refactor

        row dialog base class
 
*/

#if !defined(AFX_ROWDLG_H__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_)
#define AFX_ROWDLG_H__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RowDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRowDlg dialog

#include "RowView.h"

class CRowDlg : public CDialog
{
	DECLARE_DYNAMIC(CRowDlg);
// Construction
public:
	CRowDlg(UINT Template, CWnd* pParent = NULL);

// Attributes
	CRowView	*GetView() const;
	CWnd	*GetNotifyWnd() const;
	HACCEL	GetAccel(CWnd*& AccelWnd) const;
	int		GetRowIndex() const;
	void	SetRowIndex(int Idx);
	int		GetRowPos() const;
	void	SetRowPos(int Pos);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRowDlg)
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
	//{{AFX_MSG(CRowDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog Data
	//{{AFX_DATA(CRowDlg)
	//}}AFX_DATA

// Member data
	int		m_RowIdx;		// row's index in parent array
	int		m_RowPos;		// row's display position; may differ from index
};

inline CRowView *CRowDlg::GetView() const
{
	CWnd	*pForm = GetParent();
	ASSERT(DYNAMIC_DOWNCAST(CRowView, pForm->GetParent()));
	return((CRowView *)pForm->GetParent());
}

inline CWnd *CRowDlg::GetNotifyWnd() const
{
	return(GetView()->GetNotifyWnd());
}

inline HACCEL CRowDlg::GetAccel(CWnd*& AccelWnd) const
{
	return(GetView()->GetAccel(AccelWnd));
}

inline int CRowDlg::GetRowIndex() const
{
	return(m_RowIdx);
}

inline void CRowDlg::SetRowIndex(int Idx)
{
	m_RowIdx = Idx;
}

inline int CRowDlg::GetRowPos() const
{
	return(m_RowPos);
}

inline void CRowDlg::SetRowPos(int Pos)
{
	m_RowPos = Pos;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROWDLG_H__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_)
