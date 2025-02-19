// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov13	initial version
		01		13feb14	add SetSelected
		02		15jun14	add tool tip support
		03		15jul14	fix OnToolHitTest return type
		04		23mar15	add RedrawSubItem
		05		24mar15	add column order methods
		06		04apr15	add GetInsertPos
		07		24apr18	standardize names
		08		25apr18	add GetSelected
		09		05jun18	make LoadArray public
		10		21jun18	add RedrawItem
		11		17mar20	add method to delete all columns
		12		01apr20	add ResetColumnHeader
		13		29jan22	add method to ensure item is horizontally visible

		extended selection list control
 
*/

#pragma once

// ListCtrlExSel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListCtrlExSel control

#include "ArrayEx.h"

class CListCtrlExSel : public CListCtrl
{
	DECLARE_DYNCREATE(CListCtrlExSel)
public:
// Construction
	CListCtrlExSel();
	virtual ~CListCtrlExSel();

// Types
	struct COL_INFO {	// column info
		int		nTitleID;	// string resource ID of column title
		int		nAlign;		// column alignment format
		int		nWidth;		// column width, in pixels
	};

// Attributes
public:
	int		GetColumnCount() const;
	int		GetSelection() const;
	void	GetSelection(CIntArrayEx& arrSel) const;
	void	SetSelection(const CIntArrayEx& arrSel);
	void	GetColumnWidths(CIntArrayEx& arrWidth);
	void	SetColumnWidths(const CIntArrayEx& arrWidth);
	bool	GetColumnOrder(CIntArrayEx& arrOrder);
	bool	SetColumnOrder(const CIntArrayEx& arrOrder);
	bool	GetSelected(int iItem) const;
	void	SetSelected(int iItem, bool bEnable);
	int		GetInsertPos() const;
	void	SetColumnName(int iCol, LPCTSTR pszName);

// Operations
public:
	void	CreateColumns(const COL_INFO *pColInfo, int nColumns);
	void	DeleteAllColumns();
	void	Select(int iItem);
	void	SelectOnly(int iItem);
	void	SelectRange(int iFirstItem, int nItems);
	void	SelectAll();
	void	Deselect();
	bool	SaveColumnWidths(LPCTSTR pszSection, LPCTSTR pszKey);
	bool	LoadColumnWidths(LPCTSTR pszSection, LPCTSTR pszKey);
	void	ResetColumnWidths(const COL_INFO *pColInfo, int nColumns);
	bool	SaveColumnOrder(LPCTSTR pszSection, LPCTSTR pszKey);
	bool	LoadColumnOrder(LPCTSTR pszSection, LPCTSTR pszKey);
	bool	ResetColumnOrder();
	void	ResetColumnHeader(const COL_INFO *pColInfo, int nColumns);
	void	FixContextMenuPoint(CPoint& point);
	void	EnableToolTips(BOOL bEnable = TRUE);
	BOOL	RedrawItem(int iItem);
	void	RedrawSubItem(int iItem, int iSubItem);
	void	EnsureHorizontallyVisible(int iItem, int iSubItem);
	static	bool	LoadArray(LPCTSTR pszSection, LPCTSTR pszKey, CIntArrayEx& arr, int nElems);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlExSel)
	public:
	virtual W64INT OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrlExSel)
	//}}AFX_MSG
	afx_msg BOOL OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Overrideables
	virtual	int		GetToolTipText(const LVHITTESTINFO* pHTI, CString& sText);
};

inline int CListCtrlExSel::GetColumnCount() const
{
	return(const_cast<CListCtrlExSel *>(this)->GetHeaderCtrl()->GetItemCount());
}

inline bool CListCtrlExSel::GetSelected(int iItem) const
{
	return GetItemState(iItem, LVIS_SELECTED) != 0;
}

inline BOOL CListCtrlExSel::RedrawItem(int iItem)
{
	return RedrawItems(iItem, iItem);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
