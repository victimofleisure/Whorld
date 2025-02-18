// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		30aug05	initial version
		01		07jun06	add video path history
		02		10dec07	bump file version
		03		28jan08	support Unicode

		movie container for snapshots

*/

#ifndef CSNAPMOVIE_INCLUDED
#define CSNAPMOVIE_INCLUDED

#include "WhorldView.h"

class CSnapMovie : public WObject {
public:
// Construction
	CSnapMovie();
	~CSnapMovie();

// Attributes
	void	SetView(CWhorldView *View);
	void	SetFrameRate(UINT Rate);
	UINT	GetFrameRate() const;
	DWORD	GetFrameCount() const;
	void	SetFrameSize(SIZE Size);
	SIZE	GetFrameSize() const;
	DWORD	GetCurPos() const;
	bool	IsOpen() const;

// Operations
	bool	Open(LPCTSTR Path, bool Load);
	void	Close();
	void	Rewind();
	void	Read();
	void	Write();
	void	Seek(int Frame);
	void	SaveFrame(LPCTSTR Path);
	bool	Resize(LPCTSTR Path, SIZE& Size);

private:
// Types
	typedef struct tagFILEID {
		UINT	Sig;
		UINT	Version;
		UINT	HdrSize;
	} FILEID;
	typedef struct tagHEADER {
		UINT	FrameRate;
		UINT	FrameCount;
		SIZE	FrameSize;
		__int64	FrameIndexPos;
	} HEADER;

// Constants
	enum {
		MOVIE_SIG = 0x564d4857,	// movie signature: WHMV (WHorld MoVie)
		MOVIE_VERSION = 3,		// movie version number
		INITIAL_INDEX_ENTRIES = 0x40000,	// enough for two hours at 30 FPS
		FIRST_SNAP_OFS = sizeof(FILEID) + sizeof(HEADER)
	};

// Member data
	CFile	m_File;			// movie is contained in this file
	CArchive	*m_Archive;	// snapshot I/O occurs via this archive
	CWhorldView	*m_View;	// snapshots are serialized to/from this view
	HEADER	m_Hdr;			// movie's header information
	UINT	m_CurPos;		// index of current frame
	CDWordArray	m_SnapSize;	// snapshot sizes, one per frame
	CArray<__int64, __int64&>	m_FrameIndex;	// file pointers, one per frame

// Helpers
	bool	Open(LPCTSTR Path, bool Load, int Mode);
	__int64	GetPos64();
	bool	SetPos64(__int64 Pos);
};

inline bool CSnapMovie::IsOpen() const
{
	return(m_Archive != NULL);
}

inline void CSnapMovie::SetView(CWhorldView *View)
{
	m_View = View;
}

inline void CSnapMovie::SetFrameRate(UINT Rate)
{
	m_Hdr.FrameRate = Rate;
}

inline UINT CSnapMovie::GetFrameRate() const
{
	return(m_Hdr.FrameRate);
}

inline DWORD CSnapMovie::GetFrameCount() const
{
	return(m_Hdr.FrameCount);
}

inline void CSnapMovie::SetFrameSize(SIZE Size)
{
	m_Hdr.FrameSize = Size;
}

inline SIZE CSnapMovie::GetFrameSize() const
{
	return(m_Hdr.FrameSize);
}

inline DWORD CSnapMovie::GetCurPos() const
{
	return(m_CurPos);
}

inline void CSnapMovie::Read()
{
	m_View->MiniSerialize(*m_Archive);
	m_CurPos++;
}

inline void CSnapMovie::Write()
{
	m_View->MiniSerialize(*m_Archive);
	m_SnapSize.SetAtGrow(m_Hdr.FrameCount++, m_View->GetMiniSnapshotSize());
}

inline __int64 CSnapMovie::GetPos64()
{
	LARGE_INTEGER	li;
	li.HighPart = 0;
	li.LowPart = SetFilePointer((HANDLE)m_File.m_hFile, 0, &li.HighPart, FILE_CURRENT);
	return(li.QuadPart);
}
 
inline bool CSnapMovie::SetPos64(__int64 Pos)
{
	LARGE_INTEGER	li;
	li.QuadPart = Pos;
	return(SetFilePointer((HANDLE)m_File.m_hFile, li.LowPart, 
		&li.HighPart, FILE_BEGIN) != 0xffffffff);
}
 
#endif
