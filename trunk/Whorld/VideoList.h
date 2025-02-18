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
		03		05jun06	add auto-rewind
		04		07jun06	add path history
		05		09jun06	add SetFileSearch to avoid deadlock
		06		23jan08	fix SetCycleLength assertion
		07		28jan08	support Unicode

        array of video clips
 
*/

#ifndef CVIDEOLIST_INCLUDED
#define CVIDEOLIST_INCLUDED

#include "Video.h"

class CVideoList : public WObject {
public:
// Construction
	CVideoList();

// Constants
	enum {
		MAX_VIDEOS = 10,	// maximum number of open videos
		ROPS = 10,			// number of raster operations
		DEFAULT_ROP = 6		// index of default raster operation
	};

// Types
	typedef struct tagINFO_BASE {
		int		CurPos;		// current video's position in list, or -1 if none
		int		ROPIdx;		// index of raster operation
		bool	AutoRewind;	// if true, rewind clips before playing them
		bool	Cycle;		// if true, cycle clips
		int		CycleLen;	// cycle length
	} INFO_BASE;
	typedef struct tagINFO : INFO_BASE {
		TCHAR	MRUPath[MAX_VIDEOS][MAX_PATH];	// most recently used videos
	} INFO;
	
// Attributes
	int		GetCurSel() const;
	bool	IsPlaying() const;
	CVideo&	GetCurVideo();
	bool	IsOpen(int Pos) const;
	LPCTSTR	GetPath(int Pos) const;
	void	SetROPIdx(int Idx);
	int		GetROPIdx() const;
	UINT	GetROP() const;
	HRESULT	GetLastError() const;
	LPCTSTR	GetLastErrorString() const;
	void	SetAutoRewind(bool Enable);
	bool	GetAutoRewind() const;
	void	SetCycleLength(int Length);
	int		GetCycleLength() const;
	void	Cycle(bool Enable);
	bool	IsCycling() const;
	void	SetInfo(const INFO& Info);
	void	GetInfo(INFO& Info);
	void	GetDefaults(INFO& Info);
	int		GetHistoryIdx() const;
	const	CStringArray& GetHistory() const;
	bool	GetFileSearch() const;
	void	SetFileSearch(bool Enable);

// Operations
	bool	CreateSurfaces(CBackBufDD& BackBuf);
	bool	Open(LPCTSTR Path);
	bool	Open(LPCTSTR Path, DWORD Frame);
	bool	LoadFrame(int HistIdx, DWORD Frame);
	bool	Select(int Pos);
	bool	SelectRelative(int Delta);
	bool	SelectNext();
	bool	SelectPrev();
	void	Deselect();
	void	Rewind();
	void	RemoveAll();
	void	Record(bool Enable);
	bool	SerializeHistory(CArchive& ar);
	bool	CheckLinks(CStringArray& Paths);
	bool	CheckLink(CString& Path);

private:
// Constants
	static const UINT	m_ROPList[ROPS];	// raster operations
	static const INFO_BASE	m_Defaults;		// default settings

// Member data
	CVideo	m_Video[MAX_VIDEOS];	// array of videos
	CDWordArray	m_List;	// video indices, in most recently used order
	CStringArray	m_OpenQueue;	// paths to open at next select
	HRESULT	m_hr;		// most recent DirectDraw result code
	int		m_CurIdx;	// index of current video, or -1 if none
	int		m_CurPos;	// current video's position in list, or -1 if none
	UINT	m_ROP;		// current raster operation
	int		m_ROPIdx;	// index of current raster operation
	int		m_CycleLen;	// cycle length
	bool	m_Cycling;	// if true, cycle videos
	bool	m_AutoRewind;	// if true, rewind videos before playing them
	bool	m_Recording;	// if true, record paths of selected videos
	bool	m_FileSearch;	// if true, CheckLink uses missing files dialog
	CStringArray	m_History;	// recorded video paths
	int		m_HistIdx[MAX_VIDEOS];	// index of each video's path within history

// Helpers
	void	PlayVideo(int Pos, int Idx);
	bool	OpenQueuedVideos();
	void	ClearHistIdx();
};

inline int CVideoList::GetCurSel() const
{
	return(m_CurPos);
}

inline bool CVideoList::IsPlaying() const
{
	return(m_CurIdx >= 0);
}

inline CVideo& CVideoList::GetCurVideo()
{
	ASSERT(m_CurIdx >= 0);
	return(m_Video[m_CurIdx]);
}

inline bool CVideoList::IsOpen(int Pos) const
{
	ASSERT(Pos >= 0 && Pos < MAX_VIDEOS);
	return(m_Video[m_List[Pos]].IsOpen());
}

inline LPCTSTR CVideoList::GetPath(int Pos) const
{
	ASSERT(Pos >= 0 && Pos < MAX_VIDEOS);
	return(m_Video[m_List[Pos]].GetPath());
}

inline void CVideoList::SetROPIdx(int Idx)
{
	ASSERT(Idx >= 0 && Idx < ROPS);
	m_ROPIdx = Idx;
	m_ROP = m_ROPList[Idx];
}

inline int CVideoList::GetROPIdx() const
{
	return(m_ROPIdx);
}

inline UINT CVideoList::GetROP() const
{
	return(m_ROP);
}

inline bool CVideoList::SelectNext()
{
	return(SelectRelative(1));
}

inline bool CVideoList::SelectPrev()
{
	return(SelectRelative(-1));
}

inline void CVideoList::Deselect()
{
	m_CurPos = m_CurIdx = -1;
}

inline void CVideoList::SetAutoRewind(bool Enable)
{
	m_AutoRewind = Enable;
}

inline bool CVideoList::GetAutoRewind() const
{
	return(m_AutoRewind);
}

inline void CVideoList::SetCycleLength(int Length)
{
	ASSERT(Length >= 0 && Length <= MAX_VIDEOS);
	m_CycleLen = Length;
}

inline int CVideoList::GetCycleLength() const
{
	return(m_CycleLen);
}

inline bool CVideoList::IsCycling() const
{
	return(m_Cycling);
}

inline void CVideoList::Cycle(bool Enable)
{
	m_Cycling = Enable;
}

inline int CVideoList::GetHistoryIdx() const
{
	return(m_CurIdx < 0 ? -1 : m_HistIdx[m_CurIdx]);
}

inline const CStringArray& CVideoList::GetHistory() const
{
	return(m_History);
}

inline void CVideoList::ClearHistIdx()
{
	memset(m_HistIdx, -1, sizeof(m_HistIdx));
}

inline bool CVideoList::GetFileSearch() const
{
	return(m_FileSearch);
}

inline void CVideoList::SetFileSearch(bool Enable)
{
	m_FileSearch = Enable;
}

#endif
