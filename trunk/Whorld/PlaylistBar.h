// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun05	initial version
        01      23feb06	move clipboard support into edit list control
		02		23mar06	change INFO booleans from int to bool
        03      17apr06	rename link to CPatchLink
		04		05may06	allow patches to be videos
		05		06may06	add ten extra banks
        06      03jun06	add thumbnails
        07      26jun06	add export thumbnails
		08		28jan08	support Unicode        
		09		27mar25	refactor for V2

*/

#pragma once

#include "MyDockablePane.h"
#include "DragVirtualListCtrl.h"

class CPlaylistBar : public CMyDockablePane
{
	DECLARE_DYNAMIC(CPlaylistBar)
// Construction
public:
	CPlaylistBar();

// Attributes
public:
	CDragVirtualListCtrl& CPlaylistBar::GetListCtrl();

// Operations
public:
	void	OnUpdate(CView* pSender, LPARAM lHint = 0, CObject* pHint = NULL);
	void	UpdateList();
	void	Insert(int iInsert);
	void	Delete(const CIntArrayEx& arrSelection);
	void	Move(const CIntArrayEx& arrSelection, int iDropPos);

// Overrides

// Implementation
public:
	virtual ~CPlaylistBar();

protected:
// Types

// Constants
	enum {
		IDC_PATCH_LIST = 1932,
	};
	enum {	// list columns
		COL_PATCH_NAME,
		COL_PATCH_FOLDER,
		COLUMNS,
	};
	static const CListCtrlExSel::COL_INFO m_arrColInfo[COLUMNS];

// Member data
	CDragVirtualListCtrl	m_list;

// Helpers

// Undo

// Overrides

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnListGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnListColHdrReset();
	afx_msg void OnListReorder(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditInsert();
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
};

inline CDragVirtualListCtrl& CPlaylistBar::GetListCtrl()
{
	return m_list;
}
