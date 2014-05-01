// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01aug05	initial version
		01		27sep05	disable origin clamping
		02		01oct05	dtor must delete backup without accessing view
		03		25oct05	in OnMultiFileSel, open path in case it's a folder
		04		09nov05	in Take, add default file name
		05		29mar06	remove origin clamping
		06		07jun06	add video list to ViewBackup
		07		09jun06	prevent deadlock in missing files dialog
		08		23jun06	in Load, restore timer if SetList fails
		09		21dec07	rename GetMainFrame to GetThis
		10		28jan08	support Unicode

        snapshot support
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "Snapshot.h"
#include "MainFrm.h"
#include "WhorldDoc.h"
#include "WhorldView.h"
#include "PathStr.h"
#include "MultiFileDlg.h"

#define REG_SNAP_PREVIEW	_T("SnapPreview")

inline CWhorldView *CSnapshot::GetView() const
{
	return(m_Frm->GetView());
}

CSnapshot::CSnapshot()
{
	m_SnapMenu.LoadMenu(MAKEINTRESOURCE(IDR_SNAPSHOT));
	m_Frm = NULL;
	m_ViewBackup = NULL;
	m_Preview = CPersist::GetInt(REG_SETTINGS, REG_SNAP_PREVIEW, 0) != 0;
	m_IsLoaded = FALSE;
	m_CurSel = 0;
}

CSnapshot::~CSnapshot()
{
	CPersist::WriteInt(REG_SETTINGS, REG_SNAP_PREVIEW, m_Preview);
	if (m_ViewBackup != NULL) {
		m_ViewBackup->Close();	// so ~ViewBackup won't access non-existent view
		delete m_ViewBackup;
	}
}

CSnapshot::ViewBackup::ViewBackup()
{
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	Frm->GetView()->Serialize(m_File, FALSE);	// save view to memory file
	m_IsOpen = TRUE;
	m_VideoInfo = new CVideoList::INFO;
	Frm->GetView()->GetVideo().GetInfo(*m_VideoInfo);	// save video list
}

CSnapshot::ViewBackup::~ViewBackup()
{
	if (!m_IsOpen)
		return;	// nothing to do
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	m_File.SeekToBegin();	// rewind memory file
	Frm->GetView()->Serialize(m_File, TRUE);	// restore view from memory file
	Frm->GetView()->GetVideo().SetInfo(*m_VideoInfo);	// restore video list
	delete m_VideoInfo;
}

void CSnapshot::ViewBackup::Close()
{
	m_File.Close();
	m_IsOpen = FALSE;
}

bool CSnapshot::Take() 
{
	CMainFrame::TimerState	ts(FALSE);	// stop timer, dtor restores it
	CString	DefName;
	m_Frm->GetDefaultFileName(DefName);
	CMultiFileDlg	fd(FALSE, EXT_SNAPSHOT, DefName, OFN_OVERWRITEPROMPT, 
		LDS(IDS_FILTER_SNAPSHOT), LDS(IDS_SNAP_TAKE));
	if (fd.DoModal() == IDOK)
		return(Serialize(fd.GetPathName(), FALSE));	// store snapshot
	return(FALSE);
}

bool CSnapshot::Take(LPCTSTR Path)
{
	CMainFrame::TimerState	ts(FALSE);	// stop timer, dtor restores it
	return(Serialize(Path, FALSE));	// store snapshot
}

bool CSnapshot::Load()
{
	bool	PrevEnab = m_Frm->EnableTimer(FALSE);	// stop timer
	CMultiFileDlg	fd(TRUE, EXT_SNAPSHOT, NULL, 
		OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT,
		LDS(IDS_FILTER_SNAPSHOT), LDS(IDS_SNAP_LOAD));
	fd.SetExtendedStyle(CMultiFileDlg::ES_SHOWPREVIEW);	// enable preview checkbox
	fd.SetNotifyWnd(m_Frm);	// request file selection notifications
	fd.SetPreview(m_Preview);
	CString	PrevCap;
	m_Frm->GetWindowText(PrevCap);	// OnMultiFileSel may change caption
	BOOL	result;
	CVideoList&	vl = GetView()->GetVideo();
	bool	PrevFileSearch = vl.GetFileSearch();
	vl.SetFileSearch(FALSE);	// else missing files dialog deadlocks UI
	{
		ViewBackup	vb;	// save view, dtor restores it
		result = fd.DoModal();	// preview snapshots via OnMultiFileSel
	}
	vl.SetFileSearch(PrevFileSearch);
	m_Frm->SetWindowText(PrevCap);	// restore caption
	bool	retc = FALSE;
	if (result == IDOK && fd.GetPathCount())
		retc = SetList(*fd.GetPathArray());	// enter snap mode
	if (!retc)	// if user canceled or error
		m_Frm->EnableTimer(PrevEnab);	// restore timer
	m_Preview = fd.GetPreview();	// save preview setting
	return(retc);
}

LRESULT	CSnapshot::OnMultiFileSel(WPARAM wParam, LPARAM lParam)
{
	CMultiFileDlg	*fd = (CMultiFileDlg *)wParam;
	if (fd->GetPreview()) {	// if preview is checked
		CString	Path = fd->GetPathName();
		if (!Path.IsEmpty()) {
			CFile	fp;
			if (fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite)) {
				GetView()->Serialize(fp, TRUE);	// load snapshot
				m_Frm->SetCaption(fd->GetFileName());
			}
		}
	}
	return(0);
}

bool CSnapshot::SetList(const CStringArray& SnapList)
{
	m_SnapList.Copy(SnapList);
	m_SnapList.Sort();
	m_CurSel = 0;
	return(Serialize(m_SnapList[0], TRUE));
}

bool CSnapshot::Serialize(LPCTSTR Path, bool Load)
{
	if (Load) {
		bool	retc;
		bool	PrevEnab = m_Frm->EnableTimer(FALSE);	// stop timer
		if (m_ViewBackup == NULL) {	// if view not backed up
			ViewBackup	*vb = new ViewBackup;	// save view, dtor restores it
			retc = GetView()->Serialize(Path, TRUE);	// load snapshot
			if (retc)
				m_ViewBackup = vb;	// EndSnapMode will delete, restoring view
			else
				delete vb;	// delete backup
		} else	// view already backed up
			retc = GetView()->Serialize(Path, TRUE);	// just load snapshot
		if (retc) {
			if (!m_IsLoaded) {	// insert snapshot menu into menu bar
				CMenu	*pm = m_SnapMenu.GetSubMenu(0);
				CString	s;
				m_SnapMenu.GetMenuString(0, s, MF_BYPOSITION);
				m_Frm->GetMenu()->InsertMenu(CMainFrame::MENU_SNAPSHOT, 
					MF_BYPOSITION | MF_POPUP,
					(UINT)pm->m_hMenu, s);
				m_Frm->DrawMenuBar();
				m_IsLoaded = TRUE;
			}
			m_Frm->SetCaption(PathFindFileName(Path));
			m_Frm->Pause(TRUE);
		} else
			m_Frm->EnableTimer(PrevEnab);	// restore timer
		return(retc);
	} else
		return(GetView()->Serialize(Path, FALSE));	// store snapshot
}

void CSnapshot::Unload()
{
	if (m_IsLoaded) {
		if (m_ViewBackup != NULL) {	// if backup exists
			delete m_ViewBackup;	// restore view
			m_ViewBackup = NULL;	// mark backup as deleted
		}
		m_Frm->GetMenu()->RemoveMenu(CMainFrame::MENU_SNAPSHOT, MF_BYPOSITION);
		m_Frm->DrawMenuBar();	// remove snapshot menu
		m_Frm->SetCaption(m_Frm->GetDoc()->GetTitle());
		m_SnapList.RemoveAll();
		m_IsLoaded = FALSE;
	}
}

bool CSnapshot::Show(int Idx)
{
	if (!GetCount())
		return(FALSE);
	m_CurSel = CLAMP(Idx, 0, GetCount() - 1);
	return(Serialize(m_SnapList[m_CurSel], TRUE));
}

bool CSnapshot::SlideShow()
{
	return(m_SlideShowDlg.DoModal() == IDOK);
}
