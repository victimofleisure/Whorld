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

#include "stdafx.h"
#include "SnapMovie.h"
#include "Snapshot.h"

const UINT CSnapMovie::m_nFileID = CSnapshot::MakeFourCC('W', 'H', 'M', '2');
const USHORT CSnapMovie::m_nFileVersion = 1;

CSnapMovie::CSnapMovie()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_nWriteFrames = 0;
	m_nWriteBytes = 0;
	ZeroMemory(&m_hdr, sizeof(m_hdr));
	m_nIOState = IO_CLOSED;
}

CSnapMovie::~CSnapMovie()
{
	Close();
}

bool CSnapMovie::Open(LPCTSTR pszPath, bool bWrite)
{
	Close();	// close file if it's open
	DWORD	dwAttributes;
	DWORD	dwDesiredAccess;
	DWORD	dwCreationDisposition;
	if (bWrite) {	// if opening for write
		// frame rate and target size must be specified before opening
		if (!PreparedForWrite()) {	// if caller hasn't done so
			return false;	// fail
		}
		dwDesiredAccess = GENERIC_WRITE;
		dwCreationDisposition = CREATE_ALWAYS;
		dwAttributes = FILE_FLAG_OVERLAPPED;
	} else {	// opening for read
		dwDesiredAccess = GENERIC_READ | FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
		dwAttributes = 0;
	}
	// open file for read or write as specified above
	m_hFile = CreateFile(pszPath, dwDesiredAccess, 
		0, NULL, dwCreationDisposition, dwAttributes, 0);
	if (!IsOpen()) {	// if file not open
		return false;	// fail
	}
	m_aWriteFrameIndex.RemoveAll();
	m_aReadFrameIndex.RemoveAll();
	if (bWrite) {	// if opening for write
		// write dummy header; real header will be written by Close
		if (!WriteHeader()) {	// if we can't write header
			return false;	// fail
		}
		m_nWriteBytes = sizeof(m_hdr);	// this is crucial
	} else {	// opening for read
		if (!ReadHeader()) {	// if we can't read header
			return false;	// fail
		}
		if (!ReadFrameIndex()) {	// if we can't read frame index
			return false;	// fail
		}
	}
	m_nIOState = bWrite;	// update I/O state
	return true;	// success
}

bool CSnapMovie::Close()
{
	if (!IsOpen()) {	// if file isn't open
		return false;	// nothing to do
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
		return false;	// fail
	}
	if (m_hdr.nFileID != m_nFileID) {	// if unexpected file identifier
		return false;	// fail
	}
	printf("Frames = %lld\nFrame Size = %f x %f\nFrame Rate = %f\n", 
		m_hdr.nFrameCount, m_hdr.szTarget.width, m_hdr.szTarget.height, m_hdr.fFrameRate);//@@@ debug only
	return true;	// success
}

bool CSnapMovie::WriteHeader()
{
	// frame rate and target size must be specified beforehand
	if (!PreparedForWrite()) {	// if caller didn't do so
		return false;	// fail
	}
	m_hdr.nFileID = m_nFileID;
	m_hdr.nVersion = m_nFileVersion;
	m_hdr.nDrawStateSize = sizeof(DRAW_STATE);
	m_hdr.nGlobRingSize = sizeof(GLOB_RING);
	m_hdr.nRingSize = sizeof(RING);
	m_hdr.nFrameCount = m_nWriteFrames;
	m_hdr.nIndexOffset = m_nWriteBytes + sizeof(m_hdr);
	if (!WriteWait(&m_hdr, sizeof(m_hdr), 0)) {	// if we can't write header
		return false;	// fail
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
		return false;	// fail
	}
	// read frame index
	m_aReadFrameIndex.SetSize(m_hdr.nFrameCount);
	DWORD	nEntrySize = static_cast<DWORD>(m_aWriteFrameIndex.GetSize() * sizeof(FRAME_INDEX_ENTRY));
	if (!ReadBuf(m_aReadFrameIndex.GetData(), nEntrySize)) {	// if we can't read
		return false;	// fail
	}
	// seek to first snapshot
	nFilePos.QuadPart = sizeof(m_hdr);
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {	// if we can't seek
		return false;	// fail
	}
	return true;	// success
}

bool CSnapMovie::WriteFrameIndex()
{
	INT_PTR	nBlocks = m_aWriteFrameIndex.GetBlockCount();
	ULONGLONG	nFilePos = m_nWriteBytes;
	for (int iBlock = 0; iBlock < nBlocks; iBlock++) {	// for each block
		const FRAME_INDEX_ENTRY* pEntry = m_aWriteFrameIndex.GetBlock(iBlock);
		DWORD	nIdxEntries = static_cast<DWORD>(m_aWriteFrameIndex.GetBlockSize(iBlock));
		DWORD	nBufSize = nIdxEntries * sizeof(FRAME_INDEX_ENTRY);
		if (!WriteWait(pEntry, nBufSize, nFilePos)) {	// if we can't write block
			return false;	// fail
		}
		nFilePos += nBufSize;	// increment file position by amount we wrote
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
		return NULL;	// failed to read draw state
	}
	// Allocate a snapshot having sufficient space for the specified number
	// of rings, and attach it to a smart pointer for automatic cleanup.
	CAutoPtr<CSnapshot>	pSnapshot(CSnapshot::Alloc(drawState.nRings));
	if (pSnapshot == NULL) {	// if allocation failed
		return NULL;	// can't proceed
	}
	pSnapshot->m_drawState = drawState;
	ZeroMemory(&pSnapshot->m_globRing, sizeof(GLOB_RING));
	if (!ReadBuf(&pSnapshot->m_globRing, min(m_hdr.nGlobRingSize, sizeof(GLOB_RING)))) {
		return NULL;	// failed to global ring
	}
	if (m_hdr.nRingSize == sizeof(RING)) {	// if ring is current size
		if (!ReadBuf(pSnapshot->m_aRing, sizeof(RING) * drawState.nRings)) {
			return NULL;	// failed to read rings
		}
	} else {	// legacy ring size; must read rings one at a time
		int	nRingSize = min(m_hdr.nRingSize, sizeof(RING));
		for (int iRing = 0; iRing < drawState.nRings; iRing++) {
			if (!ReadBuf(&pSnapshot->m_aRing[iRing], nRingSize)) {
				return NULL;	// failed to read ring
			}
		}
	}
	// detach snapshot from smart pointer and return it to caller
	return pSnapshot.Detach();	// success
}

bool CSnapMovie::FinalizeWrite()
{
	if (!CompletePendingWrites()) {	// if we can't complete writes
		return false;	// fail
	}
	if (!WriteFrameIndex()) {	// if we can't write the frame index
		return false;	// fail
	}
	if (!WriteHeader()) {	// if we can't rewrite the header
		return false;	// fail
	}
	return true;	// success
}

bool CSnapMovie::WriteWait(LPCVOID lpBuffer, DWORD nBytesToWrite, ULONGLONG nFilePos)
{
	OVERLAPPED	ovl;
	ZeroMemory(&ovl, sizeof(ovl));
	LARGE_INTEGER	li;	// split 64-bit file position in half, for legacy reasons
	li.QuadPart = nFilePos;
	ovl.Offset = li.LowPart;
	ovl.OffsetHigh = li.HighPart;
	// start asynchronous write; if it fails but last error is I/O pending, it's OK
	BOOL	bResult = WriteFile(m_hFile, lpBuffer, nBytesToWrite, NULL, &ovl);
	if (!bResult && GetLastError() != ERROR_IO_PENDING) {	// if any error other than pending
		return false;	// fail
	}
	// write was successfully started, so now wait for it to finish
	DWORD	nBytesWritten;
	bResult = GetOverlappedResult(m_hFile, &ovl, &nBytesWritten, true);
	if (!bResult || nBytesWritten != nBytesToWrite) {	// if any error other than pending
		return false;	// fail
	}
	return true;	// success
}

bool CSnapMovie::Write(const CSnapshot *pSnapshot)
{
	if (pSnapshot == NULL) {	// if null snapshot
		return false;	// fail
	}
	// Look for a free buffer, but without waiting; in normal conditions
	// this first method will succeed and likely return the first buffer.
	int	iBuf = FindWriteBuffer(WS_COMPLETED);	// very efficient
	if (iBuf < 0) {	// if no free buffers were found
		// all buffers are in use; system is fully loaded
		iBuf = WaitForFreeWriteBuffer();	// waits for a completion signal
		if (iBuf < 0) {	// if still no free buffer found; something's wrong
			delete pSnapshot;	// avoid memory leak
			return false;	// fail
		}
	}
	// got a free buffer
	CWriteBuf&	buf = m_arrWriteBuf[iBuf];	// for brevity
	buf.m_pSnapshot.Free();	// it's now safe to free previous snapshot
	buf.m_pSnapshot.Attach(const_cast<CSnapshot*>(pSnapshot));	// attach new snapshot to buffer
	ZeroMemory(&buf.m_ovl, sizeof(OVERLAPPED));	// clear overlapped struct, as required
	LARGE_INTEGER	li;	// split 64-bit file position in half, for legacy reasons
	li.QuadPart = m_nWriteBytes;
	buf.m_ovl.Offset = li.LowPart;
	buf.m_ovl.OffsetHigh = li.HighPart;
	DWORD	nSnapSize = pSnapshot->GetSize();	// get snapshot size
	// start asynchronous write; if it fails but last error is I/O pending, it's OK
	BOOL	bResult = WriteFile(m_hFile, buf.m_pSnapshot, nSnapSize, NULL, &buf.m_ovl);
	if (!bResult && GetLastError() != ERROR_IO_PENDING) {	// if any error other than pending
		return false;	// fail
	}
	m_aWriteFrameIndex.Add(m_nWriteBytes);	// add file position to frame index
	m_nWriteBytes += nSnapSize;	// add snapshot size to total bytes written
	m_nWriteFrames++;	// bump count of frames written
	return true;	// success
}

int CSnapMovie::FindWriteBuffer(bool bDesiredState)
{
	for (int iBuf = 0; iBuf < WRITE_BUFFERS; iBuf++) {	// for each write buffer
		CWriteBuf&	buf = m_arrWriteBuf[iBuf];	// for brevity
		// high performance test that polls for completion of an outstanding I/O
		bool	bIsCompleted = HasOverlappedIoCompleted(&buf.m_ovl) != 0;
		if (bIsCompleted == bDesiredState) {	// if buffer is in the desired state
			return iBuf;	// return index of buffer
		}
	}
	return -1;	// no buffers are in the desired state
}

int CSnapMovie::WaitForFreeWriteBuffer()
{
	int	iBuf = FindWriteBuffer(WS_PENDING);	// find a pending write
	if (iBuf < 0) {	// if no writes are pending (unlikely but possible)
		return 0;	// return index of first buffer
	}
	// found a pending write
	CWriteBuf&	buf = m_arrWriteBuf[iBuf];	// for brevity
	DWORD	nBytesWritten;
	// wait for pending write to finish
	BOOL	bResult = GetOverlappedResult(m_hFile, &buf.m_ovl, &nBytesWritten, true);
	if (!bResult) {	// if we can't get overlapped result
		return -1;	// fail
	}
	// A write completed, but we don't know which one. This ambiguity could
	// be avoided by giving each write buffer its own independent event object,
	// but it's easier (though maybe less efficient) to just poll them again.
	return FindWriteBuffer(WS_COMPLETED);	// find a completed write
}

bool CSnapMovie::CompletePendingWrites()
{
	for (int iBuf = 0; iBuf < WRITE_BUFFERS; iBuf++) {	// for each write buffer
		CWriteBuf&	buf = m_arrWriteBuf[iBuf];	// for brevity
		DWORD	nBytesWritten;
		// wait for pending write to finish
		BOOL	bResult = GetOverlappedResult(m_hFile, &buf.m_ovl, &nBytesWritten, true);
		if (!bResult) {	// if we can't get overlapped result
			return false;	// fail
		}
	}
	// We don't free ANY snapshots until ALL writes are completed, due to
	// ambiguity about which write signaled; see WaitForFreeWriteBuffer.
	for (int iBuf = 0; iBuf < WRITE_BUFFERS; iBuf++) {	// for each write buffer
		CWriteBuf&	buf = m_arrWriteBuf[iBuf];	// for brevity
		buf.m_pSnapshot.Free();	// free snapshot
	}
	return true;	// success
}
