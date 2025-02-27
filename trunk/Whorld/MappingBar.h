// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		20mar20	initial version
		01		29mar20	add property name accessors; add previous selection
		02		20jun21	add list accessor
		03		26feb25	adapt for Whorld

*/

#pragma once

#include "MyDockablePane.h"
#include "GridCtrl.h"
#include "Mapping.h"

class CMappingBar : public CMyDockablePane
{
	DECLARE_DYNAMIC(CMappingBar)
// Construction
public:
	CMappingBar();

// Attributes
public:
	static const int	GetPropertyNameID(int iProp);
	static const CString	GetPropertyName(int iProp);
	CGridCtrl&	GetListCtrl();

// Operations
public:
	void	OnUpdate(CView* pSender, LPARAM lHint = 0, CObject* pHint = NULL);
	void	OnMidiLearn();
	static void	AddMidiChannelComboItems(CComboBox& wndCombo);

// Overrides

// Implementation
public:
	virtual ~CMappingBar();

protected:
// Types
	class CModGridCtrl : public CGridCtrl {
	public:
		virtual	CWnd*	CreateEditCtrl(LPCTSTR pszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
		virtual	void	OnItemChange(LPCTSTR pszText);
	};

// Constants
	enum {
		IDC_MAPPING_GRID = 1931,
		MIDI_LEARN_COLOR = RGB(0, 255, 0),
	};
	enum {	// grid columns
		#define MAPPINGDEF_INCLUDE_NUMBER
		#define MAPPINGDEF(name, align, width, member, minval, maxval) COL_##name,
		#include "MappingDef.h"	// generate column enumeration
		COLUMNS
	};
	static const CGridCtrl::COL_INFO	m_arrColInfo[COLUMNS];	// list column data
	struct COL_RANGE {
		int		nMin;
		int		nMax;
	};
	static const COL_RANGE m_arrColRange[COLUMNS];	// array of column ranges

// Member data
	CModGridCtrl	m_grid;		// grid control
	CIntArrayEx		m_arrPrevSelection;	// previous selection during MIDI learn
	CMappingArray	m_clipboard;	// for clipboard commands

// Helpers
	void	InitEventNames();
	void	UpdateGrid();
	void	OnTrackArrayChange();
	void	OnTrackNameChange(int iTrack);

// Overrides
	virtual	void OnShowChanged(bool bShow);

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
	afx_msg void OnListColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnMidiEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnEditInsert();
	afx_msg void OnUpdateEditInsert(CCmdUI *pCmdUI);
	afx_msg void OnToolsMidiLearn();
	afx_msg void OnUpdateToolsMidiLearn(CCmdUI *pCmdUI);
};

inline const int CMappingBar::GetPropertyNameID(int iProp)
{
	ASSERT(iProp >= 0 && iProp < CMapping::PROPERTIES);
	return m_arrColInfo[iProp + 1].nTitleID;	// account for number column
}

inline const CString CMappingBar::GetPropertyName(int iProp)
{
	return LDS(GetPropertyNameID(iProp));
}

inline CGridCtrl& CMappingBar::GetListCtrl()
{
	return m_grid;
}
