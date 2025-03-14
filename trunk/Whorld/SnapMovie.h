// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar25	initial version

*/

#pragma once

#include "WhorldBase.h"
#include "BlockArray.h"

class CSnapshot;

class CSnapMovie : public CWhorldBase {
public:
// Construction
	CSnapMovie();
	~CSnapMovie();

// Constants
	enum {	// movie input/output states
		MIO_CLOSED = -1,	// movie is closed
		MIO_READ,			// movie is being read
		MIO_WRITE,			// movie is being written
	};
	static const UINT m_nFileID;
	static const USHORT m_nFileVersion;

// Attributes
	bool	IsOpen() const;
	bool	IsReading() const;
	bool	IsWriting() const;
	int		GetIOState() const;
	ULONGLONG	GetFramesWritten() const;
	ULONGLONG	GetFrameCount() const;
	float	GetFrameRate() const;
	void	SetFrameRate(float fFrameRate);
	D2D1_SIZE_F	GetTargetSize() const;
	void	SetTargetSize(const D2D1_SIZE_F& szTarget);

// Operations
	bool	Open(LPCTSTR pszPath, bool bWrite);
	bool	Close();
	bool	Write(const CSnapshot *pSnapshot);
	CSnapshot*	Read();

protected:
// Constants
	typedef ULONGLONG FRAME_INDEX_ENTRY;	// 64-bit unsigned file offset in bytes
	enum {
		// block size is in elements, and must be a power of two for efficient
		// division; this is enough for over a minute at 60 frames per second
		FRAME_INDEX_BLOCK_SIZE = 1 << 12,	// 4096 elements
	};

// Types
	struct HEADER {
		UINT	nFileID;		// file identifier
		USHORT	nVersion;		// version number
		USHORT	nDrawStateSize;	// size of DRAW_STATE in bytes
		USHORT	nGlobRingSize;	// size of GLOB_RING in bytes
		USHORT	nRingSize;		// size of RING in bytes
		float	fFrameRate;		// frame rate in frames per second
		D2D1_SIZE_F	szTarget;	// target size in device-independent pixels
		ULONGLONG	nFrameCount;	// number of frames in movie
		ULONGLONG	nIndexOffset;	// file offset of frame index in bytes
	};
	typedef CBlockArray<FRAME_INDEX_ENTRY, FRAME_INDEX_BLOCK_SIZE> CFrameIndexBlockArray;
	typedef CArrayEx<FRAME_INDEX_ENTRY, FRAME_INDEX_ENTRY> CFrameIndexArray;

// Data members
	HANDLE	m_hFile;			// handle of the movie file
	ULONGLONG	m_nWriteFrames;	// during writing, number of frames written so far
	ULONGLONG	m_nWriteBytes;	// during writing, byte offset of current frame
	CFrameIndexBlockArray	m_aWriteFrameIndex;	// during writing, each frame's byte offset
	CFrameIndexArray	m_aReadFrameIndex;	// during reading, each frame's byte offset
	HEADER	m_hdr;				// file header if reading
	int		m_nState;			// see enum above

// Helpers
	bool	WriteBuf(LPCVOID lpBuffer, DWORD nBytesToWrite);
	bool	ReadBuf(LPVOID lpBuffer, DWORD nBytesToRead);
	bool	WriteHeader();
	bool	WriteFrameIndex();
	bool	ReadFrameIndex();
	bool	FinalizeWrite();
	bool	ReadHeader();
};

inline bool CSnapMovie::IsOpen() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}

inline bool CSnapMovie::IsReading() const
{
	return m_nState == MIO_READ;
}

inline bool CSnapMovie::IsWriting() const
{
	return m_nState > MIO_READ;
}

inline int CSnapMovie::GetIOState() const
{
	return m_nState;
}

inline ULONGLONG CSnapMovie::GetFramesWritten() const
{
	return m_nWriteFrames;
}

inline ULONGLONG CSnapMovie::GetFrameCount() const
{
	return m_hdr.nFrameCount;
}

inline float CSnapMovie::GetFrameRate() const
{
	return m_hdr.fFrameRate;
}

inline void CSnapMovie::SetFrameRate(float fFrameRate)
{
	m_hdr.fFrameRate = fFrameRate;
}

inline D2D1_SIZE_F CSnapMovie::GetTargetSize() const
{
	return m_hdr.szTarget;
}

inline void CSnapMovie::SetTargetSize(const D2D1_SIZE_F& szTarget)
{
	m_hdr.szTarget = szTarget;
}

inline bool CSnapMovie::WriteBuf(LPCVOID lpBuffer, DWORD nBytesToWrite)
{
	DWORD	nBytesWritten;
	BOOL	bResult = WriteFile(m_hFile, lpBuffer, nBytesToWrite, &nBytesWritten, NULL);
	return bResult && nBytesWritten == nBytesToWrite;
}

inline bool CSnapMovie::ReadBuf(LPVOID lpBuffer, DWORD nBytesToRead)
{
	DWORD	nBytesRead;
	BOOL	bResult = ReadFile(m_hFile, lpBuffer, nBytesToRead, &nBytesRead, NULL);
	return bResult && nBytesRead == nBytesToRead;
}
