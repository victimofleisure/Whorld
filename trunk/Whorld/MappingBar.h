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
		04		27feb25	add undo

*/

#pragma once

#include "MyDockablePane.h"
#include "GridCtrl.h"
#include "Mapping.h"
#include "Undoable.h"
#include "UndoManager.h"
#include "WhorldDoc.h"	// for custom undo manager

class CMappingBar : public CMyDockablePane, public CUndoable
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
	class CUndoMultiIntegerProp : public CRefObj {
	public:
		CIntArrayEx	m_arrSelection;	// indices of selected items
		CIntArrayEx	m_arrProp;	// array of integer property values
	};
	class CUndoSelectedMappings : public CRefObj {
	public:
		CIntArrayEx	m_arrSelection;	// indices of selected mappings
		CMappingArray	m_arrMapping;	// array of mappings
	};

// Constants
	enum {
		IDC_MAPPING_GRID = 1931,
		MIDI_LEARN_COLOR = RGB(0, 255, 0),
	};
	enum {	// grid columns
		#define MAPPINGDEF_INCLUDE_NUMBER
		#define MAPPINGDEF(name, align, width, prefix, member, minval, maxval) COL_##name,
		#include "MappingDef.h"	// generate column enumeration
		COLUMNS
	};
	static const CGridCtrl::COL_INFO	m_arrColInfo[COLUMNS];	// list column data
	struct COL_RANGE {
		int		nMin;
		int		nMax;
	};
	static const COL_RANGE m_arrColRange[COLUMNS];	// array of column ranges
	enum {	// undo codes
		#define MAPPINGUNDODEF(name) UCODE_##name,
		#include "MappingDef.h"	// generate enum
		MAPPING_UNDO_CODES
	};
	static const int m_arrUndoTitleId[MAPPING_UNDO_CODES];

// Member data
	CModGridCtrl	m_grid;		// grid control
	CIntArrayEx		m_arrPrevSelection;	// previous selection during MIDI learn
	CMappingArray	m_clipboard;	// for clipboard commands
	CWhorldDoc::CMyUndoManager	m_UndoMgr;	// custom undo manager
	static const CIntArrayEx	*m_parrSelection;	// pointer to selection array, used during undo

// Helpers
	void	InitEventNames();
	void	UpdateGrid();
	void	UpdateGrid(int iMapping, int iProp);
	void	UpdateGrid(const CIntArrayEx& arrSelection, int iProp);
	void	SetModifiedFlag();
	static void	MakeSelectionRange(CIntArrayEx& arrSelection, int iFirstItem, int nItems);

// Undo
	void	SaveProperty(CUndoState& State) const;
	void	RestoreProperty(const CUndoState& State);
	void	SaveMultiProperty(CUndoState& State) const;
	void	RestoreMultiProperty(const CUndoState& State);
	void	SaveSelectedMappings(CUndoState& State) const;
	void	RestoreSelectedMappings(const CUndoState& State);
	void	SaveMappings(CUndoState& State) const;
	void	RestoreMappings(const CUndoState& State);

// Overrides
	virtual	void OnShowChanged(bool bShow);
	virtual	void SaveUndoState(CUndoState& State);
	virtual	void RestoreUndoState(const CUndoState& State);
	virtual	CString	GetUndoTitle(const CUndoState& State);
	virtual void OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI);

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
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
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
