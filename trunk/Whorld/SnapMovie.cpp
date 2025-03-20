// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar25	initial version
		01		19mar25	remove asynchronous writing
		02		20mar25	open must reset frames written count

*/

#include "stdafx.h"
#include "SnapMovie.h"
#include "Snapshot.h"

const UINT CSnapMovie::m_nFileID = CSnapshot::MakeFourCC('W', 'H', 'M', '2');
const USHORT CSnapMovie::m_nFileVersion = 1;

// Use this version to handle failure of a system function;
// it assumes the function has already called SetLastError.
#define ABORT { m_nLastErrorLine = __LINE__; return false; }

// Use this version to handle application-specific failures;
// it obliges you to specify a suitable Windows error code.
#define ABORT2(errcode) { m_nLastErrorLine = __LINE__; SetLastError(errcode); return false; }

#define ASYNCHRONOUS_WRITING 1

CSnapMovie::CSnapMovie()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_nWriteFrames = 0;
	m_nWriteBytes = 0;
	ZeroMemory(&m_hdr, sizeof(m_hdr));
	m_iReadFrame = 0;
	m_nIOState = IO_CLOSED;
	m_nLastErrorLine = 0;
}

CSnapMovie::~CSnapMovie()
{
	Close();
}

void CSnapMovie::GetLastErrorState(ERROR_STATE& errLast) const
{
	errLast.nError = GetLastError();
	errLast.nLineNum = m_nLastErrorLine;
	errLast.pszSrcFileName = __FILE__;
	errLast.pszSrcFileDate = __DATE__;
}

bool CSnapMovie::Open(LPCTSTR pszPath, bool bWrite)
{
	if (!Close()) {	// close file if it's open
		return false;
	}
	DWORD	dwDesiredAccess;
	DWORD	dwCreationDisposition;
	if (bWrite) {	// if opening for write
		// frame rate and target size must be specified before opening
		if (!PreparedForWrite()) {	// if caller hasn't prepared
			ABORT2(ERROR_INVALID_STATE);	// fail
		}
		dwDesiredAccess = GENERIC_WRITE;
		dwCreationDisposition = CREATE_ALWAYS;
	} else {	// opening for read
		dwDesiredAccess = GENERIC_READ | FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
	}
	// open file for read or write as specified above
	m_hFile = CreateFile(pszPath, dwDesiredAccess, 
		0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
	if (!IsOpen()) {	// if file not open
		ABORT;	// failed to open file
	}
	m_aWriteFrameIndex.RemoveAll();
	m_aReadFrameIndex.RemoveAll();
	if (bWrite) {	// if opening for write
		m_nWriteFrames = 0;	// reset frames written count
		m_nWriteBytes = sizeof(m_hdr);	// set output position past header
		// write dummy header; real header will be written by Close
		if (!WriteHeader()) {	// if we can't write header
			return false;	// fail; error already handled
		}
	} else {	// opening for read
		if (!ReadHeader()) {	// if we can't read header
			return false;	// fail; error already handled
		}
		if (!ReadFrameIndex()) {	// if we can't read frame index
			return false;	// fail; error already handled
		}
		m_iReadFrame = 0;	// read starts with first frame
	}
	m_nIOState = bWrite;	// update I/O state
	return true;	// success
}

bool CSnapMovie::Close()
{
	if (!IsOpen()) {	// if file isn't open
		return true;	// nothing to do, but not an error
	}
	bool	bRetVal = true;
	if (IsWriting()) {	// if we're writing
		// write frame index and then rewrite header
		bRetVal = FinalizeWrite();
	}
	CloseHandle(m_hFile);	// close file
	m_hFile = INVALID_HANDLE_VALUE;	// mark it closed
	m_nIOState = IO_CLOSED;	// update I/O state
	return bRetVal;
}

bool CSnapMovie::ReadHeader()
{
	ZeroMemory(&m_hdr, sizeof(m_hdr));
	if (!ReadBuf(&m_hdr, sizeof(m_hdr))) {	// if we can't read header
		ABORT;	// failed to read header
	}
	if (m_hdr.nFileID != m_nFileID) {	// if file identifiers don't match
		ABORT2(ERROR_INVALID_DATA);	// unexpected file format
	}
	return true;	// success
}

bool CSnapMovie::WriteHeader()
{
	// frame rate and target size must be specified beforehand
	if (!PreparedForWrite()) {	// if caller didn't do so
		ABORT2(ERROR_INVALID_STATE);	// fail
	}
	m_hdr.nFileID = m_nFileID;
	m_hdr.nVersion = m_nFileVersion;
	m_hdr.nDrawStateSize = sizeof(DRAW_STATE);
	m_hdr.nGlobRingSize = sizeof(GLOB_RING);
	m_hdr.nRingSize = sizeof(RING);
	m_hdr.nFrameCount = m_nWriteFrames;
	m_hdr.nIndexOffset = m_nWriteBytes;
	if (!WriteBuf(&m_hdr, sizeof(m_hdr))) {	// if we can't write header
		ABORT;	// failed to write header
	}
	return true;	// success
}

bool CSnapMovie::ReadFrameIndex()
{
	ASSERT(m_hdr.nFileID == m_nFileID);	// sanity check
	// seek to frame index
	LARGE_INTEGER	nFilePos;
	nFilePos.QuadPart = m_hdr.nIndexOffset;
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to seek to start of file
	}
	// read frame index
	m_aReadFrameIndex.SetSize(m_hdr.nFrameCount);
	DWORD	nBufSize = static_cast<DWORD>(m_hdr.nFrameCount * sizeof(FRAME_INDEX_ENTRY));
	if (!ReadBuf(m_aReadFrameIndex.GetData(), nBufSize)) {	// if we can't read
		ABORT;	// failed to read frame index
	}
	// seek to first snapshot
	nFilePos.QuadPart = sizeof(m_hdr);
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to seek to first snapshot
	}
	return true;	// success
}

bool CSnapMovie::WriteFrameIndex()
{
	INT_PTR	nBlocks = m_aWriteFrameIndex.GetBlockCount();
	for (int iBlock = 0; iBlock < nBlocks; iBlock++) {	// for each block
		const FRAME_INDEX_ENTRY* pEntry = m_aWriteFrameIndex.GetBlock(iBlock);
		DWORD	nIdxEntries = static_cast<DWORD>(m_aWriteFrameIndex.GetBlockSize(iBlock));
		DWORD	nBufSize = nIdxEntries * sizeof(FRAME_INDEX_ENTRY);
		if (!WriteBuf(pEntry, nBufSize)) {	// if we can't write block
			ABORT;	// failed to write frame index block
		}
	}
	LARGE_INTEGER	liZero = {0};
	if (!SetFilePointerEx(m_hFile, liZero, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to rewind to start of file
	}
	return true;	// success
}

CSnapshot* CSnapMovie::Read()
{
	// same as CSnapshot::Read except file header is handled differently
	// and we use file handle instead of CFile; no exceptions are thrown
	ASSERT(m_hdr.nFileID == m_nFileID);	// sanity check
	DRAW_STATE	drawState;
	ZeroMemory(&drawState, sizeof(DRAW_STATE));
	if (!ReadBuf(&drawState, min(m_hdr.nDrawStateSize, sizeof(DRAW_STATE)))) {
		ABORT;	// failed to read draw state
	}
	// Allocate a snapshot having sufficient space for the specified number
	// of rings, and attach it to a smart pointer for automatic cleanup.
	CAutoPtr<CSnapshot>	pSnapshot(CSnapshot::Alloc(drawState.nRings));
	if (pSnapshot == NULL) {	// if allocation failed
		ABORT;	// failed to allocate snapshot
	}
	pSnapshot->m_drawState = drawState;
	ZeroMemory(&pSnapshot->m_globRing, sizeof(GLOB_RING));
	if (!ReadBuf(&pSnapshot->m_globRing, min(m_hdr.nGlobRingSize, sizeof(GLOB_RING)))) {
		ABORT;	// failed to read global ring
	}
	if (m_hdr.nRingSize == sizeof(RING)) {	// if ring is current size
		if (!ReadBuf(pSnapshot->m_aRing, sizeof(RING) * drawState.nRings)) {
			ABORT;	// failed to read ring array
		}
	} else {	// legacy ring size; must read rings one at a time
		int	nRingSize = min(m_hdr.nRingSize, sizeof(RING));
		for (int iRing = 0; iRing < drawState.nRings; iRing++) {
			if (!ReadBuf(&pSnapshot->m_aRing[iRing], nRingSize)) {
				ABORT;	// failed to read ring
			}
		}
	}
	m_iReadFrame++;	// increment read position in frames
	// detach snapshot from smart pointer and return it to caller
	return pSnapshot.Detach();	// success
}

bool CSnapMovie::SeekFrame(LONGLONG iFrame)
{
	if (!IsReading()) {	// if not reading
		ABORT2(ERROR_INVALID_OPERATION);	// attempted seek while not reading
	}
	if (iFrame < 0 || iFrame >= GetFrameCount()) {	// if range error
		ABORT2(ERROR_INVALID_INDEX);	// frame index is out of range
	}
	// seek to specified frame
	LARGE_INTEGER	nFilePos;
	nFilePos.QuadPart = m_aReadFrameIndex[iFrame];
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to seek to specified frame
	}
	m_iReadFrame = iFrame;	// set read position
	return true;
}

bool CSnapMovie::FinalizeWrite()
{
	if (!WriteFrameIndex()) {	// if we can't write the frame index
		return false;	// fail; error already handled
	}
	if (!WriteHeader()) {	// if we can't rewrite the header
		return false;	// fail; error already handled
	}
	return true;	// success
}

bool CSnapMovie::Write(const CSnapshot *pSnapshot)
{
	if (pSnapshot == NULL) {	// if null snapshot
		ABORT2(ERROR_INVALID_PARAMETER);	// fail
	}
	CAutoPtr<const CSnapshot> pSnapshotHolder(pSnapshot);	// ensure cleanup
	DWORD	nSnapSize = pSnapshot->GetSize();	// get snapshot size
	if (!WriteBuf(pSnapshot, nSnapSize)) {	// synchronous write
		ABORT;	// failed to write snapshot
	}
	m_aWriteFrameIndex.Add(m_nWriteBytes);	// add file position to frame index
	m_nWriteBytes += nSnapSize;	// add snapshot size to total bytes written
	m_nWriteFrames++;	// bump count of frames written
	return true;	// success
}
