// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23feb06	initial version
		01		28jan08	support Unicode

        list control with clipboard support
 
*/

#if !defined(AFX_EDITLISTCTRL_H__5F732D3C_5945_4CFD_9CD1_834D993AF676__INCLUDED_)
#define AFX_EDITLISTCTRL_H__5F732D3C_5945_4CFD_9CD1_834D993AF676__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl window

#include "DragListCtrl.h"
#include "Clipboard.h"

class CEditListCtrl : public CDragListCtrl
{
	DECLARE_DYNAMIC(CEditListCtrl);
// Construction
public:
	CEditListCtrl();

// Attributes
public:
	int		GetCurSel();
	void	SetCurSel(int Idx);
	bool	CanPaste() const;
	void	SetFormat(LPCTSTR Format, int InfoSize);

// Operations
public:
	void	SelectAll();
	void	Deselect();
	void	Cut();
	void	Copy();
	void	Paste();
	void	Delete();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditListCtrl)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditListCtrl();

protected:
// Generated message map functions
	//{{AFX_MSG(CEditListCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Overridables
	virtual	void	GetInfo(int Pos, void *Info) = 0;
	virtual	void	InsertInfo(int Pos, const void *Info) = 0;
	virtual	void	DeleteInfo(int Pos) = 0;

// Data members
	CClipboard	m_Clipboard;
	int		m_InfoSize;
};

inline int CEditListCtrl::GetCurSel()
{
	return(GetSelectionMark());
}

inline bool CEditListCtrl::CanPaste() const
{
	return(m_Clipboard.HasData());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITLISTCTRL_H__5F732D3C_5945_4CFD_9CD1_834D993AF676__INCLUDED_)
