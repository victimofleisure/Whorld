// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar25	initial version
		01		20mar25	refactor asynchronous writing

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

// Types
	struct ERROR_STATE {
		DWORD	nError;			// system error code
		int		nLineNum;		// line number at which error occurred
		LPCSTR	pszSrcFileName;	// file name of relevant source file
		LPCSTR	pszSrcFileDate;	// source file's compilation date
	};

// Constants
	enum {	// input/output states
		IO_CLOSED = -1,	// file is closed
		IO_READ,		// file is being read
		IO_WRITE,		// file is being written
	};
	static const UINT m_nFileID;
	static const USHORT m_nFileVersion;

// Attributes
	void	GetLastErrorState(ERROR_STATE& errLast) const;
	bool	IsOpen() const;
	bool	IsReading() const;
	bool	IsWriting() const;
	int		GetIOState() const;
	bool	IsEndOfFile() const;
	LONGLONG	GetFramesWritten() const;
	LONGLONG	GetFrameCount() const;
	LONGLONG	GetReadFrameIdx() const;
	float	GetFrameRate() const;
	void	SetFrameRate(float fFrameRate);
	D2D1_SIZE_F	GetTargetSize() const;
	void	SetTargetSize(const D2D1_SIZE_F& szTarget);

// Operations
	bool	Open(LPCTSTR pszPath, bool bWrite);
	bool	Close();
	bool	Write(const CSnapshot *pSnapshot);
	CSnapshot*	Read();
	bool	SeekFrame(LONGLONG iFrame);

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
		LONGLONG	nFrameCount;	// number of frames in movie
		ULONGLONG	nIndexOffset;	// file offset of frame index in bytes
	};
	typedef CBlockArray<FRAME_INDEX_ENTRY, FRAME_INDEX_BLOCK_SIZE> CFrameIndexBlockArray;
	typedef CArrayEx<FRAME_INDEX_ENTRY, FRAME_INDEX_ENTRY> CFrameIndexArray;

// Data members
	HANDLE	m_hFile;			// handle of the movie file
	LONGLONG	m_nWriteFrames;	// during writing, number of frames written so far
	ULONGLONG	m_nWriteBytes;	// during writing, byte offset of current frame
	CFrameIndexBlockArray	m_aWriteFrameIndex;	// during writing, each frame's byte offset
	CFrameIndexArray	m_aReadFrameIndex;	// during reading, each frame's byte offset
	HEADER	m_hdr;				// file header; used during both reading and writing
	LONGLONG	m_iReadFrame;	// during reading, current position as a frame index
	int		m_nIOState;			// movie input/output state; see enum above
	int		m_nLastErrorLine;	// line number on which last error occurred
	OVERLAPPED	m_ovlWrite;		// overlapped struct for asynchronous writing
	CAutoPtr<const CSnapshot>	m_pSnapWrite;	// snapshot for overlapped write
	DWORD	m_nAsyncWriteSize;	// size of overlapped write in bytes

// Helpers
	bool	PreparedForWrite() const;
	bool	ReadHeader();
	bool	WriteHeader();
	bool	ReadFrameIndex();
	bool	WriteFrameIndex();
	bool	FinalizeWrite();
	bool	ReadBuf(LPVOID lpBuffer, DWORD nBytesToRead);
	bool	WriteBuf(LPCVOID lpBuffer, DWORD nBytesToWrite);
	bool	WriteSync(LPCVOID lpBuffer, DWORD nBytesToWrite, ULONGLONG nWritePos);
	bool	WriteAsync(const CSnapshot *pSnapshot);
	bool	FinishAsyncWrite();
};

inline bool CSnapMovie::IsOpen() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}

inline bool CSnapMovie::IsReading() const
{
	return m_nIOState == IO_READ;
}

inline bool CSnapMovie::IsWriting() const
{
	return m_nIOState > IO_READ;
}

inline int CSnapMovie::GetIOState() const
{
	return m_nIOState;
}

inline bool CSnapMovie::IsEndOfFile() const
{
	ASSERT(IsReading());	// only supported for reading
	return m_iReadFrame >= m_hdr.nFrameCount;
}

inline LONGLONG CSnapMovie::GetFramesWritten() const
{
	return m_nWriteFrames;
}

inline LONGLONG CSnapMovie::GetFrameCount() const
{
	return m_hdr.nFrameCount;
}

inline LONGLONG CSnapMovie::GetReadFrameIdx() const
{
	return m_iReadFrame;
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

inline bool CSnapMovie::PreparedForWrite() const
{
	// true if frame rate and target size are specified
	return m_hdr.fFrameRate > 0 && m_hdr.szTarget.width > 0 && m_hdr.szTarget.height > 0;
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
