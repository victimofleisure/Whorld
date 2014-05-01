// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06may06	initial version
		01		16may06	if cycling, defer updates until next select
		02		22may06	add get/set info
		03		02jun06	clamp ROP and cycle length args
		04		05jun06	add auto-rewind
		05		07jun06	add path history
		06		09jun06	add SetFileSearch to avoid deadlock
		07		28jan08	support Unicode

        array of video clips
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "VideoList.h"
#include "MissingFilesDlg.h"

const UINT CVideoList::m_ROPList[ROPS] = {
	0x00220326,	// ~Src & Dst		DSna		
	0x00BB0226,	// ~Src | Dst		DSno	MERGEPAINT
	0x00440328,	// Src & ~Dst		SDna	SRCERASE
	0x00DD0228,	// Src & ~Dst		SDno		
	0x008800C6,	// Src & Dst		DSa		SRCAND
	0x00EE0086,	// Src | Dst		DSo		SRCPAINT
	0x00660046,	// Src ^ Dst		DSx		SRCINVERT
	0x007700E6,	// ~(Src & Dst)		DSan		
	0x001100A6,	// ~(Src | Dst)		DSon	NOTSRCERASE
	0x00990066,	// ~(Src ^ Dst)		DSxn		
};

const CVideoList::INFO_BASE CVideoList::m_Defaults = {
	-1,				// CurPos
	DEFAULT_ROP,	// ROPIdx
	FALSE,			// AutoRewind
	FALSE,			// IsCycling
	MAX_VIDEOS		// CycleLen
};

CVideoList::CVideoList()
{
	m_List.SetSize(MAX_VIDEOS);
	for (int i = 0; i < MAX_VIDEOS; i++)
		m_List[i] = i;
	m_hr = NULL;
	m_CurIdx = -1;
	m_CurPos = -1;
	m_ROP = m_ROPList[DEFAULT_ROP];
	m_ROPIdx = DEFAULT_ROP;
	m_Cycling = FALSE;
	m_AutoRewind = FALSE;
	m_Recording = FALSE;
	m_FileSearch = TRUE;
	m_CycleLen = MAX_VIDEOS;
	ClearHistIdx();
}

void CVideoList::SetInfo(const INFO& Info)
{
	for (int i = 0; i < MAX_VIDEOS; i++) {
		if (Info.MRUPath[i][0])
			m_Video[i].Open(Info.MRUPath[i]);
		else
			m_Video[i].Close();
		m_List[i] = i;
	}
	ClearHistIdx();
	m_CurPos = Info.CurPos >= 0 && IsOpen(Info.CurPos) ? Info.CurPos : -1;
	m_CurIdx = m_CurPos;
	m_ROPIdx = Info.ROPIdx;
	m_ROP = m_ROPList[m_ROPIdx];
	m_AutoRewind = Info.AutoRewind;
	m_Cycling = Info.Cycle;
	m_CycleLen = Info.CycleLen;
}

void CVideoList::GetInfo(INFO& Info)
{
	for (int i = 0; i < MAX_VIDEOS; i++)
		_tcscpy(Info.MRUPath[i], GetPath(i));
	Info.CurPos = m_CurPos;
	Info.ROPIdx = m_ROPIdx;
	Info.AutoRewind = m_AutoRewind;
	Info.Cycle = m_Cycling;
	Info.CycleLen = m_CycleLen;
}

void CVideoList::GetDefaults(INFO& Info)
{
	INFO_BASE&	ibp = Info;
	ibp = m_Defaults;
	ZeroMemory(Info.MRUPath, sizeof(Info.MRUPath));
}

bool CVideoList::CreateSurfaces(CBackBufDD& BackBuf)
{
	for (int i = 0; i < MAX_VIDEOS; i++) {
		if (FAILED(m_hr = m_Video[i].CreateSurface(BackBuf)))
			return(FALSE);
	}
	return(TRUE);
}

void CVideoList::PlayVideo(int Pos, int Idx)
{
	m_CurPos = Pos;
	m_CurIdx = Idx;
	if (m_AutoRewind && Idx >= 0)
		m_Video[Idx].Rewind();
}

bool CVideoList::Open(LPCTSTR Path)
{
	ASSERT(Path != NULL);
	if (!Path[0])	// else search gives false positive
		return(FALSE);
	if (m_Cycling) {
		m_OpenQueue.Add(Path);	// defer update until next transition
		return(TRUE);
	}
	int	i, Vid = 0;
	for (i = 0; i < MAX_VIDEOS; i++) {	// search our list for path
		Vid = m_List[i];
		if (!_tcscmp(Path, m_Video[Vid].GetPath()))
			break;
	}
	if (i < MAX_VIDEOS)	{				// if video is already open
		m_List.RemoveAt(i);
		m_List.InsertAt(0, Vid);			// move it to front of list
	} else {
		Vid = m_List[MAX_VIDEOS - 1];	// least recently used slot
		if (!m_Video[Vid].Open(Path)) 	// open video in LRU slot
			return(FALSE);
		if (m_Recording) {
			m_HistIdx[Vid] = m_History.GetSize();
			m_History.Add(Path);
		}
		m_List.RemoveAt(MAX_VIDEOS - 1);
		m_List.InsertAt(0, Vid);			// move it to front	of list
	}
	PlayVideo(0, Vid);						// make it current
	return(TRUE);
}

bool CVideoList::OpenQueuedVideos()
{
	bool	retc = TRUE;	// assume success
	bool	WasCycling = m_Cycling;
	m_Cycling = FALSE;	// so open doesn't queue
	int	vids = m_OpenQueue.GetSize();
	for (int i = 0; i < vids; i++) {
		if (!Open(m_OpenQueue[i]))
			retc = FALSE;
	}
	m_Cycling = WasCycling;	// restore cycling state
	m_OpenQueue.RemoveAll();
	return(retc);
}

bool CVideoList::SelectRelative(int Delta)
{
	int	Lower = 0, Upper;
	if (m_Cycling) {
		if (m_OpenQueue.GetSize())
			return(OpenQueuedVideos());
		Upper = m_CycleLen - 1;
	} else
		Upper = MAX_VIDEOS - 1;
	int	Pos = m_CurPos;
	for (int i = 0; i < MAX_VIDEOS; i++) {
		Pos += Delta;	// bump and wrap
		if (Pos < Lower)
			Pos = Upper;
		else if (Pos > Upper)
			Pos = Lower;
		int	Vid = m_List[Pos];
		if (m_Video[Vid].IsOpen()) {
			PlayVideo(Pos, Vid);
			return(TRUE);
		}
	}
	return(FALSE);
}

bool CVideoList::Select(int Pos)
{
	if (Pos >= 0 && Pos < MAX_VIDEOS) {
		int	Vid = m_List[Pos];
		if (m_Video[Vid].IsOpen()) {
			PlayVideo(Pos, Vid);
			return(TRUE);
		}
	} else
		Deselect();
	return(FALSE);
}

void CVideoList::Rewind()
{
	if (IsPlaying())
		GetCurVideo().Rewind();
}

void CVideoList::RemoveAll()
{
	for (int i = 0; i < MAX_VIDEOS; i++)
		m_Video[i].Close();
	m_OpenQueue.RemoveAll();
	ClearHistIdx();
	Deselect();
}

HRESULT CVideoList::GetLastError() const
{
	if (FAILED(m_hr))
		return(m_hr);
	return(m_Video[MAX_VIDEOS - 1].GetLastError());
}

LPCTSTR CVideoList::GetLastErrorString() const
{
	if (FAILED(m_hr))
		return(CBackBufDD::GetErrorString(m_hr));
	return(m_Video[MAX_VIDEOS - 1].GetLastErrorString());
}

bool CVideoList::Open(LPCTSTR Path, DWORD Frame)
{
	bool	WasCycling = m_Cycling;
	m_Cycling = FALSE;	// so open doesn't queue
	bool	retc = Open(Path);
	if (retc)
		GetCurVideo().Seek(Frame);
	m_Cycling = WasCycling;
	return(retc);
}

bool CVideoList::LoadFrame(int HistIdx, DWORD Frame)
{
	// if specified video is already playing, just seek frame
	if (IsPlaying() && HistIdx == m_HistIdx[m_CurIdx]) {
		GetCurVideo().Seek(Frame);
		return(TRUE);
	}
	bool	retc = Open(m_History[HistIdx], Frame);
	if (retc)
		m_HistIdx[m_CurIdx] = HistIdx;
	else
		Deselect();
	return(retc);
}

void CVideoList::Record(bool Enable)
{
	m_Recording = Enable;
	if (Enable) {
		m_History.RemoveAll();
		for (int i = 0; i < MAX_VIDEOS; i++) {
			if (m_Video[i].IsOpen()) {
				m_HistIdx[i] = m_History.GetSize();
				m_History.Add(m_Video[i].GetPath());
			} else
				m_HistIdx[i] = -1;
		}
	}
}

bool CVideoList::SerializeHistory(CArchive& ar)
{
	if (m_Recording && ar.IsLoading()) {
		AfxMessageBox(IDS_VL_CANT_LOAD_MOVIE);
		return(FALSE);
	}
	m_History.Serialize(ar);
	if (ar.IsLoading()) {
		ClearHistIdx();
		return(CheckLinks(m_History));
	}
	return(TRUE);
}

bool CVideoList::CheckLink(CString& Path)
{
	// the following test prevents UI deadlock if we're called during snapshot
	// preview by the open snapshot dialog's UWM_MULTIFILESEL message handler;
	// apparently file dialog message handlers shouldn't create modal dialogs
	if (!m_FileSearch)
		return(TRUE);	// let caller handle file not found
	CStringArray	sa;
	sa.Add(Path);
	bool	retc = CheckLinks(sa);
	Path = sa[0];
	return(retc && !Path.IsEmpty());	// false if cancel or file still missing
}

bool CVideoList::CheckLinks(CStringArray& Paths)
{
	CString	Filter((LPCTSTR)IDS_FILTER_AVI);
	CMissingFilesDlg	mf(Paths, EXT_AVI, Filter);
	return(mf.Check() != IDCANCEL);
}
