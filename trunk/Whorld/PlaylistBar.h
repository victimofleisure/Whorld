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
		10		30mar25	add playing state icon

*/

#pragma once

#include "MyDockablePane.h"
#include "DragVirtualListCtrl.h"
#include "Undoable.h"
#include "Playlist.h"

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
	void	Insert(int iInsert, CPlaylist::CPatchLinkArray& aPatchLink);
	void	Delete(const CIntArrayEx& arrSelection);
	void	Move(const CIntArrayEx& arrSelection, int iDropPos);
	bool	Play(int iPatch);

// Overrides
	void SaveUndoState(CUndoState& State);
	void RestoreUndoState(const CUndoState& State);

// Implementation
public:
	virtual ~CPlaylistBar();

protected:
// Types
	class CUndoSelection : public CRefObj {
	public:
		CIntArrayEx	m_arrSelection;	// indices of selected items
	};
	class CUndoSelectedPatches : public CRefObj {
	public:
		CIntArrayEx	m_arrSelection;	// indices of selected items
		CPlaylist::CPatchLinkArray	m_arrPatch;	// array of patches
	};
	class CUpdatePlaying {
	public:
		CUpdatePlaying();
		~CUpdatePlaying();
	};
	friend class CUpdatePlaying;

// Constants
	enum {
		IDC_PATCH_LIST = 1932,
		IDC_ICON_SIZE = 12,
	};
	enum {	// list columns
		COL_PATCH_NAME,
		COL_PATCH_FOLDER,
		COLUMNS,
	};
	static const CListCtrlExSel::COL_INFO m_arrColInfo[COLUMNS];
	enum {	// patch states
		PS_NORMAL,			// patch is normal
		PS_PLAYING,			// patch is currently playing
		PATCH_STATES,		// number of patch states
		STATE_ICONS = PATCH_STATES - 1,
	};
	static const int m_aStateIcon[STATE_ICONS];

// Member data
	CDragVirtualListCtrl	m_list;	// virtual list control with drag support
	static const CIntArrayEx	*m_parrSelection;	// pointer to selection array, used during undo
	CImageList	m_ilState;		// image list for patch state icons
	int		m_iPlayingPatch;	// index of currently playing patch, or -1 if none
	CString	m_sPlayingPatchPath;	// path string of currently playing patch, if any
	bool	m_bInPlay;			// reentrance guard for play method

// Helpers
	static CPlaylist*	GetPlaylist();

// Undo
	void	SaveSelectedPatches(CUndoState& State) const;
	void	RestoreSelectedPatches(const CUndoState& State);
	void	SavePatchMove(CUndoState& State) const;
	void	RestorePatchMove(const CUndoState& State);

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
	afx_msg void OnUpdateEditSelectAll(CCmdUI *pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnPlay();
	afx_msg void OnUpdatePlay(CCmdUI *pCmdUI);
};

inline CDragVirtualListCtrl& CPlaylistBar::GetListCtrl()
{
	return m_list;
}
