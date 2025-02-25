// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		08jan19	initial version
		01		01apr20	add ShowDockingContextMenu
		02		18nov20	add maximize/restore to docking context menu
		03		01nov21	add toggle show pane method
		04		17dec21	add full screen mode
		05		16feb24	override OnUpdateCmdUI to skip iterating child controls
		06		08feb25	remove maximize/restore and full screen
		07		25feb25	subclass parent mini-frame for get min/max info

*/

#pragma once

class CListCtrlExSel;

class CMyDockablePane : public CDockablePane
{
	DECLARE_DYNAMIC(CMyDockablePane)
// Construction
public:
	CMyDockablePane();

// Attributes
public:
	bool	FastIsVisible() const;

// Operations
public:
	bool	ShowDockingContextMenu(CWnd* pWnd, CPoint point);
	bool	FixContextMenuPoint(CWnd *pWnd, CPoint& point);
	void	ToggleShowPane();

// Overrides
	virtual void OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler);

// Overridables
	virtual void OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI);

// Implementation
public:
	virtual ~CMyDockablePane();

protected:
// Constants
	enum {
		ID_TOGGLE_MAXIMIZE = -1110,	// don't conflict with IDs in CPane::OnShowControlBarMenu
		ID_MINI_FRAME_SUBCLASS = 1989,
	};

// Data members
	bool	m_bIsShowPending;		// true if deferred show is pending
	bool	m_bFastIsVisible;		// true if this pane is visible

// Overrides

// Overridables
	virtual	void	OnShowChanged(bool bShow);

// Helpers
	static LRESULT CALLBACK MiniFrameSubclassProc(HWND hWnd, UINT uMsg, 
		WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnShowChanging(WPARAM wParam, LPARAM lParam);
};

inline bool CMyDockablePane::FastIsVisible() const
{
	return m_bFastIsVisible;
}
