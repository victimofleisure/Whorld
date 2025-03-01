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

#pragma once

// RowDlg.h : header file
//

#include "RowView.h"

class CRowDlg : public CDialog
{
	DECLARE_DYNAMIC(CRowDlg);
// Construction
public:
	CRowDlg(UINT nTemplate, CWnd* pParent = NULL);

// Attributes
	CRowView	*GetView() const;
	CWnd	*GetNotifyWnd() const;
	HACCEL	GetAccel(CWnd*& hAccelWnd) const;
	int		GetRowIndex() const;
	void	SetRowIndex(int iRow);
	int		GetRowPos() const;
	void	SetRowPos(int iPos);

// Overrides
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()

// Member data
	int		m_iRow;		// row's index in parent array
	int		m_iPos;		// row's display position; may differ from index
};

inline CRowView *CRowDlg::GetView() const
{
	CWnd	*pForm = GetParent();
	ASSERT(DYNAMIC_DOWNCAST(CRowView, pForm->GetParent()));
	return (CRowView *)pForm->GetParent();
}

inline CWnd *CRowDlg::GetNotifyWnd() const
{
	return GetView()->GetNotifyWnd();
}

inline HACCEL CRowDlg::GetAccel(CWnd*& hAccelWnd) const
{
	return GetView()->GetAccel(hAccelWnd);
}

inline int CRowDlg::GetRowIndex() const
{
	return m_iRow;
}

inline void CRowDlg::SetRowIndex(int iRow)
{
	m_iRow = iRow;
}

inline int CRowDlg::GetRowPos() const
{
	return m_iPos;
}

inline void CRowDlg::SetRowPos(int iPos)
{
	m_iPos = iPos;
}
