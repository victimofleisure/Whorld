// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01aug05	initial version
		01		29mar06	remove origin clamping
		02		07jun06	add video list to ViewBackup
		03		28jan08	support Unicode

        snapshot support
 
*/

#ifndef CSNAPSHOT_INCLUDED
#define CSNAPSHOT_INCLUDED

#include "SlideShowDlg.h"
#include "SortStringArray.h"
#include "VideoList.h"

class CWhorldView;

class CSnapshot : public WObject {
public:
// Construction
	CSnapshot();
	~CSnapshot();

// Nested classes
	class ViewBackup : public WObject {
	public:
		ViewBackup();
		~ViewBackup();
		void	Close();

	private:
		CMemFile	m_File;
		bool		m_IsOpen;
		CVideoList::INFO	*m_VideoInfo;
	};

// Attributes
	void	SetFrame(CMainFrame *Frm);
	bool	IsLoaded() const;
	int		GetCount() const;
	int		GetCurSel() const;
	LPCTSTR	GetPath(int Idx) const;
	LPCTSTR	GetCurPath() const;

// Operations
	bool	Take();
	bool	Take(LPCTSTR Path);
	bool	Load();
	LRESULT	OnMultiFileSel(WPARAM wParam, LPARAM lParam);
	bool	SetList(const CStringArray& SnapList);
	bool	Serialize(LPCTSTR Path, bool Load);
	void	Unload();
	bool	Show(int Idx);
	bool	SlideShow();

private:
// Data members
	CSlideShowDlg	m_SlideShowDlg;	// slide show dialog
	CSortStringArray	m_SnapList;	// sorted array of snapshot paths
	CMenu	m_SnapMenu;				// snapshot menu, inserted in main menu
	CMainFrame	*m_Frm;				// pointer to main frame
	ViewBackup	*m_ViewBackup;		// dynamically allocated backup of view
	bool	m_Preview;				// if true, file dialog previews snapshots
	bool	m_IsLoaded;				// if true, at least one snapshot is loaded
	int		m_CurSel;				// index of currently selected snapshot

// Helpers
	CWhorldView	*GetView() const;
};

inline void CSnapshot::SetFrame(CMainFrame *Frm)
{
	m_Frm = Frm;
}

inline bool CSnapshot::IsLoaded() const
{
	return(m_IsLoaded);
}

inline int CSnapshot::GetCount() const
{
	return(m_SnapList.GetSize());
}

inline int CSnapshot::GetCurSel() const
{
	return(m_CurSel);
}

inline LPCTSTR CSnapshot::GetPath(int Idx) const
{
	return(m_SnapList[Idx]);
}

inline LPCTSTR CSnapshot::GetCurPath() const
{
	return(m_SnapList[m_CurSel]);
}

#endif
